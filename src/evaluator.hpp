#pragma once

#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <iomanip>

#include "types.hpp"
#include "parser.hpp"
#include "stack.hpp"
#include "words.hpp"

class Evaluator
{
    using enum TokenType;

    using VarTable = std::map<std::string, Token>;

public:
    Evaluator(Words& words, std::vector<Token>& tokens, int argc, char **argv)
    : words(words), tokens(tokens)
    {
        global_variables.emplace("argc", Token(NUMBER, (double)argc));

        Array args;

        args.reserve(argc);

        for(size_t i = 0; i < argc; i++)
            args.emplace_back(argv[i]);

        global_variables.emplace("argv", Token(ARRAY, std::move(args)));
    }

    void eval()
    {
        for(auto &token : tokens)
        {
            eval_token(token, global_variables);
        }
    }

private:
    Words&              words;
    std::vector<Token>& tokens;
    Stack<Value>        stack;

    VarTable global_variables;

    void eval_token(Token& token, VarTable& vars)
    {
        if(token.value.index() != 0)
            return stack.push(token.value);

        if(token.type == IDENTIFIER)
        {
            if(words.contains(token.lexeme))
                return run_word(token.lexeme);
            else if(vars.contains(token.lexeme) || global_variables.contains(token.lexeme))
            {
                stack.push(&vars[token.lexeme]);
            }
        }

        if(token.type > DOT && token.type < QUESTION)
            return do_binary_arithmetic(token);
        if(token.type > AT && token.type < AND)
            return do_var_op(token);

        if(token.type != END && stack.empty())
            logger::runtime_error(token, "stack is empty");

        switch(token.type)
        {
            case DOT: print_top(token); break;
            case VARIABLE:
            {
                token.value = std::move(stack.back());

                stack.pop();

                vars[token.lexeme] = token;

                break;
            }
            case AT:
            {
                Token *tk = get_var(token);

                stack.pop();
                stack.push(std::move(tk->value));

                break;
            }
            case QUESTION:
            {
                Token *tk = get_var(token);

                print_value(tk->value);

                stack.pop();

                break;
            }
        }
    }

    void do_binary_arithmetic(Token& token)
    {
        if(stack.len() < 2)
            logger::runtime_error(token, "stack state is invalid for binary operator");

        auto [a, b]   = top_nums(token);
        double output = 0;

        switch(token.type)
        {
            case PLUS:          output = a + b; break;
            case MINUS:         output = a - b; break;
            case SLASH:         output = a / b; break;
            case STAR:          output = a * b; break;
            case EQUAL:         output = (a == b ? -1 : 0); break;
            case BANG_EQUAL:    output = (a != b ? -1 : 0); break;
            case LESS_THEN:     output = (a < b  ? -1 : 0); break;
            case GREATER_THEN:  output = (a > b  ? -1 : 0); break;
            case LESS_EQUAL:    output = (a <= b ? -1 : 0); break;
            case GREATER_EQUAL: output = (a >= b ? -1 : 0); break;
            default: logger::runtime_error(token, "invalid operator");
        }

        stack.pop_n(2);
        stack.push(output);
    }

    void do_var_op(Token& token)
    {
        auto [a, b] = stack.top_two();

        if(stack.len() < 2 && a.index() != 4 || b.index() != 4)
            logger::runtime_error(token, "top value on stack is not a variable");

        Token *tk = std::get<Token*>(b);

        if(tk->type == CONSTANT)
            logger::runtime_error(token, "cannot modify a constant");

        double value = std::get<double>(a), current = std::get<double>(tk->value);

        switch(token.type)
        {
            case BANG: tk->value = std::move(a);        break;
            case PLUS_BANG: tk->value = current+value;  break;
            case MINUS_BANG: tk->value = current-value; break;
            case STAR_BANG: tk->value = current*value;  break;
            case SLASH_BANG: tk->value = current/value; break;
        }

        stack.pop_n(2);
    }

    void run_word(std::string &word_name)
    {
        auto word = words.at(word_name);

        VarTable vars;

        if(word.index() == 0)
        {
            builtin_fn fn = std::get<0>(word);
            return fn(stack);
        }

        std::vector<Token> word_tokens = std::get<1>(word);

        for(size_t i = 0; i < word_tokens.size(); i++)
        {
            Token& token = word_tokens[i];

            switch(token.type)
            {
                case IF:
                {
                    bool truthful = is_truthful();

                    auto at_end = [&i, &word_tokens] () -> bool
                    {
                        return i < word_tokens.size() && word_tokens[i].type != THEN;
                    };

                    if(truthful)
                    {
                        while(at_end())
                        {
                            if(word_tokens[i].type == ELSE)
                            {
                                while(at_end())
                                    i++;
                                break;
                            }
                            eval_token(word_tokens[i++], vars);
                        }
                    }
                    else
                    {
                        while(at_end())
                        {
                            if(word_tokens[i].type == ELSE)
                            {
                                while(at_end())
                                    eval_token(word_tokens[i++], vars);
                                break;
                            }
                            i++;
                        }
                    }
                    break;
                }
                case BEGIN:
                {
                    i++;
                    size_t start = i;
                    while(is_truthful())
                    {
                        while(word_tokens[i].type != UNTIL)
                            eval_token(word_tokens[i++], vars);
                        i = start;
                    }
                    break;
                }
                case DO:
                {
                    i++;
                    size_t start = i;

                    if(stack.len() < 2)
                        logger::runtime_error(word_tokens[i], "Stack is invalid state for do loop");

                    auto [end, begin] = top_nums(word_tokens[i]);

                    stack.pop_n(2);

                    for(; begin < end; begin++)
                    {
                        while(word_tokens[i].type != LOOP)
                        {
                            if(word_tokens[i].lexeme == "i")
                                stack.push((double)begin);
                            else
                                eval_token(word_tokens[i], vars);
                            i++;
                        }
                        i = start;
                    }
                    break;
                }
                default: eval_token(token, vars);
            }
        }
    }

    inline void print_top(Token& token)
    {
        if(stack.empty())
            logger::runtime_error(token, "Stack is empty");

        Value& val = stack.back();

        print_value(val);

        stack.pop();
    }

    inline void print_value(Value& val)
    {
        switch(val.index())
        {
            case 1: std::cout << std::get<double>(val); break;
            case 2: std::cout << std::get<std::string>(val); break;
        }
    }

    inline bool is_truthful()
    {
        if(stack.empty())
            return false;

        Value& top = stack.back();

        if(top.index() != 1)
            return false;

        return std::get<double>(top) != 0;
    }

    std::pair<double, double> top_nums(Token& token)
    {
        auto [v_a, v_b] = stack.top_two();

        if(v_a.index() != 1 || v_b.index() != 1)
            logger::runtime_error(token, "top two stack items are not numeric values");

        return {std::get<double>(v_a), std::get<double>(v_b)};
    }

    Token *get_var(Token& token)
    {
        Value &val = stack.back();

        if(val.index() != 4)
            logger::runtime_error(token, "top value on stack is not a variable ", val.index());

        return std::get<Token*>(val);
    }
};