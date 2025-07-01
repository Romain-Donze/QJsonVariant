#ifndef QUTF8_H
#define QUTF8_H

#include <QString>

namespace QUtf8 {

static inline bool isAsciiDigit(char32_t c)
{
    return c >= '0' && c <= '9';
}
static inline uchar hexdig(uint u)
{
    return (u < 0xa ? '0' + u : 'a' + u - 0xa);
}
static inline QByteArray escapedString(QStringView s)
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
            ba.resize(ba.size() * 2);
            cursor = reinterpret_cast<uchar *>(ba.data()) + pos;
            ba_end = ba_const_start() + ba.size();
        }

        char16_t u = *src++;
        if (u < 0x80) {
            if (u < 0x20 || u == 0x22 || u == 0x5c) {
                ensureCapacity(6);
                *cursor++ = '\\';
                switch (u) {
                case 0x22:
                    *cursor++ = '"';
                    break;
                case 0x5c:
                    *cursor++ = '\\';
                    break;
                case 0x08:
                    *cursor++ = 'b';
                    break;
                case 0x0c:
                    *cursor++ = 'f';
                    break;
                case 0x0a:
                    *cursor++ = 'n';
                    break;
                case 0x0d:
                    *cursor++ = 'r';
                    break;
                case 0x09:
                    *cursor++ = 't';
                    break;
                default:
                    *cursor++ = 'u';
                    *cursor++ = '0';
                    *cursor++ = '0';
                    *cursor++ = hexdig(u >> 4);
                    *cursor++ = hexdig(u & 0xf);
                }
            } else {
                ensureCapacity(1);
                *cursor++ = (uchar) u;
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

static inline QString unescapedString(const QByteArray &ba)
{
    QByteArray decoded;
    decoded.reserve(ba.size());
    const char *src = ba.constData();
    const char *end = src + ba.size();

    while (src < end) {
        if (*src == '\\' && src + 1 < end) {
            ++src;
            switch (*src) {
            case 'b':
                decoded.append('\b');
                break;
            case 'f':
                decoded.append('\f');
                break;
            case 'n':
                decoded.append('\n');
                break;
            case 'r':
                decoded.append('\r');
                break;
            case 't':
                decoded.append('\t');
                break;
            case '"':
                decoded.append('\"');
                break;
            case '\\':
                decoded.append('\\');
                break;
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
                decoded.append(*src);
                break;
            }
        } else {
            decoded.append(*src);
        }
        ++src;
    }

    decoded.squeeze();

    return QString::fromUtf8(decoded);
}
} // namespace QUtf8

#endif // QUTF8_H
