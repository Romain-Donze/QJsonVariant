#include "qcborvariantreader.h"
#include <QCborValue>
#include <QCborStreamReader>

QCborVariantReader::QCborVariantReader(QIODevice *device):
    QCborVariantReader(device->readAll())
{

}

QCborVariantReader::QCborVariantReader(const QByteArray &data):
    m_device(new QCborStreamReader(data)),
    m_size(data.size())
{

}

QCborVariantReader::~QCborVariantReader()
{
    delete m_device;
}

QCborStreamReader* QCborVariantReader::device() const
{
    return m_device;
}

bool QCborVariantReader::atEnd()
{
    return currentOffset() >= m_size;
}

QVariantReader::Type QCborVariantReader::type() const
{
    switch (m_device->type()) {
    case QCborStreamReader::Array:
        return QVariantReader::List;
    case QCborStreamReader::Map:
        return QVariantReader::Map;
    case QCborStreamReader::Invalid:
        return QVariantReader::Invalid;
    default:
        return QVariantReader::Value;
        break;
    }
}

QString QCborVariantReader::readString()
{
    QString key = m_device->readString().data;
    m_device->next();
    return key;
    // return m_device->readAllString();
}

QVariant QCborVariantReader::readValue()
{
    switch (m_device->type()) {
    case QCborStreamReader::ByteArray:
        return m_device->readAllByteArray();
    case QCborStreamReader::String:
        return m_device->readAllString();
    default:
        return QCborValue::fromCbor(*m_device).toVariant();
    }
}

QCborParserError QCborVariantReader::error() const
{
    QCborParserError error;
    error.error = lastError();
    error.offset = currentOffset();
    return error;
}

QVariant QCborVariantReader::fromCbor(const QByteArray& cbor, QCborParserError* error)
{
    QCborVariantReader reader(cbor);
    QVariant variant = reader.read();
    if(error)
        *error = reader.error();
    return variant;
}

QVariant QCborVariantReader::fromCbor(QIODevice* device, QCborParserError* error)
{
    QCborVariantReader reader(device);
    QVariant variant = reader.read();
    if(error)
        *error = reader.error();
    return variant;
}
