# QJsonVariant

**QJsonVariant** is a lightweight JSON parser/writer designed to make working with JSON in Qt more intuitive with QVariant.

## ✨ Features

- Seamless writing to JSON values from `QVariant` instead of `QJsonDocument`.
- Seamless parsing from JSON values into `QVariant` instead of `QJsonDocument`.

## 📚 Usage Examples

### Creating from a JSON string

```cpp
QString json = R"({
    "person": {
        "name": "Alice",
        "age": 30,
        "hobbies": ["reading", "cycling"]
    }
})";

QVariant variant = QtJson::Parser::jsonToVariant(json);

QString name = variant.toMap().value("person").toMap().value("name").toString();
int age = variant.toMap().value("person").toMap().value("age").toInt();
QString secondHobby = variant.toMap().value("person").toMap().value("hobbies").toList().at(1).toString();
```

## 🔧 Dependencies

- Qt 5 or later
- No third-party libraries

## 🛠️ Integration

1. Add the `.h` and `.cpp` files to your Qt project.
2. Include the header in your source files:

```cpp
#include <QJsonVariant>
```

3. Compile as usual using `qmake`, `cmake`, or other Qt build tools.

## 🧪 Testing

The library has been tested against the basics tests from Qt JSON library. It validates:

- Basic and complex types (numbers, strings, booleans)
- Arrays and nested objects
- Error handling

## 📄 License

MIT License — see the [LICENSE](LICENSE) file for details.

## 🤝 Contributions

Contributions are welcome! Feel free to submit a pull request or open an issue if you encounter bugs or have suggestions for improvements.

