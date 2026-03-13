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
static inline QByteArray escapedString(QStringView s)
{
    QByteArray ba(std::max(s.size(), qsizetype(16)), Qt::Uninitialized);
    auto ba_const_start = [&]() { return reinterpret_cast<const uchar *>(ba.constData()); };
    uchar *cursor = reinterpret_cast<uchar *>(ba.data());
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

    auto hexValue = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return -1;
    };

    auto readHex4 = [&](const char *p, char16_t &out) -> bool {
        int v0 = hexValue(p[0]);
        int v1 = hexValue(p[1]);
        int v2 = hexValue(p[2]);
        int v3 = hexValue(p[3]);
        if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0)
            return false;
        out = static_cast<char16_t>((v0 << 12) | (v1 << 8) | (v2 << 4) | v3);
        return true;
    };

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
                    char16_t first = 0;
                    if (readHex4(src + 1, first)) {
                        if (first >= 0xD800 && first <= 0xDBFF && src + 10 < end && src[5] == '\\' && src[6] == 'u') {
                            char16_t second = 0;
                            if (readHex4(src + 7, second) && second >= 0xDC00 && second <= 0xDFFF) {
                                const char32_t cp = 0x10000u + ((char32_t(first - 0xD800) << 10) | char32_t(second - 0xDC00));
                                decoded.append(QString::fromUcs4(&cp, 1).toUtf8());
                                src += 10;
                                break;
                            }
                        }

                        decoded.append(QString::fromUtf16(&first, 1).toUtf8());
                        src += 4;
                        break;
                    }
                }
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
