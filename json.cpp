#include "json.h"

using namespace std;

namespace json {

    Node LoadNode(istream& input);

    Node LoadArray(istream& input) {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        if (it == end) {
            throw ParsingError("String parsing error");
        }

        Array result;
        for (char c; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                ++it;
                break;
            } else if (ch == '\\') {
                ++it;
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            } else if (ch == '\n' || ch == '\r') {
                throw ParsingError("Unexpected end of line"s);
            } else {
                s.push_back(ch);
            }
            ++it;
        }

        return Node(move(s));
    }

    Node LoadDict(istream& input) {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        if (it == end) {
            throw ParsingError("String parsing error");
        }

        Dict result;
        for (char c; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.insert({move(key), LoadNode(input)});
        }

        return Node(move(result));
    }

    Node LoadBool(istream& input) {
        input.unget();
        string res;
        for(char c; input >> c && c != EOF;){
            if (c == '}' || c == ','){
                input.unget();
                break;
            }
            res += c;
        }
        if (res == "true"s)
            return Node(true);
        if (res == "false"s)
            return Node(false);
        throw ParsingError("String parsing error");
    }

    Node LoadNull(istream& input) {
        input.unget();
        string res;
        for(char c; input >> c && c != EOF;){
            if (c == '}' || c == ','){
                input.unget();
                break;
            }

            res += c;
        }
        if(res == "null"s)
            return Node(nullptr);
        else {
            throw ParsingError("String parsing error");
        }
    }

    Node LoadNumber(std::istream& input) {
        using namespace std::literals;

        input.unget();
        std::string parsed_num;

        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        if (input.peek() == '0') {
            read_char();
        } else {
            read_digits();
        }

        bool is_int = true;
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                try {
                    return Node(std::stoi(parsed_num));
                } catch (...) {}
            }
            return Node(std::stod(parsed_num));
        } catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        switch(c){
            case EOF:
                throw ParsingError("Failed to read from stream"s);
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case '\\':
                return LoadString(input);
            case 't':
                return LoadBool(input);
            case 'f':
                return LoadBool(input);
            case 'n':
                return LoadNull(input);
            default:
                return LoadNumber(input);
        }
    }

    Node::Node(int value): value_(value) {}
    Node::Node(string value): value_(move(value)) {}
    Node::Node(double value): value_(value){}
    Node::Node(bool value): value_(value) {}
    Node::Node(nullptr_t ptr): value_(ptr) {}
    Node::Node(Array array): value_(move(array)) {}
    Node::Node(Dict map): value_(move(map)) {}

    const Array& Node::AsArray() const {
        if (this->IsArray())
            return get<Array>(value_);
        else
            throw std::logic_error("Json Array get error");
    }

    const Dict& Node::AsMap() const {
        if (this->IsMap())
            return get<Dict>(value_);
        else
            throw std::logic_error("Json Map get error");
    }

    int Node::AsInt() const {
        if (this->IsInt())
            return get<int>(value_);
        else
            throw std::logic_error("Json Int get error");
    }

    const string& Node::AsString() const {
        if (this->IsString())
            return get<std::string>(value_);
        else
            throw std::logic_error("Json String get error");
    }

    bool Node::AsBool() const {
        if (this->IsBool())
            return get<bool>(value_);
        else
            throw std::logic_error("Json Bool get error");
    }

    double Node::AsDouble() const {
        if (this->IsDouble()) {
            if (this->IsPureDouble())
                return get<double>(value_);
            else
                return get<int>(value_);
        }
        else
            throw std::logic_error("Json Double get error");
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(value_) || holds_alternative<int>(value_);
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }

    Document::Document(Node root): root_(move(root)) {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    void PrintValue(bool value, std::ostream& out){
        out << (value ? "true"sv : "false"sv);
    }

    void PrintValue(const std::string& value, std::ostream& out){
        out << '\"';
        for(char ch : value){
            switch (ch) {
                case '\n':
                    out << "\\n"sv;
                    break;
                case '\r':
                    out << "\\r"sv;
                    break;
                case '\"':
                    out << "\\\""sv;
                    break;
                case '\\':
                    out << "\\\\"sv;
                    break;
                default:
                    out << ch;
            }
        }
        out << '\"';
    }

    void PrintValue(const Array& value, std::ostream& out){
        out << '[';
        if (!value.empty()) {
            for (size_t i = 0; i < value.size() - 1; ++i) {
                PrintNode(value[i], out);
                out << ", "sv;
            }
            PrintNode(value[value.size() - 1], out);
        }
        out << ']';
    }

    void PrintValue(const Dict& value, std::ostream& out) {
        out << '{';
        bool is_last = false;
        for (const auto &p: value) {
            out << '"' << p.first <<  '"' << ": "sv;
            PrintNode(p.second, out);
            if(p == *(--value.end()))
                is_last = true;
            if(!is_last)
                out << ", "sv;
        }
        out << '}';
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
                [&out](const auto& value){ PrintValue(value, out); },
                node.GetValue());
    }

    void Print(const Document& doc, std::ostream& out) {
        PrintNode(doc.GetRoot(), out);
    }
}
