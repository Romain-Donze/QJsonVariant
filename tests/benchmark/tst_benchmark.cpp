#include <QtTest>

#include "qjsonvariantwriter.h"
#include "qjsonvariantreader.h"
#include "qcborvariantwriter.h"
#include "qcborvariantreader.h"
#include "qutf8.h"

#include "task_manager.h"

TaskManager generateTasks(size_t amount);

class TestBenchmark : public QObject
{
    Q_OBJECT

public:
    // using QObject::QObject;
    TestBenchmark()
        : m_testData(generateTasks(10'000))
    {}

private slots:
    void initTestCase();

    void benchmark_QJsonValue_String_toJson();
    void benchmark_QJsonValue_String_fromJson();
    void benchmark_QUtf8_String_toJson();
    void benchmark_QUtf8_String_fromJson();

    void benchmark_QJsonDocument_fromJson();
    void benchmark_QJsonDocument_toVariant();
    void benchmark_QJsonDocument_toJson();
    void benchmark_QCborValue_fromCbor();
    void benchmark_QCborValue_toVariant();
    void benchmark_QCborValue_toCbor();

    void benchmark_QJsonDocument_fromJson_toVariant();
    void benchmark_QJsonVariantReader_fromJson();
    void benchmark_QCborValue_fromCbor_toVariant();
    void benchmark_QCborVariantReader_fromCbor();

    void benchmark_QJsonDocument_fromVariant_toJson();
    void benchmark_QJsonVariantWriter_fromVariant();
    void benchmark_QCborValue_fromVariant_toCbor();
    void benchmark_QCborVariantWriter_fromVariant();

    void QtCoreSerialization_data() const;
    void QtCoreSerialization();

private:
    QJsonDocument m_doc;
    QVariant m_variant;
    QByteArray m_jsonIndented;
    QByteArray m_jsonCompact;
    QCborValue m_cborValue;
    QByteArray m_cborIndented;
    QByteArray m_cborCompact;

    bool m_compact;
    TaskManager m_testData;
};

void TestBenchmark::initTestCase()
{
    QFile file(":/benchmark.json");
    QVERIFY(file.open(QFile::ReadOnly));
    m_doc = QJsonDocument::fromJson(file.readAll());
    m_variant = m_doc.toVariant();

    m_jsonIndented = m_doc.toJson(QJsonDocument::Indented);
    m_jsonCompact = m_doc.toJson(QJsonDocument::Compact);

    m_cborValue = QCborValue::fromVariant(m_variant);
    m_cborIndented = m_cborValue.toCbor(QCborValue::NoTransformation);
    m_cborCompact = m_cborValue.toCbor(QCborValue::UseFloat16);

    m_compact = true;
}

#define SETUP_COMPACT \
    QByteArray& json = m_compact ? m_jsonCompact : m_jsonIndented; \
    QByteArray& cbor = m_compact ? m_cborCompact : m_cborIndented;

#define UNESCAPED_STRING QString("Hello World")
#define ESCAPED_STRING QByteArray("Hello World")

void TestBenchmark::benchmark_QJsonValue_String_toJson()
{
    QBENCHMARK {
        for(int i=0; i<1000000; i++)
        QJsonValue(UNESCAPED_STRING).toJson();
    }
}

void TestBenchmark::benchmark_QJsonValue_String_fromJson()
{
    QBENCHMARK {
        for(int i=0; i<1000000; i++)
        QJsonValue::fromJson(ESCAPED_STRING).toString();
    }
}

void TestBenchmark::benchmark_QUtf8_String_toJson()
{
    QBENCHMARK {
        for(int i=0; i<1000000; i++)
        QUtf8::escapedString(UNESCAPED_STRING);
    }
}

void TestBenchmark::benchmark_QUtf8_String_fromJson()
{
    QBENCHMARK {
        for(int i=0; i<1000000; i++)
        QUtf8::unescapedString(ESCAPED_STRING);
    }
}

void TestBenchmark::benchmark_QJsonDocument_fromJson()
{
    SETUP_COMPACT
    QBENCHMARK {
        QJsonDocument::fromJson(json);
    }
}

void TestBenchmark::benchmark_QJsonDocument_toVariant()
{
    QBENCHMARK {
        m_doc.toVariant();
    }
}

void TestBenchmark::benchmark_QJsonDocument_toJson()
{
    SETUP_COMPACT
    QBENCHMARK {
        m_doc.toJson(m_compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    }
}

void TestBenchmark::benchmark_QCborValue_fromCbor()
{
    SETUP_COMPACT
    QBENCHMARK {
        QCborValue::fromCbor(cbor);
    }
}

void TestBenchmark::benchmark_QCborValue_toVariant()
{
    QBENCHMARK {
        m_cborValue.toVariant();
    }
}

void TestBenchmark::benchmark_QCborValue_toCbor()
{
    SETUP_COMPACT
    QBENCHMARK {
        m_cborValue.toCbor(m_compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }
}

void TestBenchmark::benchmark_QJsonDocument_fromJson_toVariant()
{
    SETUP_COMPACT
    QBENCHMARK {
        QJsonDocument::fromJson(json).toVariant();
    }
}

void TestBenchmark::benchmark_QJsonVariantReader_fromJson()
{
    SETUP_COMPACT
    QBENCHMARK {
        QJsonVariantReader::fromJson(json);
    }
}

void TestBenchmark::benchmark_QCborValue_fromCbor_toVariant()
{
    SETUP_COMPACT
    QBENCHMARK {
        QCborValue::fromCbor(cbor).toVariant();
    }
}

void TestBenchmark::benchmark_QCborVariantReader_fromCbor()
{
    SETUP_COMPACT
    QBENCHMARK {
        QCborVariantReader::fromCbor(cbor);
    }
}

void TestBenchmark::benchmark_QJsonDocument_fromVariant_toJson()
{
    SETUP_COMPACT
    QBENCHMARK {
        QJsonDocument::fromVariant(m_variant).toJson(m_compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    }
}

void TestBenchmark::benchmark_QJsonVariantWriter_fromVariant()
{
    QBENCHMARK {
        QJsonVariantWriter::fromVariant(m_variant, m_compact);
    }
}

void TestBenchmark::benchmark_QCborValue_fromVariant_toCbor()
{
    SETUP_COMPACT
    QBENCHMARK {
        QCborValue::fromVariant(m_variant).toCbor(m_compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }
}

void TestBenchmark::benchmark_QCborVariantWriter_fromVariant()
{
    SETUP_COMPACT
    QBENCHMARK {
        QCborVariantWriter::fromVariant(m_variant, m_compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }
}

// ## QtCoreSerialization
using namespace Qt::StringLiterals;
using namespace Qt::StringLiterals;

QByteArray serializeDataStream(const TaskManager &manager);
TaskManager deserializeDataStream(const QByteArray &data);

QByteArray serializeXml(const TaskManager &contacts);
TaskManager deserializeXml(const QByteArray &data);

QByteArray serializeJson(const TaskManager &contacts);
TaskManager deserializeJson(const QByteArray &data);

QByteArray serializeCbor(const TaskManager &contacts);
TaskManager deserializeCbor(const QByteArray &data);

QByteArray serializeCborStream(const TaskManager &contacts);
TaskManager deserializeCborStream(const QByteArray &data);

QByteArray serializeJsonVariantStream(const TaskManager &contacts);
TaskManager deserializeJsonVariantStream(const QByteArray &data);

QByteArray serializeCborVariantStream(const TaskManager &contacts);
TaskManager deserializeCborVariantStream(const QByteArray &data);

struct SerializationFormat
{
    QByteArray (*serialize)(const TaskManager &);
    TaskManager (*deserialize)(const QByteArray &);
};

void TestBenchmark::QtCoreSerialization_data() const
{
    QTest::addColumn<SerializationFormat>("format");

    QTest::newRow("QDataStream") << SerializationFormat {
                                                        serializeDataStream, deserializeDataStream };
    QTest::newRow("XML") << SerializationFormat{
                                                serializeXml, deserializeXml };
    QTest::newRow("JSON") << SerializationFormat{
                                                 serializeJson, deserializeJson };
    QTest::newRow("JSONVariantStream") << SerializationFormat{
                                                        serializeJsonVariantStream, deserializeJsonVariantStream };
    QTest::newRow("CBOR") << SerializationFormat{
                                                 serializeCbor, deserializeCbor };
    QTest::newRow("CBORStream") << SerializationFormat{
                                                       serializeCborStream, deserializeCborStream };
    QTest::newRow("CBORVariantStream") << SerializationFormat{
                                                        serializeCborVariantStream, deserializeCborVariantStream };
}
void TestBenchmark::QtCoreSerialization()
{
    QFETCH(SerializationFormat, format);

    QByteArray encodedData;
    TaskManager taskManager;

    QBENCHMARK {
        encodedData = format.serialize(m_testData);
    }
    QBENCHMARK {
        taskManager = format.deserialize(encodedData);
    }
    QTest::setBenchmarkResult(encodedData.size(), QTest::BytesAllocated);

    QCOMPARE_EQ(taskManager, m_testData);
}
#include <QtCore/QDataStream>

QDataStream &operator<<(QDataStream &stream, const TaskHeader &header) {
    return stream << header.id << header.name << header.created << header.color;
}
QDataStream &operator<<(QDataStream &stream, const Task &task) {
    return stream << task.header << task.description << qint8(task.priority) << task.completed;
}
QDataStream &operator<<(QDataStream &stream, const TaskList &list) {
    return stream << list.header << list.tasks;
}
QDataStream &operator<<(QDataStream &stream, const TaskManager &manager) {
    return stream << manager.user << manager.version << manager.lists;
}

QByteArray serializeDataStream(const TaskManager &manager)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << manager;
    return data;
}

QDataStream &operator>>(QDataStream &stream, TaskHeader &header) {
    return stream >> header.id >> header.name >> header.created >> header.color;
}
QDataStream &operator>>(QDataStream &stream, Task &task) {
    qint8 priority;
    stream >> task.header >> task.description >> priority >> task.completed;
    task.priority = Task::Priority(priority);
    return stream;
}
QDataStream &operator>>(QDataStream &stream, TaskList &list) {
    return stream >> list.header >> list.tasks;
}
QDataStream &operator>>(QDataStream &stream, TaskManager &manager) {
    return stream >> manager.user >> manager.version >> manager.lists;
}

TaskManager deserializeDataStream(const QByteArray &data)
{
    TaskManager manager;
    QDataStream stream(data);
    stream >> manager;
    return manager;
}

#include <QtCore/QXmlStreamWriter>

void encodeXmlHeader(QXmlStreamWriter &writer, const TaskHeader &header) {
    writer.writeAttribute("id"_L1, header.id.toString(QUuid::WithoutBraces));
    writer.writeAttribute("name"_L1, header.name);
    writer.writeAttribute("color"_L1, header.color.name());
    writer.writeAttribute("created"_L1, header.created.toString(Qt::ISODateWithMs));
}
void encodeXmlTask(QXmlStreamWriter &writer, const Task &task) {
    writer.writeStartElement("task"_L1);
    encodeXmlHeader(writer, task.header);
    writer.writeAttribute("priority"_L1, QString::number(qToUnderlying(task.priority)));
    writer.writeAttribute("completed"_L1, task.completed ? "true"_L1 : "false"_L1);
    writer.writeCharacters(task.description);
    writer.writeEndElement();
}
void encodeXmlTaskList(QXmlStreamWriter &writer, const TaskList &list) {
    writer.writeStartElement("tasklist"_L1);
    encodeXmlHeader(writer, list.header);
    for (const auto &task : list.tasks)
        encodeXmlTask(writer, task);
    writer.writeEndElement();
}
void encodeXmlTaskManager(QXmlStreamWriter &writer, const TaskManager &manager) {
    writer.writeStartElement("taskmanager"_L1);
    writer.writeAttribute("user"_L1, manager.user);
    writer.writeAttribute("version"_L1, manager.version.toString());
    for (const auto &list : manager.lists)
        encodeXmlTaskList(writer, list);
    writer.writeEndElement();
}

QByteArray serializeXml(const TaskManager &manager)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);

    writer.writeStartDocument();
    encodeXmlTaskManager(writer, manager);
    writer.writeEndDocument();

    return data;
}

#include <QtCore/QXmlStreamReader>

TaskHeader decodeXmlHeader(const QXmlStreamAttributes &attrs) {
    return TaskHeader {
        .id = QUuid(attrs.value("id"_L1).toString()),
        .name = attrs.value("name"_L1).toString(),
        .color = QColor(attrs.value("color"_L1).toString()),
        .created = QDateTime::fromString(attrs.value("created"_L1).toString(), Qt::ISODateWithMs)
    };
}
Task decodeXmlTask(QXmlStreamReader &reader) {
    const auto attrs = reader.attributes();
    return Task {
        .header = decodeXmlHeader(attrs),
        .priority = Task::Priority(attrs.value("priority"_L1).toInt()),
        .completed = attrs.value("completed"_L1) == "true"_L1,
        .description = reader.readElementText(),
    };
}
TaskList decodeXmlTaskList(QXmlStreamReader &reader) {
    const auto attrs = reader.attributes();
    return TaskList {
        .header = decodeXmlHeader(attrs),
        .tasks = [](auto &reader) {
            QList<Task> tasks;
            while (reader.readNextStartElement() && reader.name() == "task"_L1)
                tasks.append(decodeXmlTask(reader));
            return tasks;
        }(reader)
    };
}
TaskManager decodeXmlTaskManager(QXmlStreamReader &reader) {
    const auto attrs = reader.attributes();
    return TaskManager {
        .user = attrs.value("user"_L1).toString(),
        .version = QVersionNumber::fromString(attrs.value("version"_L1).toString()),
        .lists = [](auto &reader) {
            QList<TaskList> taskLists;
            while (reader.readNextStartElement() && reader.name() == "tasklist"_L1)
                taskLists.append(decodeXmlTaskList(reader));
            return taskLists;
        }(reader)
    };
}

TaskManager deserializeXml(const QByteArray &data)
{
    QXmlStreamReader reader(data);
    while (reader.readNextStartElement() && reader.name() == "taskmanager"_L1)
        return decodeXmlTaskManager(reader);
    return {};
}

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

QJsonValue encodeJsonHeader(const TaskHeader &header) {
    return QJsonObject{
        { "id"_L1, header.id.toString(QUuid::WithoutBraces) },
        { "name"_L1, header.name },
        { "color"_L1, header.color.name() },
        { "created"_L1, header.created.toString(Qt::ISODateWithMs) }
    };
}
QJsonValue encodeJsonTask(const Task &task) {
    return QJsonObject{
        { "header"_L1, encodeJsonHeader(task.header) },
        { "description"_L1, task.description },
        { "priority"_L1, qToUnderlying(task.priority) },
        { "completed"_L1, task.completed }
    };
}
QJsonValue encodeJsonTaskList(const TaskList &list) {
    return QJsonObject{
        { "header"_L1, encodeJsonHeader(list.header) },
        { "tasks"_L1, [](const auto &tasks) {
             QJsonArray taskArray;
             for (const auto &t : tasks)
                 taskArray.append(encodeJsonTask(t));
             return taskArray;
         }(list.tasks) }
    };
}
QJsonObject encodeJsonTaskManager(const TaskManager &manager) {
    return QJsonObject{
        { "user"_L1, manager.user },
        { "version"_L1, manager.version.toString() },
        { "lists"_L1, [](const auto &lists) {
             QJsonArray taskListArray;
             for (const auto &l : lists)
                 taskListArray.append(encodeJsonTaskList(l));
             return taskListArray;
         }(manager.lists) }
    };
}

QByteArray serializeJson(const TaskManager &manager)
{
    return QJsonDocument(encodeJsonTaskManager(manager)).toJson(QJsonDocument::Compact);
}

TaskHeader decodeJsonHeader(const QJsonObject &obj) {
    return {
        .id = QUuid(obj["id"_L1].toString()),
        .name = obj["name"_L1].toString(),
        .color = QColor(obj["color"_L1].toString()),
        .created = QDateTime::fromString(obj["created"_L1].toString(), Qt::ISODateWithMs)
    };
}
Task decodeJsonTask(const QJsonObject &obj) {
    return {
        .header = decodeJsonHeader(obj["header"_L1].toObject()),
        .priority = Task::Priority(obj["priority"_L1].toInt()),
        .completed = obj["completed"_L1].toBool(),
        .description = obj["description"_L1].toString()
    };
}
TaskList decodeJsonTaskList(const QJsonObject &obj) {
    return {
        .header = decodeJsonHeader(obj["header"_L1].toObject()),
        .tasks = [](const QJsonArray &array) {
            QList<Task> tasks; tasks.reserve(array.size());
            for (const auto &taskValue : array)
                tasks.append(decodeJsonTask(taskValue.toObject()));
            return tasks;
        }(obj["tasks"_L1].toArray())
    };
}
TaskManager decodeJsonTaskManager(const QJsonObject &obj) {
    return {
        .user = obj["user"_L1].toString(),
        .version = QVersionNumber::fromString(obj["version"_L1].toString()),
        .lists = [](const QJsonArray &array) {
            QList<TaskList> lists; lists.reserve(array.size());
            for (const auto &listValue : array)
                lists.append(decodeJsonTaskList(listValue.toObject()));
            return lists;
        }(obj["lists"_L1].toArray())
    };
}

TaskManager deserializeJson(const QByteArray &data)
{
    const auto jsonRoot = QJsonDocument::fromJson(data).object();
    return decodeJsonTaskManager(jsonRoot);
}

#include <QtCore/QCborArray>
#include <QtCore/QCborMap>
#include <QtCore/QCborValue>

QCborMap encodeCborHeader(const TaskHeader &header) {
    return {
        { "id"_L1, QCborValue(header.id) },
        { "name"_L1, header.name },
        { "color"_L1, header.color.name() },
        { "created"_L1, QCborValue(header.created) }
    };
}
QCborMap encodeCborTask(const Task &task) {
    return {
        {"header"_L1, encodeCborHeader(task.header)},
        {"description"_L1, task.description},
        {"priority"_L1, qToUnderlying(task.priority)},
        {"completed"_L1, task.completed}
    };
}
QCborMap encodeCborTaskList(const TaskList &list) {
    return {
        { "header"_L1, encodeCborHeader(list.header) },
        { "tasks"_L1, [](const auto &tasks) {
             QCborArray tasksArray;
             for (const auto &t : tasks)
                 tasksArray.append(encodeCborTask(t));
             return tasksArray;
         }(list.tasks) }
    };
}
QCborMap encodeCborTaskManager(const TaskManager &manager) {
    return {
        { "user"_L1, manager.user },
        { "version"_L1, manager.version.toString() },
        { "lists"_L1, [](const auto &lists) {
             QCborArray listsArray;
             for (const auto &l : lists)
                 listsArray.append(encodeCborTaskList(l));
             return listsArray;
         }(manager.lists) }
    };
}

QByteArray serializeCbor(const TaskManager &manager)
{
    return QCborValue(encodeCborTaskManager(manager)).toCbor();
}

TaskHeader decodeCborHeader(const QCborMap &map) {
    return TaskHeader{
        .id = map["id"_L1].toUuid(),
        .name = map["name"_L1].toString(),
        .color = QColor(map["color"_L1].toString()),
        .created = map["created"_L1].toDateTime(),
    };
}
Task decodeCborTask(const QCborMap &map) {
    return Task{
        .header = decodeCborHeader(map["header"_L1].toMap()),
        .priority = Task::Priority(map["priority"_L1].toInteger()),
        .completed = map["completed"_L1].toBool(),
        .description = map["description"_L1].toString()
    };
}
TaskList decodeCborTaskList(const QCborMap &map) {
    return TaskList {
        .header = decodeCborHeader(map["header"_L1].toMap()),
        .tasks = [](const QCborArray &array) {
            QList<Task> tasks; tasks.reserve(array.size());
            for (const auto &taskValue : array)
                tasks.append(decodeCborTask(taskValue.toMap()));
            return tasks;
        }(map["tasks"_L1].toArray())
    };
}
TaskManager decodeCborTaskManager(const QCborMap &map) {
    return TaskManager {
        .user = map["user"_L1].toString(),
        .version = QVersionNumber::fromString(map["version"_L1].toString()),
        .lists = [](const QCborArray &array) {
            QList<TaskList> lists; lists.reserve(array.size());
            for (const auto &listValue : array)
                lists.append(decodeCborTaskList(listValue.toMap()));
            return lists;
        }(map["lists"_L1].toArray())
    };
}

TaskManager deserializeCbor(const QByteArray &data) {
    const auto cborRoot = QCborValue::fromCbor(data).toMap();
    return decodeCborTaskManager(cborRoot);
}
#include <QtCore/QCborStreamWriter>

void encodeCborStreamHeader(QCborStreamWriter &writer, const TaskHeader &header) {
    writer.startMap(4);

    writer.append("id"_L1);
    writer.append(header.id.toString(QUuid::WithoutBraces));

    writer.append("name"_L1);
    writer.append(header.name);

    writer.append("color"_L1);
    writer.append(header.color.name());

    writer.append("created"_L1);
    writer.append(header.created.toString(Qt::ISODateWithMs));

    writer.endMap();
}
void encodeCborStreamTask(QCborStreamWriter &writer, const Task &task) {
    writer.startMap(4);
    writer.append("header"_L1);
    encodeCborStreamHeader(writer, task.header);
    writer.append("description"_L1);
    writer.append(task.description);
    writer.append("priority"_L1);
    writer.append(qint64(qToUnderlying(task.priority)));
    writer.append("completed"_L1);
    writer.append(task.completed);
    writer.endMap();
}
void encodeCborStreamTaskList(QCborStreamWriter &writer, const TaskList &list) {
    writer.startMap(2);

    writer.append("header"_L1);
    encodeCborStreamHeader(writer, list.header);

    writer.append("tasks"_L1);
    writer.startArray(quint64(list.tasks.size()));
    for (const auto &t : list.tasks)
        encodeCborStreamTask(writer, t);
    writer.endArray();

    writer.endMap();
}
void encodeCborStreamTaskManager(QCborStreamWriter &writer, const TaskManager &manager) {
    writer.startMap(3);
    writer.append("user"_L1);
    writer.append(manager.user);
    writer.append("version"_L1);
    writer.append(manager.version.toString());
    writer.append("lists"_L1);
    writer.startArray(quint64(manager.lists.size()));
    for (const auto &l : manager.lists)
        encodeCborStreamTaskList(writer, l);
    writer.endArray();

    writer.endMap();
}

QByteArray serializeCborStream(const TaskManager &manager)
{
    QByteArray data;
    QCborStreamWriter writer(&data);
    encodeCborStreamTaskManager(writer, manager);
    return data;
}

#include <QtCore/QCborStreamReader>

TaskHeader decodeCborStreamHeader(QCborStreamReader &reader) {
    TaskHeader header;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString().data;
        reader.next();
        if (key == "id"_L1)
            header.id = QUuid(reader.readString().data);
        else if (key == "name"_L1)
            header.name = reader.readString().data;
        else if (key == "color"_L1)
            header.color = QColor(reader.readString().data);
        else if (key == "created"_L1)
            header.created = QDateTime::fromString(reader.readString().data, Qt::ISODateWithMs);
        reader.next();
    }
    reader.leaveContainer();
    return header;
}
Task decodeCborStreamTask(QCborStreamReader &reader) {
    Task task;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString().data;
        reader.next();
        if (key == "header"_L1) {
            task.header = decodeCborStreamHeader(reader);
        } else if (key == "description"_L1) {
            task.description = reader.readString().data;
            reader.next();
        } else if (key == "priority"_L1) {
            task.priority = Task::Priority(reader.toInteger());
            reader.next();
        } else if (key == "completed"_L1) {
            task.completed = reader.toBool();
            reader.next();
        } else {
            reader.next();
        }
    }
    reader.leaveContainer();
    return task;
}
TaskList decodeCborStreamTaskList(QCborStreamReader &reader) {
    TaskList list;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString().data;
        reader.next();
        if (key == "header"_L1) {
            list.header = decodeCborStreamHeader(reader);
        } else if (key == "tasks"_L1) {
            reader.enterContainer();
            while (reader.hasNext())
                list.tasks.append(decodeCborStreamTask(reader));
            reader.leaveContainer();
        } else {
            reader.next();
        }
    }
    reader.leaveContainer();
    return list;
}
TaskManager decodeCborStreamTaskManager(QCborStreamReader &reader) {
    TaskManager manager;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString().data;
        reader.next();
        if (key == "user"_L1) {
            manager.user = reader.readString().data;
            reader.next();
        } else if (key == "version"_L1) {
            manager.version = QVersionNumber::fromString(reader.readString().data);
            reader.next();
        } else if (key == "lists"_L1) {
            reader.enterContainer();
            while (reader.hasNext())
                manager.lists.append(decodeCborStreamTaskList(reader));
            reader.leaveContainer();
        } else {
            reader.next();
        }
    }
    reader.leaveContainer();
    return manager;
}

