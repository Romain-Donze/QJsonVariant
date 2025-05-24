#ifndef QCBORVARIANTWRITER_H
#define QCBORVARIANTWRITER_H

#include <QVariant>
#include <QByteArray>
#include <QCborStreamWriter>

class QCborVariantWriter
{
public:
    explicit QCborVariantWriter(QIODevice *device, int options=0);
    explicit QCborVariantWriter(QByteArray *data, int options=0);
    ~QCborVariantWriter();
    Q_DISABLE_COPY(QCborVariantWriter)

    void start();
    void startArray();
    void startArray(quint64 count);
    void endArray();
    void startMap();
    void startMap(quint64 count);
    void endMap();

    void writeKeyValue(const QString& key, const QVariant& value);

    void writeString(QStringView s);
    void writeRaw(const char *data, qint64 len);
    void writeRaw(const char *data);
    void writeRaw(const QByteArray &ba);
    void writeVariant(const QVariant &v);

    static QByteArray fromVariant(const QVariant& variant, int options = 0);
    static void fromVariant(const QVariant& variant, QIODevice* device, int options = 0);

private:
    QCborStreamWriter *m_device;

    int m_options;
};

#endif // QCBORVARIANTWRITER_H
