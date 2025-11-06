#ifndef QJSONVARIANTREADER_H
#define QJSONVARIANTREADER_H

#include "qvariantreader.h"
#include <QJsonParseError>

class QJsonVariantReader: public QVariantReader
{
public:
    explicit QJsonVariantReader(QIODevice *device);
    explicit QJsonVariantReader(const QByteArray &data);
    virtual ~QJsonVariantReader();
    Q_DISABLE_COPY(QJsonVariantReader)

    qint64 currentOffset() const final override { return ptr - json; }
    qint64 totalSize() const final override { return m_buffer.size(); }

    bool hasError() final override { return lastError() != QJsonParseError::NoError; };
    bool hasNext() const final override;
    bool next() final override;
    bool atEnd() final override;

    Type type() const final override;

    bool isLengthKnown() const final override { return false; }
    quint64 length() const final override { return -1; }

    bool enterContainer() final override;
    bool leaveContainer() final override;

    QString readString() final override;
    QVariant readValue() final override;

    QJsonParseError::ParseError lastError() const { return m_lastError; }
    QJsonParseError error() const;
    int errorCode() final override { return error().error; }
    QString errorString() final override { return error().errorString(); }

    static QVariant fromJson(const QByteArray& json, QJsonParseError* error = nullptr);
    static QVariant fromJson(QIODevice* device, QJsonParseError* error = nullptr);

private:
    inline void skipByteOrderMark();
    inline bool skipWhitespace();
    inline QString parseString();
    inline QVariant parseNumber();

    QJsonParseError::ParseError m_lastError;

    QByteArray m_buffer;
    const char *json;
    const char *ptr;
    const char *end;
};

#endif // QJSONVARIANTREADER_H