TaskManager deserializeCborStream(const QByteArray &data) {
    QCborStreamReader reader(data);
    return decodeCborStreamTaskManager(reader);
}

#include "qjsonvariantwriter.h"
#include "qjsonvariantreader.h"

void encodeJsonVariantStreamHeader(const TaskHeader &header, QJsonVariantWriter& writer) {
    writer.startMap();
    writer.writeKeyValue("id", header.id.toString(QUuid::WithoutBraces));
    writer.writeValueSeparator();
    writer.writeKeyValue("name", header.name);
    writer.writeValueSeparator();
    writer.writeKeyValue("color", header.color.name());
    writer.writeValueSeparator();
    writer.writeKeyValue("created", header.created.toString(Qt::ISODateWithMs));
    writer.endMap();
}
void encodeJsonVariantStreamTask(const Task &task, QJsonVariantWriter& writer) {
    writer.startMap();
    writer.writeString("header");
    writer.writeNameSeparator();
    encodeJsonVariantStreamHeader(task.header, writer);
    writer.writeValueSeparator();
    writer.writeKeyValue("description", task.description);
    writer.writeValueSeparator();
    writer.writeKeyValue("priority", qToUnderlying(task.priority));
    writer.writeValueSeparator();
    writer.writeKeyValue("completed", task.completed);
    writer.endMap();
}
void encodeJsonVariantStreamTaskList(const TaskList &list, QJsonVariantWriter& writer) {
    writer.startMap();
    writer.writeString("header");
    writer.writeNameSeparator();
    encodeJsonVariantStreamHeader(list.header, writer);
    writer.writeValueSeparator();
    writer.writeString("tasks");
    writer.writeNameSeparator();
    writer.startArray();
    qsizetype i = 0;
    for(const auto& t: list.tasks) {
        encodeJsonVariantStreamTask(t, writer);
        if (++i == list.tasks.size()) {
            break;
        }
        writer.writeValueSeparator();
    }
    writer.endArray();
    writer.endMap();
}
void encodeJsonVariantStreamTaskManager(const TaskManager &manager, QJsonVariantWriter& writer) {
    writer.startMap();
    writer.writeKeyValue("user", manager.user);
    writer.writeValueSeparator();
    writer.writeKeyValue("version", manager.version.toString());
    writer.writeValueSeparator();
    writer.writeString("lists");
    writer.writeNameSeparator();
    writer.startArray();
    qsizetype i = 0;
    for(const auto& t: manager.lists) {
        encodeJsonVariantStreamTaskList(t, writer);
        if (++i == manager.lists.size()) {
            break;
        }
        writer.writeValueSeparator();
    }
    writer.endArray();
    writer.endMap();
}

