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

static inline QByteArray escapedString(const QString& s)
{
    QByteArray utf8 = s.toUtf8();
    QByteArray escaped;
    escaped.reserve(utf8.size());

    for (uchar c : std::as_const(utf8)) {
        switch (c) {
        case '\"': escaped.append("\\\""); break;
        case '\\': escaped.append("\\\\"); break;
        case '\b': escaped.append("\\b");  break;
        case '\f': escaped.append("\\f");  break;
        case '\n': escaped.append("\\n");  break;
        case '\r': escaped.append("\\r");  break;
        case '\t': escaped.append("\\t");  break;
        default:
            if (c < 0x20) {
                // encode control chars as \u00XX
                escaped.append(QString("\\u%1").arg(c, 4, 16, QLatin1Char('0')).toUtf8());
            } else {
                escaped.append(c);
            }
            break;
        }
    }

    return escaped;
}
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
        d->write("null");
        break;
    case QMetaType::QString:
    case QMetaType::QDateTime:
        stringToJson(value.toString(), d);
        break;
    default:
        if(!value.isValid()) {
            d->write("null");
            break;
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
        stringToJson(value.toString(), d);
#else
        d->write(QJsonValue::fromVariant(value).toJson(QJsonDocument::Compact));
#endif
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
        d->write("]");
        break;
    }
    case QMetaType::QVariantList: {
        d->write(compact ? "[" : "[\n");
        variantListToJson(value.toList(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write("]");
        break;
    }
    case QMetaType::QVariantMap: {
        d->write(compact ? "{" : "{\n");
        variantObjectToJson(value.toMap(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write("}");
        break;
    }
    case QMetaType::QVariantHash: {
        d->write(compact ? "{" : "{\n");
        variantObjectToJson(value.toHash(), d, indent + (compact ? 0 : 1), compact);
        d->write(QByteArray(4*indent, ' '));
        d->write("}");
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

    if (!compact) {

        switch (variant.metaType().id()) {
        case QMetaType::QStringList:
        case QMetaType::QVariantList:
        case QMetaType::QVariantMap:
        case QMetaType::QVariantHash:
            buffer.write("\n");
            break;
        default:
            break;
        }
    }

    return json;
}

void QtJson::Writer::variantToJsonStream(const QVariant& variant, QIODevice* device, bool compact)
{
    *g_showType = false;

    ::variantToJson(variant, device, 0, compact);

    if (!compact) {

        switch (variant.metaType().id()) {
        case QMetaType::QStringList:
        case QMetaType::QVariantList:
        case QMetaType::QVariantMap:
        case QMetaType::QVariantHash:
            device->write("\n");
            break;
        default:
            break;
        }
    }

}

