#include "qcborparser.h"
#include <QCborValue>
#include <QCborStreamReader>

static QVariant variantFromCbor(QCborStreamReader &reader);

static QVariantList arrayFromCbor(QCborStreamReader &reader)
{
    QVariantList list;
    if (reader.isLengthKnown())
        list.reserve(reader.length());

    reader.enterContainer();
    while (reader.lastError() == QCborError::NoError && reader.hasNext()) {
        list.append(variantFromCbor(reader));
    }
    if (reader.lastError() == QCborError::NoError)
        reader.leaveContainer();

    list.squeeze();

    return list;
}

static QVariantMap objectFromCbor(QCborStreamReader &reader)
{
    QVariantMap map;
    // if (reader.isLengthKnown())
    //     map.reserve(reader.length());

    reader.enterContainer();
    while (reader.lastError() == QCborError::NoError && reader.hasNext()) {
        QString key = variantFromCbor(reader).toString();
        map.insert(std::move(key), variantFromCbor(reader));
    }
    if (reader.lastError() == QCborError::NoError)
        reader.leaveContainer();

    return map;
}

static QVariant variantValueFromCbor(QCborStreamReader &reader)
{
    switch (reader.type()) {
    case QCborStreamReader::ByteArray:
        return reader.readAllByteArray();
    case QCborStreamReader::String:
        return reader.readAllString();
    default:
        return QCborValue::fromCbor(reader).toVariant();
    }
}

QVariant variantFromCbor(QCborStreamReader &reader)
{
    switch (reader.type()) {
    case QCborStreamReader::Array:
        return arrayFromCbor(reader);
    case QCborStreamReader::Map:
        return objectFromCbor(reader);
    default:
        return variantValueFromCbor(reader);
        break;
    }
}

QVariant QtCbor::Parser::cborToVariant(const QByteArray& cbor, QCborParserError* error)
{
    QCborStreamReader reader(cbor);
    return variantFromCbor(reader);
}

QVariant QtCbor::Parser::cborStreamToVariant(QIODevice* device, QCborParserError* error)
{
    QCborStreamReader reader(device);
    return variantFromCbor(reader);
}
