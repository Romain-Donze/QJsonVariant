#include "qjsonparser.h"
#include <QBuffer>
#include <QVariant>
#include <QJsonValue>

constexpr inline bool isAsciiDigit(char32_t c) noexcept
{
    return c >= '0' && c <= '9';
}

/*
begin-array     = ws %x5B ws  ; [ left square bracket
begin-object    = ws %x7B ws  ; { left curly bracket
end-array       = ws %x5D ws  ; ] right square bracket
end-object      = ws %x7D ws  ; } right curly bracket
name-separator  = ws %x3A ws  ; : colon
value-separator = ws %x2C ws  ; , comma

Insignificant whitespace is allowed before or after any of the six
structural characters.

ws = *(
          %x20 /              ; Space
          %x09 /              ; Horizontal tab
          %x0A /              ; Line feed or New line
          %x0D                ; Carriage return
      )
*/

enum {
    Space = 0x20,
    Tab = 0x09,
    LineFeed = 0x0a,
    Return = 0x0d,
    BeginArray = 0x5b,
    BeginObject = 0x7b,
    EndArray = 0x5d,
    EndObject = 0x7d,
    NameSeparator = 0x3a,
    ValueSeparator = 0x2c,
    Quote = 0x22
};

static inline void skipByteOrderMark(const char* &ptr, const char* end)
{
    // eat UTF-8 byte order mark
    uchar utf8bom[3] = { 0xef, 0xbb, 0xbf };
    if (end - ptr > 3 &&
        (uchar)ptr[0] == utf8bom[0] &&
        (uchar)ptr[1] == utf8bom[1] &&
        (uchar)ptr[2] == utf8bom[2])
        ptr += 3;
}

static inline bool skipWhitespace(const char* &ptr, const char* end)
{
    while (ptr < end) {
        if (*ptr > Space)
            break;
        if (*ptr != Space &&
            *ptr != Tab &&
            *ptr != LineFeed &&
            *ptr != Return)
            break;
        ++ptr;
    }
    return (ptr < end);
}

static inline QString unescapedString(const QByteArray& ba)
{
    QByteArray decoded;
    const char* src = ba.constData();
    const char* end = src + ba.size();

    while (src < end) {
        if (*src == '\\' && src + 1 < end) {
            ++src;
            switch (*src) {
            case 'b': decoded.append('\b'); break;
            case 'f': decoded.append('\f'); break;
            case 'n': decoded.append('\n'); break;
            case 'r': decoded.append('\r'); break;
            case 't': decoded.append('\t'); break;
            case '"': decoded.append('\"'); break;
            case '\\': decoded.append('\\'); break;
            case 'u':
                if (src + 4 < end) {
                    bool ok = false;
                    ushort u = QByteArray(src + 1, 4).toUShort(&ok, 16);
                    if (ok) {
                        decoded.append(QString(QChar(u)).toUtf8());
                        src += 4;
                        break;
                    }
                }
                // if not valid \uXXXX, fall through and treat literally
                decoded.append('\\');
                decoded.append('u');
                break;
            default:
                decoded.append('\\');
                decoded.append(*src);
                break;
            }
        } else {
            decoded.append(*src);
        }
        ++src;
    }

    return QString::fromUtf8(decoded);
}

static QString parseJsonString(const char* &ptr, const char* end)
{
    bool isUtf8 = true;
    const char* start = ptr;
    while (ptr < end && *ptr != '"') {
        if (*ptr == '\\') {
            ++ptr;
            isUtf8 = false;
        }
        ++ptr;
    }
    if (ptr < end && *ptr == '"') ++ptr;

    int len = ptr - start;
    if(isUtf8) {
        return QString::fromUtf8(start, len-1); // exclude surrounding quotes
    }
    QByteArray sub(start, len-1); // exclude surrounding quotes
    return unescapedString(sub);
}

