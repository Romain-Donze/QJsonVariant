#ifndef QVARIANTREADER_H
#define QVARIANTREADER_H

#include <QVariant>
#include <QByteArray>
#include <QIODevice>

class QVariantReader
{
public:
    enum Type : quint8 {
        List = 0,
        Map = 1,
        Value = 2,
        Invalid = 3
    };

    QVariantReader() = default;
    virtual ~QVariantReader() = default;

    int currentProgress() const { return (currentOffset()/(double)totalSize()) * 10000.0; }
    virtual qint64 currentOffset() const = 0;
    virtual qint64 totalSize() const = 0;

    virtual bool hasError() = 0;
    virtual bool hasNext() const = 0;
    virtual bool next() = 0;
    virtual bool atEnd() = 0;

    virtual Type type() const = 0;
    bool isList() const     { return type() == List; }
    bool isMap() const      { return type() == Map; }
    bool isValue() const    { return type() == Value; }
    bool isInvalid() const  { return type() == Invalid; }
    bool isValid() const    { return !isInvalid(); }

    virtual bool isLengthKnown() const = 0;
    virtual quint64 length() const = 0;

    bool isContainer() const    { return isList() || isMap(); }
    virtual bool enterContainer() = 0;
    virtual bool leaveContainer() = 0;

    QVariant read();
    QVariantList readList();
    QVariantMap readMap();
    virtual QVariant readValue() = 0;

    virtual int errorCode() = 0;
    virtual QString errorString() = 0;
};

#endif // QVARIANTREADER_H
