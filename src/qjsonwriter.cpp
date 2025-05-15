#include "qjsonwriter.h"
#include <QBuffer>
#include <QIODevice>
#include <QStringList>
#include <QLocale>
#include <QJsonParseError>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonValue>

Q_GLOBAL_STATIC_WITH_ARGS(bool, g_showType, (false))

static void variantToJson(const QVariant &value, QIODevice *d, int indent, bool compact);

static inline char hexdig(int val)
{
    return val < 10 ? '0' + val : 'a' + (val - 10);
}
static QByteArray escapedString(QStringView s)
{
    QByteArray ba(qMax(s.size(), 16), Qt::Uninitialized);
    auto ba_const_start = [&]() { return reinterpret_cast<const uchar *>(ba.constData()); };
    uchar *cursor = reinterpret_cast<uchar *>(const_cast<char *>(ba.constData()));
    const uchar *ba_end = cursor + ba.size();

    const char16_t *src = s.utf16();
    const char16_t *const end = s.utf16() + s.size();

    auto ensureCapacity = [&](int extra) {
        if (cursor + extra >= ba_end) {
            qptrdiff pos = cursor - ba_const_start();
            ba.resize(ba.size() * 2 + extra);
            cursor = reinterpret_cast<uchar *>(ba.data()) + pos;
            ba_end = ba_const_start() + ba.size();
        }
    };

    while (src != end) {
        if (cursor >= ba_end - 6) {
            // ensure we have enough space
            qptrdiff pos = cursor - ba_const_start();
            ba.resize(ba.size()*2);
            cursor = reinterpret_cast<uchar *>(ba.data()) + pos;
            ba_end = ba_const_start() + ba.size();
        }

        char16_t u = *src++;
        if (u < 0x80) {
            if (u < 0x20 || u == 0x22 || u == 0x5c) {
                ensureCapacity(6);
                *cursor++ = '\\';
                switch (u) {
                case 0x22: *cursor++ = '"'; break;
                case 0x5c: *cursor++ = '\\'; break;
                case 0x08: *cursor++ = 'b'; break;
                case 0x0c: *cursor++ = 'f'; break;
                case 0x0a: *cursor++ = 'n'; break;
                case 0x0d: *cursor++ = 'r'; break;
                case 0x09: *cursor++ = 't'; break;
                default:
                    *cursor++ = 'u';
                    *cursor++ = '0';
                    *cursor++ = '0';
                    *cursor++ = hexdig(u>>4);
                    *cursor++ = hexdig(u & 0xf);
                }
            } else {
                ensureCapacity(1);
                *cursor++ = (uchar)u;
            }
        } else {
            if (u >= 0xD800 && u <= 0xDBFF) {
                // high surrogate
                if (src < end) {
                    char16_t low = *src;
                    if (low >= 0xDC00 && low <= 0xDFFF) {
                        ++src;
                        // Valid surrogate pair
                        char32_t cp = 0x10000 + (((u - 0xD800) << 10) | (low - 0xDC00));
                        ensureCapacity(4);
                        *cursor++ = 0xF0 | (cp >> 18);
                        *cursor++ = 0x80 | ((cp >> 12) & 0x3F);
                        *cursor++ = 0x80 | ((cp >> 6) & 0x3F);
                        *cursor++ = 0x80 | (cp & 0x3F);
                        continue;
                    }
                }
                // Invalid surrogate, encode as \uFFFD
                u = 0xFFFD;
            } else if (u >= 0xDC00 && u <= 0xDFFF) {
                // Lone low surrogate – invalid
                u = 0xFFFD;
            }

            // Encode BMP character (0x80 - 0xFFFF, not part of surrogate pair)
            if (u <= 0x7FF) {
                ensureCapacity(2);
                *cursor++ = 0xC0 | (u >> 6);
                *cursor++ = 0x80 | (u & 0x3F);
            } else {
                ensureCapacity(3);
                *cursor++ = 0xE0 | (u >> 12);
                *cursor++ = 0x80 | ((u >> 6) & 0x3F);
                *cursor++ = 0x80 | (u & 0x3F);
            }
        }
    }

    ba.resize(cursor - ba_const_start());
    return ba;
}

// static inline QByteArray escapedString(const QString& s)
// {
//     QByteArray utf8 = s.toUtf8();
//     QByteArray escaped;
//     escaped.reserve(utf8.size());

//     for (uchar c : std::as_const(utf8)) {
//         switch (c) {
//         case '\"': escaped.append("\\\""); break;
//         case '\\': escaped.append("\\\\"); break;
//         case '\b': escaped.append("\\b");  break;
//         case '\f': escaped.append("\\f");  break;
//         case '\n': escaped.append("\\n");  break;
//         case '\r': escaped.append("\\r");  break;
//         case '\t': escaped.append("\\t");  break;
//         default:
//             escaped.append(c);
//             break;
//         }
//     }

