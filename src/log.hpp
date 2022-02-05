#pragma once

#include <iostream>
#include <cstdlib>

#include "types.hpp"

namespace logger
{
    template<class ...A>
    void fatal(const char *message, A ...a)
    {
        std::cerr << "Fatal error: " << message;
        ((std::cerr << a), ...);
        std::exit(-1);
    }

    template<class ...A>
    void syntax_error(Token &token, const char *message, A ...a)
    {
        std::cerr
                << "["
                << token.line
                << ':'
                << token.column
                << "] Syntax error on token '"
                << token.lexeme
                << "'\n\tMessage: "
                << message;

        ((std::cerr << a), ...);
        std::exit(-1);
    }

    template<class ...A>
    void syntax_error(size_t line, size_t column, char c, const char *message, A ...a)
    {
        std::cerr
                << "["
                << line
                << ':'
                << column
                << "] Syntax error on token"
                << " '"
                << c
                << "'\n\tMessage: "
                << message;

        ((std::cerr << a), ...);
        std::exit(-1);
    }

    template<class ...A>
    void runtime_error(Token& token, const char *message, A ...a)
    {
        std::cerr
            << "\nRuntime Error: "
            << '['
            << token.line
            << ':'
            << token.column
            << ']'
            << "\n\tMessage: "
            << message;
        ((std::cerr << a), ...);
        std::exit(-1);
    }
}