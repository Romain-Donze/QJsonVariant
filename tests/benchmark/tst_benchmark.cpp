#include <QtTest>

#include "qjsonvariantwriter.h"
#include "qjsonvariantreader.h"
#include "qcborvariantwriter.h"
#include "qcborvariantreader.h"

class TestBenchmark : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;


private slots:
    void initTestCase();

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

private:
    QJsonDocument m_doc;
    QVariant m_variant;
    QByteArray m_jsonIndented;
    QByteArray m_jsonCompact;
    QCborValue m_cborValue;
    QByteArray m_cborIndented;
    QByteArray m_cborCompact;

    bool m_compact;
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

QTEST_APPLESS_MAIN(TestBenchmark)

#include "tst_benchmark.moc"