//     escaped.squeeze();

//     return escaped;
// }
static inline void stringToJson(const QString &string, QIODevice *d)
{
    d->write("\"");
    d->write(escapedString(string));
    d->write("\"");
}

template<typename T>
static inline void variantListToJson(const QList<T>& array, QIODevice *d, int indent, bool compact)
{
    QByteArray indentString(4*indent, ' ');
    qsizetype i = 0;
    for(const T& variant: array) {
        d->write(indentString);
        variantToJson(variant, d, indent, compact);
        if (++i == array.size()) {
            if (!compact)
                d->write("\n");
            break;
        }
        d->write(compact ? "," : ",\n");
    }
}
template<typename T>
static inline void variantObjectToJson(const T& object, QIODevice *d, int indent, bool compact)
{
    QByteArray indentString(4*indent, ' ');
    qsizetype i = 0;
    auto it = object.begin();
    auto end = object.end();
    for ( ; it != end; ++it) {
        d->write(indentString);
        stringToJson(it.key(), d);
        d->write(compact ? ":" : ": ");
        variantToJson(it.value(), d, indent, compact);
        if (++i == object.size()) {
            if (!compact)
                d->write("\n");
            break;
        }
        d->write(compact ? "," : ",\n");
    }
}
static inline void variantValueToJson(const QVariant &value, QIODevice *d)
{
    switch (value.metaType().id()) {
    case QMetaType::Bool:
        if(value.toBool())
            d->write("true");
        else
            d->write("false");
        break;
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::Long:
    case QMetaType::UInt:
        d->write(QByteArray::number(value.toLongLong()));
        break;
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        if (value.toULongLong() <= static_cast<uint64_t>(std::numeric_limits<qint64>::max())) {
            d->write(QByteArray::number(value.toULongLong()));
            break;
        }
        Q_FALLTHROUGH();
    case QMetaType::Float16:
    case QMetaType::Float:
    case QMetaType::Double: {
        const double val = value.toDouble();
        if (qIsFinite(val))
            d->write(QByteArray::number(val, 'g', QLocale::FloatingPointShortest));
        else
            d->write("null"); // +INF || -INF || NaN (see RFC4627#section2.4)
        break;
    }
    case QMetaType::Nullptr:
    case QMetaType::QString:
    case QMetaType::QDateTime:
    default:
        if(value.isNull() || !value.isValid()) {
            d->write("null");
            break;
        }
        stringToJson(value.toString(), d);
        break;
    }
}
void variantToJson(const QVariant &value, QIODevice *d, int indent, bool compact)
{
    switch (value.metaType().id()) {
    case QMetaType::QStringList: {
        d->write(compact ? "[" : "[\n");
        variantListToJson(value.toStringList(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write((compact || indent) ? "]" : "]\n");
        break;
    }
    case QMetaType::QVariantList: {
        d->write(compact ? "[" : "[\n");
        variantListToJson(value.toList(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write((compact || indent) ? "]" : "]\n");
        break;
    }
    case QMetaType::QVariantMap: {
        d->write(compact ? "{" : "{\n");
        variantObjectToJson(value.toMap(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write((compact || indent) ? "}" : "}\n");
        break;
    }
    case QMetaType::QVariantHash: {
        d->write(compact ? "{" : "{\n");
        variantObjectToJson(value.toHash(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write((compact || indent) ? "}" : "}\n");
        break;
    }
    default: {
        variantValueToJson(value, d);
        break;
    }
    }

    if(*g_showType) {
        d->write(compact ? "" : " ");
        d->write(QString("(%1)").arg(value.metaType().name()).toUtf8());
    }
}

QByteArray QtJson::Writer::variantToJsonDebug(const QVariant& variant, bool compact)
{
    *g_showType = true;

    QByteArray debug;
    QBuffer buffer(&debug);
    buffer.open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    ::variantToJson(variant, &buffer, 0, compact);

    if(!compact)
    {
        debug.prepend("\n");
        debug.prepend(QByteArray(100,'-'));
        debug.prepend("\n");

        debug.append(QByteArray(100,'-'));
        debug.append("\n");
    }

    return debug;
}

QByteArray QtJson::Writer::variantToJson(const QVariant& variant, bool compact)
{
    *g_showType = false;

    QByteArray json;
    QBuffer buffer(&json);
    buffer.open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    ::variantToJson(variant, &buffer, 0, compact);

    json.squeeze();

    return json;
}

void QtJson::Writer::variantToJsonStream(const QVariant& variant, QIODevice* device, bool compact)
{
    *g_showType = false;

    ::variantToJson(variant, device, 0, compact);
}

