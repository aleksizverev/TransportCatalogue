#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:

        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;

        Node(double value);
        Node(int value);
        Node(std::string value);
        Node(bool value);
        Node(nullptr_t ptr);
        Node(Array array);
        Node(Dict map);

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsPureDouble() const;
        bool IsDouble() const;
        bool IsInt() const;

        const Value& GetValue() const { return value_; }

        bool operator==(const Node& other) const {
            return value_ == other.GetValue();
        }

        bool operator!=(const Node& other) const {
            return value_ != other.GetValue();
        }


    private:
        Value value_;
    };

    class Document {
    public:
        Document(Node root);
        const Node& GetRoot() const;

        bool operator==(const Document& other) const {
            return root_ == other.GetRoot();
        }

        bool operator!=(const Document& other) const {
            return root_ != other.GetRoot();
        }
    private:
        Node root_;
    };

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return {out, indent_step, indent_step + indent};
        }
    };

    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream& out);
    void PrintValue(bool value, std::ostream& out);
    void PrintValue(const std::string& value, std::ostream& out);
    void PrintValue(const Dict& value, std::ostream& out);
    void PrintValue(const Array& value, std::ostream& out);

    void PrintNode(const Node& node, std::ostream& out);

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& out);
}