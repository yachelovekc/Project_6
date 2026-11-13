#pragma once

#include "Board.hpp"
#include "Coroutine.hpp"

#include <cstdint>

PlayerTask make_player_coroutine(const Board& board,
                                 Cell mark,
                                 std::uint64_t seed);
