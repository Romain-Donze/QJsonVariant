#ifndef QUTF8_H
#define QUTF8_H

#include <QIODevice>
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
inline void escapedString(QIODevice *dev, QStringView s)
{
    if (!dev || s.isEmpty())
        return;

    constexpr qsizetype BufferSize = 1024;
    char buffer[BufferSize];
    char *cursor = buffer;

    auto flushBuffer = [&]() {
        if (cursor > buffer) {
            dev->write(buffer, cursor - buffer);
            cursor = buffer;
        }
    };

    const char16_t *src = s.utf16();
    const char16_t *end = src + s.size();

    while (src != end) {
        char16_t u = *src++;

        if (u < 0x80) {
            // ASCII
            if (u < 0x20 || u == '"' || u == '\\') {
                // Escape sequence, need up to 6 bytes (\u00XX)
                if (cursor + 6 > buffer + BufferSize)
                    flushBuffer();

                *cursor++ = '\\';
                switch (u) {
                case '"':  *cursor++ = '"'; break;
                case '\\': *cursor++ = '\\'; break;
                case '\b': *cursor++ = 'b'; break;
                case '\f': *cursor++ = 'f'; break;
                case '\n': *cursor++ = 'n'; break;
                case '\r': *cursor++ = 'r'; break;
                case '\t': *cursor++ = 't'; break;
                default:
                    *cursor++ = 'u';
                    *cursor++ = '0';
                    *cursor++ = '0';
                    *cursor++ = hexdig(u >> 4);
                    *cursor++ = hexdig(u & 0xF);
                    break;
                }
            } else {
                // Regular ASCII
                if (cursor == buffer + BufferSize)
                    flushBuffer();
                *cursor++ = static_cast<char>(u);
            }
        } else {
            // Non-ASCII: UTF-8 encoding
            char utf8[4];
            int len = 0;

            if (u >= 0xD800 && u <= 0xDBFF && src < end) {
                // High surrogate
                char16_t low = *src;
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    ++src;
                    char32_t cp = 0x10000 + (((u - 0xD800) << 10) | (low - 0xDC00));
                    utf8[0] = 0xF0 | ((cp >> 18) & 0x07);
                    utf8[1] = 0x80 | ((cp >> 12) & 0x3F);
                    utf8[2] = 0x80 | ((cp >> 6) & 0x3F);
                    utf8[3] = 0x80 | (cp & 0x3F);
                    len = 4;
                } else {
                    u = 0xFFFD; // invalid surrogate
                }
            } else if (u >= 0xDC00 && u <= 0xDFFF) {
                u = 0xFFFD; // lone low surrogate
            }

            if (len == 0) {
                // BMP character
                if (u <= 0x7FF) {
                    utf8[0] = 0xC0 | ((u >> 6) & 0x1F);
                    utf8[1] = 0x80 | (u & 0x3F);
                    len = 2;
                } else {
                    utf8[0] = 0xE0 | ((u >> 12) & 0x0F);
                    utf8[1] = 0x80 | ((u >> 6) & 0x3F);
                    utf8[2] = 0x80 | (u & 0x3F);
                    len = 3;
                }
            }

            if (cursor + len > buffer + BufferSize)
                flushBuffer();

            for (int i = 0; i < len; ++i)
                *cursor++ = utf8[i];
        }
    }

    flushBuffer();
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
