#pragma once

#include "Board.hpp"

#include <cstddef>
#include <cstdint>
#include <ostream>

struct GameConfig {
    std::size_t board_size{3};
    std::uint64_t seed{42};
    bool print_each_board{true};
};

class GameEngine {
public:
    explicit GameEngine(GameConfig config);

    GameStatus run(std::ostream& os) const;

private:
    GameConfig config_;
};
