#include "InputParser.hpp"
#include "Board.hpp"

#include <charconv>
#include <fstream>
#include <random>
#include <stdexcept>
#include <system_error>

namespace {

std::string_view trim(std::string_view sv) noexcept {
    while (!sv.empty() &&
           (sv.front() == ' ' || sv.front() == '\t' ||
            sv.front() == '\r' || sv.front() == '\n')) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() &&
           (sv.back() == ' ' || sv.back() == '\t' ||
            sv.back() == '\r' || sv.back() == '\n')) {
        sv.remove_suffix(1);
    }
    return sv;
}

template <typename UInt>
UInt parse_strict_uint(std::string_view text, std::string_view field_name) {
    const std::string_view s = trim(text);
    if (s.empty()) {
        throw std::invalid_argument(
            "Порожнє значення для поля '" + std::string(field_name) + "'.");
    }
    for (char ch : s) {
        if (ch < '0' || ch > '9') {
            throw std::invalid_argument(
                "Очікується невід'ємне ціле число для '" +
                std::string(field_name) + "', отримано: '" + std::string(s) + "'.");
        }
    }

    UInt value{};
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec == std::errc::result_out_of_range) {
        throw std::out_of_range(
            "Число занадто велике для '" + std::string(field_name) + "': '" +
            std::string(s) + "'.");
    }
    if (ec != std::errc{} || ptr != s.data() + s.size()) {
        throw std::invalid_argument(
            "Некоректне число для '" + std::string(field_name) + "': '" +
            std::string(s) + "'.");
    }
    return value;
}

std::uint64_t generate_random_seed() {
    std::random_device rd;
    return (static_cast<std::uint64_t>(rd()) << 32) ^
           static_cast<std::uint64_t>(rd());
}

}

std::size_t parse_board_size(std::string_view text) {
    const auto value = parse_strict_uint<std::size_t>(text, "size");
    if (value < Board::kMinSize || value > Board::kMaxSize) {
        throw std::out_of_range(
            "Розмір поля n має бути в межах [" +
            std::to_string(Board::kMinSize) + ", " +
            std::to_string(Board::kMaxSize) + "].");
    }
    return value;
}

std::uint64_t parse_seed(std::string_view text) {
    return parse_strict_uint<std::uint64_t>(text, "seed");
}

bool parse_bool(std::string_view text) {
    const std::string_view s = trim(text);
    if (s == "true" || s == "1") {
        return true;
    }
    if (s == "false" || s == "0") {
        return false;
    }
    throw std::invalid_argument(
        "Очікується логічне значення (true/false/1/0), отримано: '" +
        std::string(s) + "'.");
}

GameConfig parse_config_stream(std::istream& is) {
    GameConfig config{};
    config.seed = generate_random_seed();

    bool has_size = false;
    bool has_seed = false;
    bool has_print = false;

    std::string line;
    std::size_t line_num = 0;

    while (std::getline(is, line)) {
        ++line_num;
        const std::string_view trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }

        const std::size_t eq_pos = trimmed.find('=');
        if (eq_pos == std::string_view::npos) {
            throw std::invalid_argument(
                "Синтаксична помилка в рядку " + std::to_string(line_num) +
                ": очікується формат 'ключ=значення'.");
        }

        const std::string_view key = trim(trimmed.substr(0, eq_pos));
        const std::string_view val = trim(trimmed.substr(eq_pos + 1));

        if (key == "size") {
            if (has_size) {
                throw std::invalid_argument(
                    "Повторне визначення ключа 'size' у рядку " +
                    std::to_string(line_num) + ".");
            }
            config.board_size = parse_board_size(val);
            has_size = true;
        } else if (key == "seed") {
            if (has_seed) {
                throw std::invalid_argument(
                    "Повторне визначення ключа 'seed' у рядку " +
                    std::to_string(line_num) + ".");
            }
            config.seed = parse_seed(val);
            has_seed = true;
        } else if (key == "print_each_board") {
            if (has_print) {
                throw std::invalid_argument(
                    "Повторне визначення ключа 'print_each_board' у рядку " +
                    std::to_string(line_num) + ".");
            }
            config.print_each_board = parse_bool(val);
            has_print = true;
        } else {
            throw std::invalid_argument(
                "Невідомий ключ '" + std::string(key) + "' у рядку " +
                std::to_string(line_num) + ".");
        }
    }

    if (!is.eof() && is.fail()) {
        throw std::runtime_error("Помилка читання конфігураційного потоку.");
    }
    if (!has_size) {
        throw std::invalid_argument(
            "У конфігураційному файлі відсутній обов'язковий ключ 'size'.");
    }
    return config;
}

