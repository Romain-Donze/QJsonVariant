#include "qvariantreader.h"

QVariant QVariantReader::read()
{
    switch (type()) {
    case QVariantReader::List:
        return readList();
    case QVariantReader::Map:
        return readMap();
    default:
        return readValue();
        break;
    }
}
QVariantList QVariantReader::readList()
{
    QVariantList list;
    if (isLengthKnown())
        list.reserve(length());

    enterContainer();
    while (!hasError() && hasNext()) {
        list.append(read());
    }
    if (!hasError())
        leaveContainer();

    list.squeeze();

    return list;
}
QVariantMap QVariantReader::readMap()
{
    QVariantMap map;
    // if (isLengthKnown())
    //     map.reserve(length());

    enterContainer();
    while (!hasError() && hasNext()) {
        QString key = readString();
        map.insert(std::move(key), read());
    }
    if (!hasError())
        leaveContainer();

    return map;
}
