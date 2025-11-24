# QJsonVariant

QJsonVariant is a tiny Qt/QtCore-only helper that converts `QVariant` trees to and from JSON or CBOR. It offers both one-shot helpers (`fromJson`, `fromVariant`) and streaming readers/writers so you can walk large payloads incrementally without building a full `QJsonDocument` or `QCborValue` in memory.

## Highlights
- **Streaming-first API** – `QJsonVariantReader`, `QCborVariantReader`, and their writer counterparts let you pull values from any `QIODevice` (files, sockets, buffers) while tracking progress and container depth.
- **Drop-in converters** – `QJsonVariantWriter::fromVariant()` and friends mirror Qt's convenience API but avoid extra conversions.
- **Symmetric JSON/CBOR support** – the same `QVariantReader` semantics work for both formats, including string escaping, numeric parsing, and nested structures.
- **Fast & dependency-light** – no Qt GUI modules required; benchmarks (see `tests/benchmark`) compare favorably against `QJsonDocument`, `QCborValue`, XML, and `QDataStream` serialization.

## Feature Matrix
| Format | Reader | Writer | Notes |
| --- | --- | --- | --- |
| JSON | `QJsonVariantReader` | `QJsonVariantWriter` | Optional compact/indented output, UTF-8 BOM skipping, works with `QByteArray` or any `QIODevice`. |
| CBOR | `QCborVariantReader` | `QCborVariantWriter` | Honors `QCborStreamReader` length hints and supports `QCborValue::EncodingOptions` (e.g., `UseFloat16`). |

## Requirements
- Qt 5.15+ or Qt 6.x (`Qt::Core` only)
- CMake 3.16+
- A compiler compatible with your Qt build (C++17 recommended)

## Build & Integration
### As a CMake subproject
```cmake
# FetchContent, submodule, or local copy
add_subdirectory(external/QJsonVariant/src)

qt_add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE Qt${QT_VERSION_MAJOR}::Core QJsonVariant)
```

### Adding the sources directly
Copy the `.h`/`.cpp` files from `src/` into your Qt project (CMake/qmake/Qbs) and include `<qjsonvariantreader.h>`, `<qcborvariantwriter.h>`, etc. A convenience umbrella header lives at `src/QJsonVariant`.

## Usage
### Drop-in conversions
```cpp
QVariantMap payload {{"name", "Alice"}, {"age", 42}};

QByteArray json = QJsonVariantWriter::fromVariant(payload, /*compact=*/true);
QVariant fromJson = QJsonVariantReader::fromJson(json);

QByteArray cbor = QCborVariantWriter::fromVariant(payload, QCborValue::UseFloat16);
QVariant fromCbor = QCborVariantReader::fromCbor(cbor);
```

### Streaming parse with `QVariantReader`
```cpp
QJsonVariantReader reader(jsonData);
reader.enterContainer();           // enter the root object
while (reader.hasNext()) {
    const QString key = reader.readString();
    if (key == "tasks") {
        reader.enterContainer();
        while (reader.hasNext()) {
            QVariant task = reader.read();    // task is a QVariantMap
            // ... process task ...
        }
        reader.leaveContainer();
    } else {
        reader.read(); // consume and ignore this entry
    }
}
reader.leaveContainer();
```

### Writing incrementally
```cpp
QByteArray buffer;
QJsonVariantWriter writer(&buffer, /*compact=*/false);
writer.start();
writer.startMap();
writer.writeKeyValue("ok", true);
writer.writeValueSeparator();
writer.writeString("items");
writer.writeNameSeparator();
writer.startArray();
for (int value : {1, 2, 3}) {
    writer.writeVariant(value);
    if (value != 3)
        writer.writeValueSeparator();
}
writer.endArray();
writer.endMap();
```

The CBOR API mirrors the JSON writer (with `QCborVariantWriter`/`QCborVariantReader`) and accepts optional container lengths when known up front.

## Tests & Benchmarks
- `tests/json/tst_json.cpp` & `tests/cbor/tst_cbor.cpp` assert byte-for-byte parity with the official Qt JSON/CBOR helpers for a wide range of types (nulls, nested maps, `QDateTime`, Unicode, etc.).
- `tests/benchmark/tst_benchmark.cpp` runs `QTest` benchmarks that compare QJsonVariant against `QJsonDocument`, `QCborValue`, Qt XML, `QDataStream`, and the streaming APIs on a 10k-task workload.

## Repository Layout
```
src/                 library implementation + umbrella header
tests/json           JSON parity tests
tests/cbor           CBOR parity tests
tests/benchmark      Performance + serialization comparisons
```

## License & Contributions
QJsonVariant is licensed under the MIT License (see `LICENSE`). Issues and pull requests are welcome—whether it is performance work, new helpers, or additional documentation.