GameConfig parse_config_file(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Шлях до файлу не може бути порожнім.");
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error(
            "Не вдалося відкрити файл конфігурації: '" + path + "'.");
    }
    return parse_config_stream(file);
}

void print_usage(std::ostream& out, std::string_view program_name) {
    out << "Використання: " << program_name << " [параметри]\n"
        << "Параметри:\n"
        << "  -n, --size <n>      Розмір поля n x n (від "
        << Board::kMinSize << " до " << Board::kMaxSize << ")\n"
        << "  -s, --seed <число>  Початкове значення генератора випадкових чисел\n"
        << "  -f, --file <шлях>   Шлях до текстового файлу конфігурації\n"
        << "      --compact       Не виводити поле після кожного ходу\n"
        << "  -h, --help          Показати цю довідку\n\n"
        << "Якщо --size і --file не вказані, параметри зчитуються зі стандартного вводу.\n";
}

CliResult parse_cli_or_prompt(int argc,
                              char* argv[],
                              std::istream& in,
                              std::ostream& out) {
    CliResult result{};
    std::optional<std::size_t> size_arg;
    std::optional<std::uint64_t> seed_arg;
    std::optional<std::string> file_arg;
    bool compact = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "-h" || arg == "--help") {
            result.show_help = true;
            return result;
        }
        if (arg == "--compact") {
            compact = true;
            continue;
        }
        if (arg == "-n" || arg == "--size") {
            if (i + 1 >= argc) {
                throw std::invalid_argument(
                    "Параметр '" + std::string(arg) + "' потребує значення.");
            }
            if (size_arg.has_value()) {
                throw std::invalid_argument("Повторний параметр --size.");
            }
            size_arg = parse_board_size(argv[++i]);
        } else if (arg == "-s" || arg == "--seed") {
            if (i + 1 >= argc) {
                throw std::invalid_argument(
                    "Параметр '" + std::string(arg) + "' потребує значення.");
            }
            if (seed_arg.has_value()) {
                throw std::invalid_argument("Повторний параметр --seed.");
            }
            seed_arg = parse_seed(argv[++i]);
        } else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= argc) {
                throw std::invalid_argument(
                    "Параметр '" + std::string(arg) + "' потребує значення.");
            }
            if (file_arg.has_value()) {
                throw std::invalid_argument("Повторний параметр --file.");
            }
            file_arg = std::string(argv[++i]);
        } else {
            throw std::invalid_argument(
                "Невідомий параметр командного рядка: '" + std::string(arg) + "'.");
        }
    }

    if (file_arg.has_value()) {
        if (size_arg.has_value() || seed_arg.has_value()) {
            throw std::invalid_argument(
                "Не можна одночасно використовувати --file та --size/--seed.");
        }
        result.config = parse_config_file(*file_arg);
        if (compact) {
            result.config.print_each_board = false;
        }
        return result;
    }

    if (size_arg.has_value()) {
        result.config.board_size = *size_arg;
        result.config.seed = seed_arg.value_or(generate_random_seed());
        result.config.print_each_board = !compact;
        return result;
    }

    out << "Введіть розмір поля n (" << Board::kMinSize << ".."
        << Board::kMaxSize << "): " << std::flush;
    std::string size_line;
    if (!std::getline(in, size_line)) {
        throw std::runtime_error(
            "Не вдалося зчитати розмір поля зі стандартного вводу (EOF).");
    }
    result.config.board_size = parse_board_size(size_line);

    if (seed_arg.has_value()) {
        result.config.seed = *seed_arg;
    } else {
        out << "Введіть seed (або натисніть Enter для випадкового): " << std::flush;
        std::string seed_line;
        if (std::getline(in, seed_line) && !trim(seed_line).empty()) {
            result.config.seed = parse_seed(seed_line);
        } else {
            result.config.seed = generate_random_seed();
        }
    }

    result.config.print_each_board = !compact;
    return result;
}
