#pragma once

#include <string>
#include <string_view>
#include <map>

#include "types.hpp"
#include "log.hpp"

static const std::map<std::string_view , TokenType> keyword_table =
{
        {"and",      TokenType::AND},
        {"or",       TokenType::OR},
        {"invert",   TokenType::INVERT},
        {"if",       TokenType::IF},
        {"then",     TokenType::THEN},
        {"else",     TokenType::ELSE},
        {"do",       TokenType::DO},
        {"loop",     TokenType::LOOP},
        {"begin",    TokenType::BEGIN},
        {"until",    TokenType::UNTIL},
        {"variable", TokenType::VARIABLE},
        {"constant", TokenType::CONSTANT}
};

class Lexer
{
    using enum TokenType;

public:

    Lexer(std::string& source)
    : source(std::move(source))
    {}

    std::vector<Token> scan()
    {
        tokens.reserve(source.size());

        while(!at_end())
        {
            start = current;
            scan_token();
        }

        set(END);

        return std::move(tokens);
    }

private:
    const std::string  source;
    std::vector<Token> tokens;

    size_t
         current = 0,
         start   = 0,
         line    = 1,
         column  = 1;

private:

    void scan_token()
    {
        char c = advance();

        switch(c)
        {
            case ':': set(COLON);      break;
            case ';': set(SEMI_COLON); break;
            case '.': set(DOT);        break;
            case '?': set(QUESTION);   break;
            case '@': set(AT);         break;
            case '=': set(EQUAL);      break;
            case '!': set(match_next('=') ? BANG_EQUAL : BANG);       break;
            case '+': set(match_next('!') ? PLUS_BANG : PLUS);             break;
            case '*': set(match_next('!') ? STAR_BANG : STAR);             break;
            case '/': set(match_next('!') ? SLASH_BANG : SLASH);           break;
            case '<': set(match_next('=') ? LESS_EQUAL : LESS_THEN);       break;
            case '>': set(match_next('=') ? GREATER_EQUAL : GREATER_THEN); break;
            case '-':
            {
                if(is_digit(peek()))
                    scan_number();
                else
                    set(match_next('!') ? MINUS_BANG : MINUS);
                break;
            }
            case '(': scan_comment(); break;
            case '"': scan_string(); break;
            case ' ':
            case '\t':
            case '\r': return;
            case '\n': line++; column = 1; return;
            default:
            {
                if(is_alpha(c))
                    scan_identifier();
                else if(is_digit(c))
                    scan_number();
                else
                    logger::syntax_error(line, column, c, "invalid token found");
            }
        }

        if(!is_terminator(peek()))
            logger::syntax_error(line, column, peek(), "Token must be terminated");
    }

    void scan_identifier()
    {
        if(!is_terminator(peek()))
        {
            while(!at_end() && !is_terminator(peek_next()))
                advance();
            advance();
        }

        std::string text = source.substr(start, current - start);
        TokenType   type = keyword_table.contains(text) ? keyword_table.at(text) : IDENTIFIER;

        if(type == VARIABLE || type == CONSTANT)
            scan_var();

        set(type);
    }

    void scan_var()
    {
        advance();

        start = current;

        while(!at_end() && !is_terminator(peek_next()))
            advance();

        if(is_terminator(peek()))
            logger::syntax_error(line, column, ' ', "you must provide an identifier for this variable");

        advance();
    }

    void scan_number()
    {
        loop: while(!at_end() && is_digit(peek()))
            advance();

        if(peek() == '.' && is_digit(peek_next()))
        {
            advance();
            goto loop;
        }

        set(NUMBER);
    }

    void scan_comment()
    {
        while(!at_end() && peek() != ')')
            advance();
        advance();
    }

    void scan_string()
    {
        start++; // moves past the '"' char
        column++;

        while(!at_end() && peek() != '"')
        {
            if(peek() == '\n')
            {
                column = 1;
                line++;
            }
            advance();
        }

        if(at_end())
            return logger::syntax_error(line, column, ' ', "String is not closed");

        set(STRING);

        current++;
        column++;
    }

    inline char advance()
    {
        if(current >= source.size())
            return '\0';
        column++;
        return source[current++];
    }

    inline bool match_next(char c)
    {
        if(current >= source.size())
            return false;
        if(source[current] != c)
            return false;

        column++;
        current++;

        return true;
    }

    inline char peek() const
    {
        if(current >= source.size())
            return '\0';
        return source[current];
    }

    inline char peek_next() const
    {
        if(current+1 >= source.size())
            return '\0';
        return source[current+1];
    }

    inline bool at_end()
    {
        return current >= source.size();
    }

    inline bool is_alpha(char c)
    {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_' || c == '-';
    }

    inline bool is_digit(char c)
    {
        return (c >= '0' && c <= '9');
    }

    inline bool is_terminator(char c)
    {
        return
           c == '\n'
        || c == ' '
        || c == '\r'
        || c == '\t'
        || c == '\0';
    }

    inline void set(TokenType type)
    {
        std::string lexeme = source.substr(start, current-start);

        Value value { get_value(type, lexeme) };
        Token token { type, line, column, lexeme, value };

        tokens.emplace_back(std::move(token));
    }

    inline Value get_value(TokenType type, std::string& str)
    {
        switch(type)
        {
            case NUMBER: return std::stod(str);
            case STRING: return str;
            default:     return std::monostate();
        }
    }
};