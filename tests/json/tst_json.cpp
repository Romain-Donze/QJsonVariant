#include <QtTest>

#include "qjsonvariantwriter.h"
#include "qjsonvariantreader.h"

#include "qcborvariantwriter.h"
#include "qcborvariantreader.h"
class TestJson : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void writing_data();
    void writing();

    void parsing_data();
    void parsing();

    void fileParser_data();
    void fileParser();

    void fileWriter_data();
    void fileWriter();

    void benchmark_data();
    void benchmark();

private:
    QVariant m_testVariant;
};

void TestJson::initTestCase()
{
    QVariantMap map;
    map.insert("null", QVariant::fromValue(nullptr));
    map.insert("boolTrue", true);
    map.insert("boolFalse", false);
    map.insert("char", (char)0);
    map.insert("int", 42);
    map.insert("zero", 0);
    map.insert("vide", "");
    map.insert("notValid", QVariant());
    map.insert("bytearray", QByteArray("test utf8"));
    map.insert("datetime", QDateTime::currentDateTime());
    map.insert("double", 3.14159);
    map.insert("string", "Hello world");
    map.insert("ztring", "Texte spécial: é, 中文, 😊, et \"guillemets\"");
    map.insert("emoji", "😊");
    map.insert("stringEsc", QString("Hello \"world\" \n \u263A"));
    map.insert("array", QVariantList{1, "two", false, QVariant(), 3.5});
    map.insert("object", QVariantMap{
                             {"nestedKey", "nestedValue"},
                             {"number", 123},
                             {"bool", true},
                             {"list", QVariantList{"a", "b", "c"}}
                         });
    map.insert("point", QVariantMap{
                            {"x", 0.016},
                            {"y", 0.55},
                            {"z", 0.349368}
                        });

    m_testVariant = map;
}

void TestJson::cleanupTestCase()
{

}

void TestJson::writing_data()
{
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<bool>("compact");

    QTest::newRow("writing indented") << m_testVariant << false;
    QTest::newRow("writing compact") << m_testVariant << true;
}

void TestJson::writing()
{
    QFETCH(QVariant, variant);
    QFETCH(bool, compact);

    QByteArray expected = QJsonDocument::fromVariant(variant).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    QByteArray result = QJsonVariantWriter::fromVariant(variant, compact);

    QCOMPARE(result, expected);
}

void TestJson::parsing_data()
{
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<bool>("compact");

    QTest::newRow("parsing indented") << m_testVariant << false;
    QTest::newRow("parsing compact") << m_testVariant << true;
}

void TestJson::parsing()
{
    QFETCH(QVariant, variant);
    QFETCH(bool, compact);

    QByteArray json = QJsonDocument::fromVariant(variant).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);

    QVariant expected = QJsonDocument::fromJson(json).toVariant();
    QVariant result = QJsonVariantReader::fromJson(json);

    QCOMPARE(result, expected);
}

void TestJson::fileParser_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow(":/test.json") << ":/test.json";
    QTest::newRow(":/test2.json") << ":/test2.json";
    QTest::newRow(":/test3.json") << ":/test3.json";
    QTest::newRow(":/test4.json") << ":/test4.json";
}

void TestJson::fileParser()
{
    QFETCH(QString, fileName);

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QByteArray json = file.readAll();

    QVariant expected = QJsonDocument::fromJson(json).toVariant();
    QVariant result = QJsonVariantReader::fromJson(json);

    QCOMPARE(result, expected);
}

void TestJson::fileWriter_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("compact");

    QTest::newRow(":/test.json indented") << ":/test.json" << false;
    QTest::newRow(":/test.json compact") << ":/test.json" << true;
    QTest::newRow(":/test2.json indented") << ":/test2.json" << false;
    QTest::newRow(":/test2.json compact") << ":/test2.json" << true;
    QTest::newRow(":/test3.json indented") << ":/test3.json" << false;
    QTest::newRow(":/test3.json compact") << ":/test3.json" << true;
    QTest::newRow(":/test4.json indented") << ":/test4.json" << false;
    QTest::newRow(":/test4.json compact") << ":/test4.json" << true;
}

void TestJson::fileWriter()
{
    QFETCH(QString, fileName);
    QFETCH(bool, compact);

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QByteArray json = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QVariant variant = doc.toVariant();

    QByteArray expected = doc.toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    QByteArray result = QJsonVariantWriter::fromVariant(variant, compact);

    QCOMPARE(result, expected);
}

void TestJson::benchmark_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("compact");

    QTest::newRow(":/benchmark.json indented") << ":/benchmark.json" << false;
    QTest::newRow(":/benchmark.json compact") << ":/benchmark.json" << true;
}

void TestJson::benchmark()
{
    QFETCH(QString, fileName);
    QFETCH(bool, compact);

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QVariant variant = doc.toVariant();
    QByteArray json = doc.toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    QCborValue value = QCborValue::fromVariant(variant);
    QByteArray cbor = value.toCbor(compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);

    QBENCHMARK {
        QJsonDocument::fromJson(json);
    }
    QBENCHMARK {
        doc.toVariant();
    }
    QBENCHMARK {
        doc.toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    }

    QBENCHMARK {
        QCborValue::fromCbor(cbor);
    }
    QBENCHMARK {
        value.toVariant();
    }
    QBENCHMARK {
        value.toCbor(compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }

    QBENCHMARK {
        QJsonDocument::fromJson(json).toVariant();
    }
    QBENCHMARK {
        QJsonVariantReader::fromJson(json);
    }
    QBENCHMARK {
        QCborValue::fromCbor(cbor).toVariant();
    }
    QBENCHMARK {
        QCborVariantReader::fromCbor(cbor);
    }

    QBENCHMARK {
        QJsonDocument::fromVariant(variant).toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented);
    }
    QBENCHMARK {
        QJsonVariantWriter::fromVariant(variant, compact);
    }
    QBENCHMARK {
        QCborValue::fromVariant(variant).toCbor(compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }
    QBENCHMARK {
        QCborVariantWriter::fromVariant(variant, compact ? QCborValue::UseFloat16 : QCborValue::NoTransformation);
    }
}

QTEST_APPLESS_MAIN(TestJson)

#include "tst_json.moc"