QByteArray serializeJsonVariantStream(const TaskManager &manager)
{
    QByteArray json;
    QJsonVariantWriter writer(&json, true);

    writer.start();
    encodeJsonVariantStreamTaskManager(manager, writer);

    return json;
}

TaskHeader decodeVariantStreamHeader(QVariantReader &reader) {
    TaskHeader header;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString();
        if(key=="id"_L1)
            header.id = QUuid(reader.readString());
        else if(key=="name"_L1)
            header.name = reader.readString();
        else if(key=="color"_L1)
            header.color = QColor(reader.readString());
        else if(key=="created"_L1)
            header.created = QDateTime::fromString(reader.readString(), Qt::ISODateWithMs);
    }
    reader.leaveContainer();
    return header;
}
Task decodeVariantStreamTask(QVariantReader &reader) {
    Task task;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString();
        if(key=="header"_L1)
            task.header = decodeVariantStreamHeader(reader);
        else if(key=="priority"_L1)
            task.priority = Task::Priority(reader.read().toInt());
        else if(key=="completed"_L1)
            task.completed = reader.read().toBool();
        else if(key=="description"_L1)
            task.description = reader.readString();
    }
    reader.leaveContainer();
    return task;
}
TaskList decodeVariantStreamTaskList(QVariantReader &reader) {
    TaskList list;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString();
        if(key=="header"_L1)
            list.header = decodeVariantStreamHeader(reader);
        else if(key=="tasks"_L1) {
            if (reader.isLengthKnown())
                list.tasks.reserve(reader.length());
            reader.enterContainer();
            while (reader.hasNext()) {
                list.tasks.append(decodeVariantStreamTask(reader));
            }
            reader.leaveContainer();
        }
    }
    reader.leaveContainer();
    return list;
}
TaskManager decodeVariantStreamTaskManager(QVariantReader &reader) {
    TaskManager manager;
    reader.enterContainer();
    while (reader.hasNext()) {
        const QString key = reader.readString();
        if(key=="user"_L1)
            manager.user = reader.readString();
        else if(key=="version"_L1)
            manager.version = QVersionNumber::fromString(reader.readString());
        else if(key=="lists"_L1) {
            if (reader.isLengthKnown())
                manager.lists.reserve(reader.length());
            reader.enterContainer();
            while (reader.hasNext()) {
                manager.lists.append(decodeVariantStreamTaskList(reader));
            }
            reader.leaveContainer();
        }
    }
    reader.leaveContainer();
    return manager;
}

