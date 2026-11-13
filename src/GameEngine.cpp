#include "GameEngine.hpp"
#include "Player.hpp"

#include <array>
#include <stdexcept>
#include <string_view>

namespace {

struct PlayerSlot {
    std::string_view name;
    Cell mark;
    PlayerTask task;
};

void print_outcome(std::ostream& os, GameStatus status, std::size_t moves) {
    os << "Усього зроблено ходів: " << moves << '\n';
    switch (status) {
        case GameStatus::CrossWins:
            os << "Переможець: Гравець 1 (X)\n";
            break;
        case GameStatus::NoughtWins:
            os << "Переможець: Гравець 2 (O)\n";
            break;
        case GameStatus::Draw:
            os << "Результат: Гра завершена вничию.\n";
            break;
        case GameStatus::InProgress:
            os << "Результат: Гру перервано до завершення.\n";
            break;
    }
}

}

GameEngine::GameEngine(GameConfig config) : config_(config) {
    if (config_.board_size < Board::kMinSize ||
        config_.board_size > Board::kMaxSize) {
        throw std::invalid_argument(
            "Неприпустимий розмір поля для запуску гри.");
    }
}

GameStatus GameEngine::run(std::ostream& os) const {
    Board board(config_.board_size);

    std::array<PlayerSlot, 2> players{{
        {"Гравець 1", Cell::Cross,
         make_player_coroutine(board, Cell::Cross, config_.seed)},
        {"Гравець 2", Cell::Nought,
         make_player_coroutine(board, Cell::Nought, config_.seed ^ 0x9E3779B97F4A7C15ULL)},
    }};

    os << "Гра 'Хрестики-нолики' (" << board.size() << "x" << board.size()
       << ", seed=" << config_.seed << ")\n";
    os << "Протокол гри:\n";

    std::size_t turn_index = 0;
    while (board.status() == GameStatus::InProgress) {
        PlayerSlot& current = players[turn_index % players.size()];
        if (!current.task.resume()) {
            break;
        }

        const auto move = current.task.last_move();
        if (!move.has_value() || !board.is_valid_move(move->row, move->col)) {
            throw std::runtime_error("Сопрограма повернула некоректний хід.");
        }

        board.make_move(move->row, move->col, current.mark);
        ++turn_index;

        os << "Хід " << turn_index << ": " << current.name
           << " (" << cell_to_char(current.mark) << ") -> ("
           << (move->row + 1) << ", " << (move->col + 1) << ")\n";

        if (config_.print_each_board) {
            board.print(os);
        }
    }

    const GameStatus final_status = board.status();
    if (!config_.print_each_board) {
        os << "\nФінальне поле:\n";
        board.print(os);
    }
    print_outcome(os, final_status, board.move_count());
    return final_status;
}
