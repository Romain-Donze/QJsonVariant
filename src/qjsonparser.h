#ifndef QJSONPARSER_H
#define QJSONPARSER_H

#include <QStack>
#include <QVariant>
#include <QByteArray>
#include <QJsonParseError>

namespace QtJson
{

class Parser
{
public:
    static QVariant jsonToVariant(const QByteArray& json, QJsonParseError* error = nullptr);
    static QVariant jsonStreamToVariant(QIODevice* device, QJsonParseError* error = nullptr);
};

}

#endif // QJSONPARSER_H
