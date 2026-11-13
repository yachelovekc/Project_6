#pragma once

#include <cstddef>
#include <optional>
#include <ostream>
#include <vector>

enum class Cell {
    Empty,
    Cross,
    Nought
};

struct Move {
    std::size_t row{};
    std::size_t col{};
};

enum class GameStatus {
    InProgress,
    CrossWins,
    NoughtWins,
    Draw
};

char cell_to_char(Cell cell) noexcept;

class Board {
public:
    static constexpr std::size_t kMinSize = 1;
    static constexpr std::size_t kMaxSize = 25;

    explicit Board(std::size_t size);

    std::size_t size() const noexcept { return size_; }
    Cell at(std::size_t row, std::size_t col) const;

    bool is_inside(std::size_t row, std::size_t col) const noexcept;
    bool is_valid_move(std::size_t row, std::size_t col) const noexcept;

    void make_move(std::size_t row, std::size_t col, Cell mark);

    std::vector<Move> available_moves() const;
    GameStatus status() const noexcept;
    bool is_full() const noexcept;
    std::size_t move_count() const noexcept { return moves_made_; }

    void print(std::ostream& os) const;

private:
    bool has_line(Cell mark) const noexcept;

    std::size_t size_;
    std::size_t moves_made_{0};
    std::vector<Cell> grid_;
};
