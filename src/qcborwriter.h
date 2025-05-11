#ifndef QCBORWRITER_H
#define QCBORWRITER_H

#include <QVariant>
#include <QByteArray>
#include <QJsonParseError>

namespace QtCbor
{

class Writer
{
public:
    static QByteArray variantToCbor(const QVariant& variant, int opt = 0);
    static void variantToCborStream(const QVariant& variant, QIODevice* device, int opt = 0);
};

}

#endif // QCBORWRITER_H
