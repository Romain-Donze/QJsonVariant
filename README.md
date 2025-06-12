# QJsonVariant

**QJsonVariant** is a lightweight C++/Qt library for serializing and deserializing `QVariant` data using JSON and CBOR formats. It provides flexible readers and writers that allow structured parsing and generation of Qt types from and into human-readable or compact binary formats.

## ✨ Features

- Read/write `QVariant`, `QVariantList`, and `QVariantMap` from/to:
  - **JSON** using `QJsonVariantReader` and `QJsonVariantWriter`
  - **CBOR** using `QCborVariantReader` and `QCborVariantWriter`
- High-performance, streaming-based readers and writers. (Faster than using `QJsonDocument::fromVariant::toJson` and/or `QJsonDocument::fromJson::toVariant`
- Works directly with `QIODevice` and `QByteArray`.
- Compact and readable JSON output modes.
- Minimal dependencies — only requires Qt Core (Qt5 or Qt6).

## 🔧 Dependencies

- Qt 5.15+ or Qt 6.x
- CMake 3.16 or newer

## 📚 Getting Started

### 🛠️ Integration

1. Add the `.h` and `.cpp` files to your Qt project.
2. Include the header in your source files:

```cpp
#include <QJsonVariant>
```

### Reading JSON

```cpp
QByteArray json = R"({
    "person": {
        "name": "Alice",
        "age": 30,
        "hobbies": ["reading", "cycling"]
    }
})";

QJsonParseError error;
QVariant variant = QJsonVariantReader::fromJson(jsonData, &error);

QString name = variant.toMap().value("person").toMap().value("name").toString();
int age = variant.toMap().value("person").toMap().value("age").toInt();
QString secondHobby = variant.toMap().value("person").toMap().value("hobbies").toList().at(1).toString();
```

### Writing JSON

```cpp
QVariantMap map;
map["hello"] = "world";
QByteArray json = QJsonVariantWriter::fromVariant(map, /*compact=*/true);
```

### Reading CBOR

```cpp
QFile file("data.cbor");
if (file.open(QIODevice::ReadOnly)) {
    QCborParserError error;
    QVariant variant = QCborVariantReader::fromCbor(&file, &error);
}
```

### Writing CBOR

```cpp
QVariantMap data;
data["bool"] = true;
data["list"] = QVariantList{1, 2, 3};

QByteArray cbor = QCborVariantWriter::fromVariant(data);
```

## 🧪 Testing

The library has been tested against the basics tests from Qt JSON library. It validates:

- Basic and complex types (numbers, strings, booleans)
- Arrays and nested objects
- String escaping
- Error handling

## 📄 License

MIT License — see the [LICENSE](LICENSE) file for details.

## 🤝 Contributions

Contributions are welcome! Feel free to submit a pull request or open an issue if you encounter bugs or have suggestions for improvements.

