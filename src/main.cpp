#include <iostream>
#include <fstream>
#include <chrono>

#include "lexer.hpp"
#include "parser.hpp"
#include "evaluator.hpp"
#include "words.hpp"
#include "log.hpp"

std::string read_file(const char *filename)
{
    std::fstream file(filename);

    if (!file.is_open())
        logger::fatal("invalid filename provided '", filename, "'");

    return {(std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()};
}

void run(std::string &contents, int argc, char **argv)
{
    //auto start = std::chrono::high_resolution_clock::now();

    auto tokens = Lexer(contents).scan();

    Parser(tokens, words).parse();

    Evaluator(words, tokens, argc, argv).eval();

    //auto end = std::chrono::high_resolution_clock::now();

//    std::cout << "======== words ========\n";
//
//    for (auto &[k, v]: words)
//    {
//        if(v.index() == 0)
//            continue;
//        auto w_tokens = std::get<1>(v);
//        std::cout << k << '\n';
//        for (auto &tk: w_tokens)
//            std::cout << "\t" << tk << '\n';
//    }
//
//    std::cout << "======== tokens ========\n";
//
//    for (auto &token: tokens)
//        std::cout << token << '\n';

    //std::cout << "\nPipeline Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start) << '\n';
}

int main(int argc, char **argv)
{
    if (argc == 1)
        logger::fatal("You must provide a valid forth file path");

    std::string contents = read_file(argv[1]);

    run(contents, argc, argv);
}
