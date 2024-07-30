#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


using Value = std::variant<std::monostate, Array, Dict, bool, int, double, std::string>;

class Node final : public Value {
public:
    using Value::Value;

    bool IsInt() const {
        return std::holds_alternative<int>(*this);
    }
    int AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Not an int");
        }
        return std::get<int>(*this);
    }

    bool IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }
    bool IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    double AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Not a double");
        }
        return IsPureDouble() ? std::get<double>(*this) : AsInt();
    }

    bool IsBool() const {
        return std::holds_alternative<bool>(*this);
    }
    bool AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Not a bool");
        }
        return std::get<bool>(*this);
    }

    bool IsNull() const {
        return std::holds_alternative<std::monostate>(*this);
    }

    bool IsArray() const {
        return std::holds_alternative<Array>(*this);
    }
    const Array& AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Not an array");
        }
        return std::get<Array>(*this);
    }

    bool IsString() const {
        return std::holds_alternative<std::string>(*this);
    }
    const std::string& AsString() const {
        if (!IsString()) {
            throw std::logic_error("Not a string");
        }
        return std::get<std::string>(*this);
    }

    bool IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }
    const Dict& AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Not a map");
        }
        return std::get<Dict>(*this);
    }

    bool operator==(const Node& rhs) const {
        return static_cast<const Value&>(*this) == static_cast<const Value&>(rhs);
    }

    const Value& GetValue() const {
        return *this;
    }
};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& GetRoot() const {
        return root_;
    }

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json