static QVariant parseJsonNumber(const char* &ptr, const char* end)
{
    const char *start = ptr;
    bool isInt = true;

    // minus
    if (ptr < end && *ptr == '-')
        ++ptr;

    // int = zero / ( digit1-9 *DIGIT )
    if (ptr < end && *ptr == '0') {
        ++ptr;
    } else {
        while (ptr < end && isAsciiDigit(*ptr))
            ++ptr;
    }

    // frac = decimal-point 1*DIGIT
    if (ptr < end && *ptr == '.') {
        ++ptr;
        while (ptr < end && isAsciiDigit(*ptr)) {
            isInt = isInt && *ptr == '0';
            ++ptr;
        }
    }

    // exp = e [ minus / plus ] 1*DIGIT
    if (ptr < end && (*ptr == 'e' || *ptr == 'E')) {
        isInt = false;
        ++ptr;
        if (ptr < end && (*ptr == '-' || *ptr == '+'))
            ++ptr;
        while (ptr < end && isAsciiDigit(*ptr))
            ++ptr;
    }

    const QByteArray number = QByteArray::fromRawData(start, ptr - start);

    if (isInt) {
        bool ok;
        qlonglong n = number.toLongLong(&ok);
        if (ok) {
            return QVariant(n);
        }
    }

    bool ok;
    double d = number.toDouble(&ok);

    if (!ok) {
        return QVariant();
    }

    return QVariant(d);
}

static QVariant parseJsonValue(const char* &ptr, const char* end);

static QVariantList parseJsonArray(const char* &ptr, const char* end)
{
    QVariantList list;
    ++ptr; // skip '['
    skipWhitespace(ptr, end);
    while (ptr < end && *ptr != ']') {
        list.append(parseJsonValue(ptr, end));
        skipWhitespace(ptr, end);
        if (*ptr == ',') {
            ++ptr;
            skipWhitespace(ptr, end);
        }
    }
    if (*ptr == ']') ++ptr;
    return list;
}

static QVariantMap parseJsonObject(const char* &ptr, const char* end)
{
    QVariantMap map;
    ++ptr; // skip '{'
    skipWhitespace(ptr, end);
    while (ptr < end && *ptr != '}') {
        if (*ptr != '"') return QVariantMap();
        ++ptr;
        QString key = parseJsonString(ptr, end);
        skipWhitespace(ptr, end);
        if (*ptr == ':') ++ptr;
        skipWhitespace(ptr, end);
        map.insert(std::move(key), parseJsonValue(ptr, end));
        skipWhitespace(ptr, end);
        if (*ptr == ',') {
            ++ptr;
            skipWhitespace(ptr, end);
        }
    }
    if (*ptr == '}') ++ptr;
    return map;
}

static QVariant parseJsonValue(const char* &ptr, const char* end)
{
    skipWhitespace(ptr, end);
    if (ptr >= end) return QVariant();

    switch (*ptr) {
    case 'n':
        ++ptr;
        if (end - ptr < 3) {
            return QVariant();
        }
        if (*ptr++ == 'u' &&
            *ptr++ == 'l' &&
            *ptr++ == 'l') {
            return QVariant::fromValue(nullptr);
        }
        return QVariant();
    case 't':
        ++ptr;
        if (end - ptr < 3) {
            return QVariant();
        }
        if (*ptr++ == 'r' &&
            *ptr++ == 'u' &&
            *ptr++ == 'e') {
            return QVariant(true);
        }
        return QVariant();
    case 'f':
        ++ptr;
        if (end - ptr < 4) {
            return QVariant();
        }
        if (*ptr++ == 'a' &&
            *ptr++ == 'l' &&
            *ptr++ == 's' &&
            *ptr++ == 'e') {
            return QVariant(false);
        }
        return QVariant();
    case BeginArray:
        return parseJsonArray(ptr, end);
    case BeginObject:
        return parseJsonObject(ptr, end);
    case Quote:
        ++ptr;
        return parseJsonString(ptr, end);
    default:
        return parseJsonNumber(ptr, end);
    }
}

QVariant parseJson(const QByteArray &json)
{
    const char* ptr = json.constData();
    const char* end = ptr + json.size();
    skipByteOrderMark(ptr, end);
    return parseJsonValue(ptr, end);
}

QVariant QtJson::Parser::jsonToVariant(const QByteArray& json, QJsonParseError* error)
{
    return ::parseJson(json);
}

QVariant QtJson::Parser::jsonStreamToVariant(QIODevice* device, QJsonParseError* error)
{
    if (!device || !device->isOpen())
        return QVariant();

    return ::parseJson(device->readAll());
}
