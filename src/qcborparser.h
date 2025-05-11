#ifndef QCBORPARSER_H
#define QCBORPARSER_H

#include <QStack>
#include <QVariant>
#include <QByteArray>
#include <QCborParserError>

namespace QtCbor
{

class Parser
{
public:
    static QVariant cborToVariant(const QByteArray& cbor, QCborParserError* error = nullptr);
    static QVariant cborStreamToVariant(QIODevice* device, QCborParserError* error = nullptr);
};

}

#endif // QCBORPARSER_H
