#pragma once

#include "GameEngine.hpp"

#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

struct CliResult {
    bool show_help{false};
    GameConfig config{};
};

GameConfig parse_config_stream(std::istream& is);
GameConfig parse_config_file(const std::string& path);

std::size_t parse_board_size(std::string_view text);
std::uint64_t parse_seed(std::string_view text);
bool parse_bool(std::string_view text);

CliResult parse_cli_or_prompt(int argc,
                              char* argv[],
                              std::istream& in,
                              std::ostream& out);

void print_usage(std::ostream& out, std::string_view program_name);
