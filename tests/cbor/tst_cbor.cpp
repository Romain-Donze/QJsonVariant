#include <QtTest>

#include "qcborvariantwriter.h"
#include "qcborvariantreader.h"

class TestCbor : public QObject
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

private:
    QVariant m_testVariant;
};

void TestCbor::initTestCase()
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

void TestCbor::cleanupTestCase()
{

}

void TestCbor::writing_data()
{
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<int>("options");

    QTest::newRow("writing default") << m_testVariant << 0;
    QTest::newRow("writing float16") << m_testVariant << (int)QCborValue::UseFloat16;
}

void TestCbor::writing()
{
    QFETCH(QVariant, variant);
    QFETCH(int, options);

    QByteArray expected = QCborValue::fromVariant(variant).toCbor((QCborValue::EncodingOptions)options);
    QByteArray result = QCborVariantWriter::fromVariant(variant, options);

    QCOMPARE(result, expected);
}

void TestCbor::parsing_data()
{
    QTest::addColumn<QVariant>("variant");
    QTest::addColumn<int>("options");

    QTest::newRow("parsing default") << m_testVariant << 0;
    QTest::newRow("parsing float16") << m_testVariant << (int)QCborValue::UseFloat16;
}

void TestCbor::parsing()
{
    QFETCH(QVariant, variant);
    QFETCH(int, options);

    QByteArray cbor = QCborValue::fromVariant(variant).toCbor((QCborValue::EncodingOptions)options);

    QVariant expected = QCborValue::fromCbor(cbor).toVariant();
    QVariant result = QCborVariantReader::fromCbor(cbor);

    QCOMPARE(result, expected);
}

QTEST_APPLESS_MAIN(TestCbor)

#include "tst_cbor.moc"
