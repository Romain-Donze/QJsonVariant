#include "qcborvariantwriter.h"
#include <QCborStreamWriter>
#include <QCborValue>
#include <QIODevice>

static void variantToCbor(const QVariant &value, QCborStreamWriter &writer, int opt);

template<typename T>
static inline void variantListToCbor(const QList<T>& array, QCborStreamWriter &writer, int opt)
{
    writer.startArray(array.size());
    for(const T& variant: array) {
        variantToCbor(variant, writer, opt);
    }
    writer.endArray();
}
template<typename T>
static inline void variantObjectToCbor(const T& object, QCborStreamWriter &writer, int opt)
{
    writer.startMap(object.size());
    auto it = object.begin();
    auto end = object.end();
    for ( ; it != end; ++it) {
        writer.append(it.key());
        variantToCbor(it.value(), writer, opt);
    }
    writer.endMap();
}
static inline void variantValueToCbor(const QVariant &value, QCborStreamWriter &writer, int opt)
{
    switch (value.metaType().id()) {
    case QMetaType::QByteArray:
        writer.append(value.toByteArray());
        break;
    case QMetaType::QString:
        writer.append(value.toString());
        break;
    default:
        QCborValue::fromVariant(value).toCbor(writer, (QCborValue::EncodingOptions)opt);
        break;
    }
}
void variantToCbor(const QVariant &value, QCborStreamWriter &writer, int opt)
{
    switch (value.metaType().id()) {
    case QMetaType::QStringList: {
        variantListToCbor(value.toStringList(), writer, opt);
        break;
    }
    case QMetaType::QVariantList: {
        variantListToCbor(value.toList(), writer, opt);
        break;
    }
    case QMetaType::QVariantMap: {
        variantObjectToCbor(value.toMap(), writer, opt);
        break;
    }
    case QMetaType::QVariantHash: {
        variantObjectToCbor(value.toHash(), writer, opt);
        break;
    }
    default: {
        variantValueToCbor(value, writer, opt);
        break;
    }
    }
}

QCborVariantWriter::QCborVariantWriter(QIODevice *device, int options):
    m_device(new QCborStreamWriter(device)),
    m_options(options)
{

}

QCborVariantWriter::QCborVariantWriter(QByteArray *data, int options):
    m_device(new QCborStreamWriter(data)),
    m_options(options)
{

}

QCborVariantWriter::~QCborVariantWriter()
{
    delete m_device;
}

void QCborVariantWriter::start()
{
    m_device->device()->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
}
void QCborVariantWriter::startArray()
{
    m_device->startArray();
}
void QCborVariantWriter::startArray(quint64 count)
{
    m_device->startArray(count);
}
void QCborVariantWriter::endArray()
{
    m_device->endArray();
}
void QCborVariantWriter::startMap()
{
    m_device->startMap();
}
void QCborVariantWriter::startMap(quint64 count)
{
    m_device->startMap(count);
}
void QCborVariantWriter::endMap()
{
    m_device->endMap();
}

void QCborVariantWriter::writeKeyValue(const QString& key, const QVariant& value)
{
    writeString(key);
    writeVariant(value);
}

void QCborVariantWriter::writeString(QStringView s)
{
    m_device->append(s);
}
void QCborVariantWriter::writeRaw(const char *data, qint64 len)
{
    m_device->device()->write(data, len);
}
void QCborVariantWriter::writeRaw(const char *data)
{
    m_device->device()->write(data);
}
void QCborVariantWriter::writeRaw(const QByteArray &data)
{
    m_device->device()->write(data);
}
void QCborVariantWriter::writeVariant(const QVariant &v)
{
    ::variantToCbor(v, *m_device, m_options);
}

QByteArray QCborVariantWriter::fromVariant(const QVariant& variant, int options)
{
    QByteArray cbor;
    QCborVariantWriter writer(&cbor, options);

    writer.start();
    writer.writeVariant(variant);

    cbor.squeeze();

    return cbor;
}

void QCborVariantWriter::fromVariant(const QVariant& variant, QIODevice* device, int options)
{
    QCborVariantWriter writer(device, options);

    writer.start();
    writer.writeVariant(variant);
}

