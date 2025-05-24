#include "qjsonvariantwriter.h"
#include <QBuffer>
#include <QIODevice>
#include <QLocale>

#include "qutf8.h"

Q_GLOBAL_STATIC_WITH_ARGS(bool, g_showType, (false))

static void variantToJson(const QVariant &value, QIODevice *d, int indent, bool compact);

static inline void stringToJson(const QString &string, QIODevice *d)
{
    d->write("\"");
    d->write(QUtf8::escapedString(string));
    d->write("\"");
}

static inline void startArray(QIODevice *d, int& indent, bool compact)
{
    d->write(compact ? "[" : "[\n");
    indent = indent + (compact ? 0 : 1);
}
static inline void endArray(QIODevice *d, int& indent, bool compact)
{
    indent = indent - (compact ? 0 : 1);
    d->write(QByteArray(4*indent, ' '));
    d->write((compact || indent) ? "]" : "]\n");
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

static inline void startMap(QIODevice *d, int& indent, bool compact)
{
    d->write(compact ? "{" : "{\n");
    indent = indent + (compact ? 0 : 1);
}
static inline void endMap(QIODevice *d, int& indent, bool compact)
{
    indent = indent - (compact ? 0 : 1);
    d->write(QByteArray(4*indent, ' '));
    d->write((compact || indent) ? "}" : "}\n");
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
        startArray(d, indent, compact);
        variantListToJson(value.toStringList(), d, indent, compact);
        endArray(d, indent, compact);
        break;
    }
    case QMetaType::QVariantList: {
        startArray(d, indent, compact);
        variantListToJson(value.toList(), d, indent, compact);
        endArray(d, indent, compact);
        break;
    }
    case QMetaType::QVariantMap: {
        startMap(d, indent, compact);
        variantObjectToJson(value.toMap(), d, indent, compact);
        endMap(d, indent, compact);
        break;
    }
    case QMetaType::QVariantHash: {
        startMap(d, indent, compact);
        variantObjectToJson(value.toHash(), d, indent, compact);
        endMap(d, indent, compact);
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

QJsonVariantWriter::QJsonVariantWriter(QIODevice *device, bool compact):
    m_device(device),
    m_deleteDevice(false),
    m_compact(compact),
    m_indent(0)
{
    *g_showType = false;
}

QJsonVariantWriter::QJsonVariantWriter(QByteArray *data, bool compact):
    QJsonVariantWriter(new QBuffer(data), compact)
{
    m_device->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    m_deleteDevice = true;
}

QJsonVariantWriter::~QJsonVariantWriter()
{
    if (m_deleteDevice)
        delete m_device;
}

void QJsonVariantWriter::start()
{
    *g_showType = false;
    m_device->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    m_indent = 0;
}
void QJsonVariantWriter::startArray()
{
    ::startArray(m_device, m_indent, m_compact);
}
void QJsonVariantWriter::endArray()
{
    ::endArray(m_device, m_indent, m_compact);
}
void QJsonVariantWriter::startMap()
{
    ::startMap(m_device, m_indent, m_compact);
}
void QJsonVariantWriter::endMap()
{
    ::endMap(m_device, m_indent, m_compact);
}

void QJsonVariantWriter::writeKeyValue(QStringView key, const QVariant& value)
{
    writeString(key);
    writeNameSeparator();
    writeVariant(value);
}
void QJsonVariantWriter::writeNameSeparator()
{
    m_device->write(m_compact ? ":" : ": ");
}
void QJsonVariantWriter::writeValueSeparator()
{
    m_device->write(m_compact ? "," : ",\n");
}

void QJsonVariantWriter::writeString(QStringView s)
{
    m_device->write("\"");
    m_device->write(QUtf8::escapedString(s));
    m_device->write("\"");
}
void QJsonVariantWriter::writeRaw(const char *data, qint64 len)
{
    m_device->write(data, len);
}
void QJsonVariantWriter::writeRaw(const char *data)
{
    m_device->write(data);
}
void QJsonVariantWriter::writeRaw(const QByteArray &data)
{
    m_device->write(data);
}
void QJsonVariantWriter::writeVariant(const QVariant &v)
{
    ::variantToJson(v, m_device, m_indent, m_compact);
}

QByteArray QJsonVariantWriter::fromVariant(const QVariant& variant, bool compact)
{
    QByteArray json;
    QJsonVariantWriter writer(&json, compact);

    writer.start();
    writer.writeVariant(variant);

    json.squeeze();

    return json;
}

void QJsonVariantWriter::fromVariant(const QVariant& variant, QIODevice* device, bool compact)
{
    QJsonVariantWriter writer(device, compact);

    writer.start();
    writer.writeVariant(variant);
}

QByteArray QJsonVariantWriter::escapedString(QStringView s)
{
    return QUtf8::escapedString(s);
}

QByteArray QJsonVariantWriter::fromVariantDebug(const QVariant& variant, bool compact)
{
    QByteArray json;
    QJsonVariantWriter writer(&json, compact);

    writer.start();
    *g_showType = true;
    writer.writeRaw("\n");
    writer.writeRaw(QByteArray(100,'-'));
    writer.writeRaw("\n");
    writer.writeVariant(variant);
    writer.writeRaw("\n");
    writer.writeRaw(QByteArray(100,'-'));

    json.squeeze();

    return json;
}
