#include "suite.h"

#include "vendors/qjsonwriter.h"
#include "vendors/qjsonparser.h"
#include "vendors/qcborwriter.h"
#include "vendors/qcborparser.h"

class TestJsonCbor: public TestSuite
{
    Q_OBJECT

public:
    using TestSuite::TestSuite;

private slots:
    void initTestCase();

    void json_writingIndented();
    void json_writingCompact();
    void json_parsingIndented();
    void json_parsingCompact();

    void cbor_writingStandard();
    void cbor_writingFloat16();
    void cbor_parsingStandard();
    void cbor_parsingFloat16();

private:
    QVariant m_testVariant;
};

void TestJsonCbor::initTestCase()
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
    map.insert("datetime", QDateTime::currentDateTime());
    map.insert("double", 3.14159);
    map.insert("string", "Hello world");
    map.insert("ztring", "Texte spécial: é, 中文, 😊, et \"guillemets\"");
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

void TestJsonCbor::json_writingIndented()
{
    QBENCHMARK {
        QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Indented);
    }
    QBENCHMARK {
        QtJson::Writer::variantToJson(m_testVariant, false);
    }

    QByteArray expected = QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Indented);
    QByteArray json = QtJson::Writer::variantToJson(m_testVariant, false);

    QCOMPARE(json, expected);
}

void TestJsonCbor::json_writingCompact()
{
    QBENCHMARK {
        QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Compact);
    }
    QBENCHMARK {
        QtJson::Writer::variantToJson(m_testVariant, true);
    }

    QByteArray expected = QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Compact);
    QByteArray json = QtJson::Writer::variantToJson(m_testVariant, true);

    QCOMPARE(json, expected);
}

void TestJsonCbor::json_parsingIndented()
{
    QByteArray json = QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Indented);

    QBENCHMARK {
        QJsonDocument::fromJson(json).toVariant();
    }
    QBENCHMARK {
        QtJson::Parser::jsonToVariant(json);
    }

    QVariant expected = QJsonDocument::fromJson(json).toVariant();
    QVariant variant = QtJson::Parser::jsonToVariant(json);

    QCOMPARE(variant, expected);
}

void TestJsonCbor::json_parsingCompact()
{
    QByteArray json = QJsonDocument::fromVariant(m_testVariant).toJson(QJsonDocument::Compact);

    QBENCHMARK {
        QJsonDocument::fromJson(json).toVariant();
    }
    QBENCHMARK {
        QtJson::Parser::jsonToVariant(json);
    }

    QVariant expected = QJsonDocument::fromJson(json).toVariant();
    QVariant variant = QtJson::Parser::jsonToVariant(json);

    QCOMPARE(variant, expected);
}

void TestJsonCbor::cbor_writingStandard()
{
    QBENCHMARK {
        QCborValue::fromVariant(m_testVariant).toCbor();
    }
    QBENCHMARK {
        QtCbor::Writer::variantToCbor(m_testVariant);
    }

    QByteArray expected = QCborValue::fromVariant(m_testVariant).toCbor();
    QByteArray cbor = QtCbor::Writer::variantToCbor(m_testVariant);

    QCOMPARE(cbor, expected);
}

void TestJsonCbor::cbor_writingFloat16()
{
    QBENCHMARK {
        QCborValue::fromVariant(m_testVariant).toCbor(QCborValue::UseFloat16);
    }
    QBENCHMARK {
        QtCbor::Writer::variantToCbor(m_testVariant, QCborValue::UseFloat16);
    }

    QByteArray expected = QCborValue::fromVariant(m_testVariant).toCbor(QCborValue::UseFloat16);
    QByteArray cbor = QtCbor::Writer::variantToCbor(m_testVariant, QCborValue::UseFloat16);

    QCOMPARE(cbor, expected);
}

void TestJsonCbor::cbor_parsingStandard()
{
    QByteArray cbor = QCborValue::fromVariant(m_testVariant).toCbor();

    QBENCHMARK {
        QCborValue::fromCbor(cbor).toVariant();
    }
    QBENCHMARK {
        QtCbor::Parser::cborToVariant(cbor);
    }

    QVariant expected = QCborValue::fromCbor(cbor).toVariant();
    QVariant variant = QtCbor::Parser::cborToVariant(cbor);

    QCOMPARE(variant, expected);
}

void TestJsonCbor::cbor_parsingFloat16()
{
    QByteArray cbor = QCborValue::fromVariant(m_testVariant).toCbor(QCborValue::UseFloat16);

    QBENCHMARK {
        QCborValue::fromCbor(cbor).toVariant();
    }
    QBENCHMARK {
        QtCbor::Parser::cborToVariant(cbor);
    }

    QVariant expected = QCborValue::fromCbor(cbor).toVariant();
    QVariant variant = QtCbor::Parser::cborToVariant(cbor);

    QCOMPARE(variant, expected);
}

static TestJsonCbor TEST_JSONCBOR(1000);
#include "tst_jsoncbor.moc"
