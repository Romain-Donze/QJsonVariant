#include "qcborparser.h"
#include <QCborValue>
#include <QCborStreamReader>

static QVariant variantFromCbor(QCborStreamReader &reader);

QVariantList arrayFromCbor(QCborStreamReader &reader)
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

    return list;
}

QVariantMap objectFromCbor(QCborStreamReader &reader)
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

QVariant variantValueFromCbor(QCborStreamReader &reader)
{
    switch (reader.type()) {
    case QCborStreamReader::UnsignedInteger: {
        quint64 num = reader.toUnsignedInteger();
        reader.next();
        return std::move(num);
    }
    case QCborStreamReader::NegativeInteger: {
        qint64 num = reader.toInteger();
        reader.next();
        return std::move(num);
    }
    case QCborStreamReader::Float16: {
        float num = reader.toFloat16();
        reader.next();
        return std::move(num);
    }
    case QCborStreamReader::Float: {
        float num = reader.toFloat();
        reader.next();
        return std::move(num);
    }
    case QCborStreamReader::Double: {
        double num = reader.toDouble();
        reader.next();
        return std::move(num);
    }
    case QCborStreamReader::SimpleType: {
        QCborSimpleType t = reader.toSimpleType();
        reader.next();
        switch (t) {
        case QCborSimpleType::False:
            return false;
        case QCborSimpleType::True:
            return true;
        case QCborSimpleType::Null:
            return QVariant::fromValue(nullptr);
        case QCborSimpleType::Undefined:
        default:
            return QVariant();
        }
        break;
    }
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

    const QCborValue cVal = QCborValue::fromCbor(reader);
    if (error) {
        error->error = reader.lastError();
        error->offset = reader.currentOffset();
    }
    return cVal.toVariant();
}

QVariant QtCbor::Parser::cborStreamToVariant(QIODevice* device, QCborParserError* error)
{
    QCborStreamReader reader(device);
    const QCborValue cVal = QCborValue::fromCbor(reader);
    if (error) {
        error->error = reader.lastError();
        error->offset = reader.currentOffset();
    }
    return cVal.toVariant();
}
