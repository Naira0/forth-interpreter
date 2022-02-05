#pragma once

#include <sstream>
#include <variant>
#include <vector>

enum class TokenType
{
    NUMBER, STRING, ARRAY, IDENTIFIER,

    COLON, SEMI_COLON, DOT,

    MINUS, PLUS, STAR, SLASH,

    LESS_THEN, GREATER_THEN, LESS_EQUAL, GREATER_EQUAL,

    EQUAL, BANG_EQUAL, QUESTION, AT, BANG,

    PLUS_BANG, MINUS_BANG, STAR_BANG, SLASH_BANG,

    AND, OR, INVERT, IF, THEN, ELSE,

    DO, LOOP, BEGIN, UNTIL, VARIABLE, CONSTANT,

    END,
};

static const char *TK_type_str[] =
{
        "Number", "String", "Array", "Identifier",
        "Colon", "Semi-colon", "Dot",
        "Minus", "Plus", "Star", "Slash",
        "Less then", "greater then", "less equal", "greater equal",
        "Equal", "Bang equal", "Question", "At", "Bang",
        "Plus bang", "Minus bang", "Star bang", "slash bang",
        "And", "Or", "Invert", "If", "Then", "Else",
        "Do", "Loop", "Begin", "Until", "Variable", "Constant",
        "End",
};

class Token;

using Array = std::vector<std::variant<double, std::string>>;
using Value = std::variant<std::monostate, double, std::string, Array, Token*>;

class Token
{
public:

    Token()
            : type(TokenType::END), line(0), column(0) {}

    Token(TokenType type, size_t line, size_t column, std::string &lexeme)
            :
            type(type),
            line(line),
            column(column),
            lexeme(std::move(lexeme)),
            value(std::monostate()) {}

    Token(TokenType type, size_t line, size_t column, std::string &lexeme, Value &value)
            :
            type(type),
            line(line),
            column(column),
            lexeme(std::move(lexeme)),
            value(std::move(value)) {}

    Token(TokenType type, Value value)
            :
            type(type),
            value(std::move(value)) {}

    std::string to_str() const
    {
        std::stringstream ss;

        const char *type_str = TK_type_str[(size_t) type];

        ss <<
           "Type(" << type_str << ") " <<
           "Position(" << line << ':' << column << ") " <<
           "Lexeme(" << lexeme << ") " <<
           "Value(";

        switch (value.index())
        {
            case 0: ss << "None"; break;
            case 1: ss << std::get<double>(value); break;
            case 2: ss << std::get<std::string>(value); break;
        }

        ss << ")";

        return ss.str();
    }


    friend std::ostream &operator<<(std::ostream &os, Token &tk)
    {
        return os << tk.to_str();
    }

    TokenType type;
    size_t line;
    size_t column;
    std::string lexeme;
    Value value;
};