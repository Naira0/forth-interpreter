#pragma once

#include <map>
#include <string>
#include <variant>
#include <cmath>
#include <conio.h>

#include "types.hpp"
#include "stack.hpp"

typedef void(*builtin_fn)(Stack<Value>&);

using Words = std::map<
        std::string,
        std::variant<
                builtin_fn,
                std::vector<Token>
                >
                >;

// for copy and pasting because im lazy
// void (Stack<Value>& stack)

void dup(Stack<Value>& stack)
{
    if(stack.empty())
        return;
    stack.push(stack.back());
}

void nl(Stack<Value>& stack)
{
    std::cout << '\n';
}

void stack_len(Stack<Value>& stack)
{
    double len = stack.len();
    stack.push(len);
}

void emit(Stack<Value>& stack)
{
    if(stack.empty())
        return;
    if(stack.back().index() != 1)
        return;

    int value = std::get<double>(stack.back());
    std::cout << (char)value;

    stack.pop();
}

void program_exit(Stack<Value>& stack)
{
    int code = -1;

    if(!stack.empty())
    {
        Value& top = stack.back();

        if(top.index() != 0)
            code = std::get<double>(top);
    }

    exit(code);
}

void mod(Stack<Value>& stack)
{
    if(stack.len() < 2)
        return;

    auto [v_a, v_b] = stack.top_two();

    if(v_a.index() != 1 || v_b.index() != 0)
        return;

    double
        a = std::get<double>(v_a),
        b = std::get<double>(v_b);

    stack.pop_n(2);
    stack.push(std::fmod(a, b));
}

void drop(Stack<Value>& stack)
{
    if(stack.empty())
        return;
    stack.pop();
}

void key(Stack<Value>& stack)
{
    double key = _getch();
    stack.push(key);
}

void rotate(Stack<Value>& stack)
{
    if(stack.empty())
        return;

    Value& v_amount = stack.back();

    if(v_amount.index() != 1)
        return;

    const size_t amount = std::get<double>(v_amount);

    if(amount > stack.len())
        return;

    stack.pop();

    auto values = stack.get_vec_from_back(amount, true);

    for(auto &val : values)
        stack.push(val);
}

void composite(Stack<Value>& stack)
{
    if(stack.empty())
        return;

    Value& v_amount = stack.back();

    if(v_amount.index() != 1)
        return;

    size_t amount = std::get<double>(v_amount);

    Array output;

    output.reserve(amount);

    stack.pop();

    for(size_t i = 0; !stack.empty() && i < amount; i++)
    {
        double val = std::get<double>(stack.back());
        output.push_back(val);
        stack.pop();
    }

    stack.push(std::move(output));
}

static Words words =
{
        {"dup",       dup},
        {"nl",        nl},
        {"emit",      emit},
        {"stack-len", stack_len},
        {"exit",      program_exit},
        {"mod",       mod},
        {"drop",      drop},
        {"key",       key},
        {"rotate",    rotate},
        {"composite", composite}
};