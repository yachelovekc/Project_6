#include "Player.hpp"

#include <random>
#include <stdexcept>

PlayerTask make_player_coroutine(const Board& board,
                                 Cell mark,
                                 std::uint64_t seed) {
    if (mark == Cell::Empty) {
        throw std::invalid_argument("Гравець не може мати порожній символ.");
    }

    std::mt19937_64 rng(seed);

    while (board.status() == GameStatus::InProgress) {
        const auto moves = board.available_moves();
        if (moves.empty()) {
            co_return;
        }
        std::uniform_int_distribution<std::size_t> dist(0, moves.size() - 1);
        const Move chosen = moves[dist(rng)];
        co_yield chosen;
    }
}