TaskManager deserializeJsonVariantStream(const QByteArray &data)
{
    QJsonVariantReader reader(data);
    return decodeVariantStreamTaskManager(reader);
}

#include "qcborvariantwriter.h"
#include "qcborvariantreader.h"

void encodeCborVariantStreamHeader(const TaskHeader &header, QCborVariantWriter& writer) {
    writer.startMap(4);
    writer.writeKeyValue("id", header.id.toString(QUuid::WithoutBraces));
    writer.writeKeyValue("name", header.name);
    writer.writeKeyValue("color", header.color.name());
    writer.writeKeyValue("created", header.created.toString(Qt::ISODateWithMs));
    writer.endMap();
}
void encodeCborVariantStreamTask(const Task &task, QCborVariantWriter& writer) {
    writer.startMap(4);
    writer.writeString("header");
    encodeCborVariantStreamHeader(task.header, writer);
    writer.writeKeyValue("description", task.description);
    writer.writeKeyValue("priority", qToUnderlying(task.priority));
    writer.writeKeyValue("completed", task.completed);
    writer.endMap();
}
void encodeCborVariantStreamTaskList(const TaskList &list, QCborVariantWriter& writer) {
    writer.startMap(2);
    writer.writeString("header");
    encodeCborVariantStreamHeader(list.header, writer);
    writer.writeString("tasks");
    writer.startArray(list.tasks.size());
    qsizetype i = 0;
    for(const auto& t: list.tasks) {
        encodeCborVariantStreamTask(t, writer);
    }
    writer.endArray();
    writer.endMap();
}
void encodeCborVariantStreamTaskManager(const TaskManager &manager, QCborVariantWriter& writer) {
    writer.startMap(3);
    writer.writeKeyValue("user", manager.user);
    writer.writeKeyValue("version", manager.version.toString());
    writer.writeString("lists");
    writer.startArray(manager.lists.size());
    qsizetype i = 0;
    for(const auto& t: manager.lists) {
        encodeCborVariantStreamTaskList(t, writer);
    }
    writer.endArray();
    writer.endMap();
}

