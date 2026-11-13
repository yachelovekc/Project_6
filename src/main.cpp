#include "GameEngine.hpp"
#include "InputParser.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

void configure_console_utf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

}

int main(int argc, char* argv[]) {
    configure_console_utf8();

    try {
        const auto cli = parse_cli_or_prompt(argc, argv, std::cin, std::cout);
        if (cli.show_help) {
            print_usage(std::cout, argc > 0 ? argv[0] : "tictactoe_demo");
            return EXIT_SUCCESS;
        }

        const GameEngine engine(cli.config);
        engine.run(std::cout);
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Помилка: " << ex.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Невідома критична помилка.\n";
        return EXIT_FAILURE;
    }
}
