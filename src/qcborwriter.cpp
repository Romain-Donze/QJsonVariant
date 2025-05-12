#include "qcborwriter.h"
#include <QCborStreamWriter>
#include <QCborValue>

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
    case QMetaType::Bool:
        writer.append(value.toBool());
        break;
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::Long:
    case QMetaType::UInt:
        writer.append(value.toLongLong());
        break;
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        if (value.toULongLong() <= static_cast<uint64_t>(std::numeric_limits<qint64>::max())) {
            writer.append(value.toULongLong());
            break;
        }
        Q_FALLTHROUGH();
    case QMetaType::Float16:
    case QMetaType::Float:
    case QMetaType::Double:
        QCborValue(value.toDouble()).toCbor(writer, (QCborValue::EncodingOptions)opt);
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

QByteArray QtCbor::Writer::variantToCbor(const QVariant& variant, int opt)
{
    QByteArray cbor;
    QCborStreamWriter writer(&cbor);
    ::variantToCbor(variant, writer, opt);

    cbor.squeeze();

    return cbor;
}

void QtCbor::Writer::variantToCborStream(const QVariant& variant, QIODevice* device, int opt)
{
    QCborStreamWriter writer(device);
    ::variantToCbor(variant, writer, opt);
}

