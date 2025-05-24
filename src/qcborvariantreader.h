#ifndef QCBORVARIANTREADER_H
#define QCBORVARIANTREADER_H

#include "qvariantreader.h"
#include <QCborStreamReader>
#include <QCborParserError>

class QCborVariantReader: public QVariantReader
{
public:
    explicit QCborVariantReader(QIODevice *device);
    explicit QCborVariantReader(const QByteArray &data);
    ~QCborVariantReader();
    Q_DISABLE_COPY(QCborVariantReader)

    int currentProgress() const final override;
    qint64 currentOffset() const final override { return m_device->currentOffset(); }

    bool hasError() final override { return lastError() != QCborError::NoError; };
    bool hasNext() const final override { return m_device->hasNext(); }
    bool next() final override { return m_device->next(); }
    bool atEnd() final override;

    QVariantReader::Type type() const final override;

    bool isLengthKnown() const final override { return m_device->isLengthKnown(); }
    quint64 length() const final override { return m_device->length(); }

    bool enterContainer() final override { return m_device->enterContainer(); }
    bool leaveContainer() final override { return m_device->leaveContainer(); }

    QVariant readValue() final override ;

    QCborError lastError() const { return m_device->lastError(); }
    QCborParserError error() const;

    static QVariant fromCbor(const QByteArray& cbor, QCborParserError* error = nullptr);
    static QVariant fromCbor(QIODevice* device, QCborParserError* error = nullptr);

private:
    QCborStreamReader *m_device;

    const qint64 m_size;
};

#endif // QCBORVARIANTREADER_H