QByteArray serializeCborVariantStream(const TaskManager &manager)
{
    QByteArray cbor;
    QCborVariantWriter writer(&cbor);

    writer.start();
    encodeCborVariantStreamTaskManager(manager, writer);

    return cbor;

    return QCborValue(encodeCborTaskManager(manager)).toCbor();
}

TaskManager deserializeCborVariantStream(const QByteArray &data) {
    QCborVariantReader reader(data);
    return decodeVariantStreamTaskManager(reader);
}

// ## Helpers
TaskManager generateTasks(size_t count)
{
    TaskManager manager;
    manager.user = "Demo User"_L1;
    manager.version = QVersionNumber{ 1, 0, 0 };

    QStringList listNames = { "Work"_L1, "Personal"_L1, "Shopping"_L1, "Home"_L1, "Learning"_L1, "Health"_L1, "Finance"_L1, "Travel"_L1 };
    QList<QColor> colors = { Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::cyan, Qt::magenta, Qt::gray, Qt::darkGreen };
    QStringList descriptions = {
        "Finish quarterly report"_L1, "Buy groceries for the week"_L1, "Call important client"_L1,
        "Study new documentation"_L1, "Fix critical production bugs"_L1, "Schedule team meeting"_L1,
        "Review project budget"_L1, "Plan vacation itinerary"_L1, "Update portfolio"_L1,
        "Research new technologies"_L1, "Write unit tests"_L1, "Deploy to staging"_L1
    };

    int targetListCount = 10 + (count % 3); // 10-12 lists
    int baseTasksPerList = count / targetListCount;

    size_t tasksLeft = count;
    int listIndex = 0;

    while (tasksLeft > 0 && listIndex < targetListCount) {
        TaskList list;
        list.header = {
            QUuid::createUuid(),
            listNames[listIndex % listNames.size()],
            colors[listIndex % colors.size()],
            QDateTime::currentDateTimeUtc().addDays(-listIndex * 7).addSecs(listIndex * 3600),
        };

        int listSize = baseTasksPerList + ((listIndex * 17) % (baseTasksPerList / 5)) - (baseTasksPerList / 10);
        listSize = std::min(static_cast<size_t>(listSize), tasksLeft);
        listSize = std::max(1, listSize); // Ensure at least 1 task

        for (int i = 0; i < listSize; ++i) {
            Task task;
            task.header = {
                QUuid::createUuid(),
                QString("Task %1-%2"_L1).arg(listIndex).arg(i),
                colors[(listIndex + i) % colors.size()],
                QDateTime::currentDateTimeUtc().addDays(-listIndex * 7).addSecs(i * 120),
            };
            task.description = descriptions[(listIndex * 13 + i * 7) % descriptions.size()];
            task.priority = static_cast<Task::Priority>((listIndex + i) % 4);
            task.completed = (i % 7) == 0;
            list.tasks.append(task);
        }

        manager.lists.append(list);
        tasksLeft -= listSize;
        listIndex++;
    }

    return manager;
}

QTEST_APPLESS_MAIN(TestBenchmark)

#include "tst_benchmark.moc"
