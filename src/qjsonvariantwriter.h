#ifndef QJSONVARIANTWRITER_H
#define QJSONVARIANTWRITER_H

#include <QVariant>
#include <QByteArray>

class QIODevice;
class QJsonVariantWriter
{
public:
    explicit QJsonVariantWriter(QIODevice *device, bool compact=true);
    explicit QJsonVariantWriter(QByteArray *data, bool compact=true);
    ~QJsonVariantWriter();
    Q_DISABLE_COPY(QJsonVariantWriter)

    void start();
    void startArray();
    void endArray();
    void startMap();
    void endMap();

    void writeKeyValue(QStringView key, const QVariant& value);
    void writeNameSeparator();
    void writeValueSeparator();

    void writeString(QStringView s);
    void writeRaw(const char *data, qint64 len);
    void writeRaw(const char *data);
    void writeRaw(const QByteArray &ba);
    void writeVariant(const QVariant &v);

    static QByteArray fromVariant(const QVariant& variant, bool compact = true);
    static void fromVariant(const QVariant& variant, QIODevice* device, bool compact = true);

    static QByteArray escapedString(QStringView s);
    static QByteArray fromVariantDebug(const QVariant& variant, bool compact = true);

private:
    QIODevice *m_device;
    bool m_deleteDevice;

    bool m_compact;
    int m_indent;
};

#endif // QJSONVARIANTWRITER_H
