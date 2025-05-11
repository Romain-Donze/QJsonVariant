#ifndef QJSONWRITER_H
#define QJSONWRITER_H

#include <QVariant>
#include <QByteArray>
#include <QJsonParseError>

namespace QtJson
{

class Writer
{
public:
    static QByteArray variantToJson(const QVariant& variant, bool compact = true);
    static void variantToJsonStream(const QVariant& variant, QIODevice* device, bool compact = true);

    static QByteArray variantToJsonDebug(const QVariant& variant, bool compact = false);
};

}

#endif // QJSONWRITER_H
