#pragma once

#include <map>

#include "types.hpp"
#include "log.hpp"
#include "words.hpp"

class Parser
{
public:
    Parser(std::vector<Token>& tokens, Words& words)
    : tokens(tokens), words(words)
    {}

    void parse()
    {
        altered_tokens.reserve(tokens.size());

        while(!at_end())
        {
            start = current;
            parse_token();
        }

        tokens = std::move(altered_tokens);
    }

private:

    using enum TokenType;

    std::vector<Token>& tokens;
    std::vector<Token> altered_tokens;

    Words& words;

    Token EMPTY_TOKEN{};

    size_t current = 0;
    size_t start   = 0;

    void parse_token()
    {
        Token& token = advance();

        if(token.type == COLON)
            scan_word();
        // checks to see if the token is allowed outside words
        else if(token.type > INVERT && token.type < VARIABLE)
            logger::syntax_error(token, "token is only allowed within words");
        else
            altered_tokens.push_back(std::move(token));
    }

    void scan_word()
    {
        Token& current_tk = advance();

        if(current_tk.type != IDENTIFIER)
            logger::syntax_error(current_tk, "expected identifier");

        std::string word_name = std::move(current_tk.lexeme);

        if(words.contains(word_name))
            logger::syntax_error(current_tk, "word has been previously defined or is reserved");

        while(!at_end() && peek().type != SEMI_COLON)
            eval();

        if(peek().type != SEMI_COLON)
            logger::syntax_error(peek(), "unterminated word");

        std::vector<Token> slice;

        start += 2; // moves past colon and identifier

        slice.reserve(current-start);

        for(size_t i = start; i < current; i++)
            slice.push_back(std::move(tokens[i]));

        words[word_name] = std::move(slice);

        current++;
    }

    void eval()
    {
        Token& token = advance();

        bool ok = true, had_if = false;

        switch(token.type)
        {
            case IF:    ok = scan_until(THEN); had_if = true;  break;
            case ELSE:  ok = had_if;           had_if = false; break;
            case DO:    ok = scan_until(LOOP);  break;
            case BEGIN: ok = scan_until(UNTIL); break;
            case THEN:
            case LOOP:
            case UNTIL: ok = false; break;
        }

        if(!ok)
            logger::syntax_error(token, "invalid expression");
    }

    inline bool scan_until(TokenType type)
    {
        while(!at_end() && peek().type != type)
            current++;

        current++;

        return peek().type != type;
    }

    inline Token& peek()
    {
        if(at_end())
            return EMPTY_TOKEN;
        return tokens[current];
    }

    inline Token& advance()
    {
        if(at_end())
            return EMPTY_TOKEN;
        return tokens[current++];
    }

    inline bool at_end()
    {
        return current >= tokens.size();
    }

};