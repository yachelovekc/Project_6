#include "Board.hpp"

#include <iomanip>
#include <stdexcept>
#include <string>

char cell_to_char(Cell cell) noexcept {
    switch (cell) {
        case Cell::Empty:
            return '.';
        case Cell::Cross:
            return 'X';
        case Cell::Nought:
            return 'O';
    }
    return '?';
}

Board::Board(std::size_t size)
    : size_(size), grid_(size * size, Cell::Empty) {
    if (size < kMinSize || size > kMaxSize) {
        throw std::invalid_argument(
            "Розмір поля n має бути в межах [" +
            std::to_string(kMinSize) + ", " +
            std::to_string(kMaxSize) + "].");
    }
}

bool Board::is_inside(std::size_t row, std::size_t col) const noexcept {
    return row < size_ && col < size_;
}

Cell Board::at(std::size_t row, std::size_t col) const {
    if (!is_inside(row, col)) {
        throw std::out_of_range("Координати клітинки виходять за межі поля.");
    }
    return grid_[row * size_ + col];
}

bool Board::is_valid_move(std::size_t row, std::size_t col) const noexcept {
    return is_inside(row, col) && grid_[row * size_ + col] == Cell::Empty;
}

void Board::make_move(std::size_t row, std::size_t col, Cell mark) {
    if (mark == Cell::Empty) {
        throw std::invalid_argument("Неможливо зробити хід порожнім символом.");
    }
    if (!is_inside(row, col)) {
        throw std::out_of_range("Координати ходу виходять за межі поля.");
    }
    if (grid_[row * size_ + col] != Cell::Empty) {
        throw std::logic_error("Клітинка вже зайнята.");
    }
    grid_[row * size_ + col] = mark;
    ++moves_made_;
}

std::vector<Move> Board::available_moves() const {
    std::vector<Move> moves;
    moves.reserve(size_ * size_ - moves_made_);
    for (std::size_t r = 0; r < size_; ++r) {
        for (std::size_t c = 0; c < size_; ++c) {
            if (grid_[r * size_ + c] == Cell::Empty) {
                moves.push_back(Move{r, c});
            }
        }
    }
    return moves;
}

bool Board::is_full() const noexcept {
    return moves_made_ == size_ * size_;
}

bool Board::has_line(Cell mark) const noexcept {
    for (std::size_t i = 0; i < size_; ++i) {
        bool row_win = true;
        bool col_win = true;
        for (std::size_t j = 0; j < size_; ++j) {
            if (grid_[i * size_ + j] != mark) {
                row_win = false;
            }
            if (grid_[j * size_ + i] != mark) {
                col_win = false;
            }
        }
        if (row_win || col_win) {
            return true;
        }
    }

    bool main_diag = true;
    bool anti_diag = true;
    for (std::size_t i = 0; i < size_; ++i) {
        if (grid_[i * size_ + i] != mark) {
            main_diag = false;
        }
        if (grid_[i * size_ + (size_ - 1 - i)] != mark) {
            anti_diag = false;
        }
    }
    return main_diag || anti_diag;
}

GameStatus Board::status() const noexcept {
    if (has_line(Cell::Cross)) {
        return GameStatus::CrossWins;
    }
    if (has_line(Cell::Nought)) {
        return GameStatus::NoughtWins;
    }
    if (is_full()) {
        return GameStatus::Draw;
    }
    return GameStatus::InProgress;
}

void Board::print(std::ostream& os) const {
    const int col_width = (size_ >= 10) ? 3 : 2;

    os << "   ";
    for (std::size_t c = 0; c < size_; ++c) {
        os << std::setw(col_width) << (c + 1);
    }
    os << '\n';

    for (std::size_t r = 0; r < size_; ++r) {
        os << std::setw(2) << (r + 1) << ' ';
        for (std::size_t c = 0; c < size_; ++c) {
            os << std::setw(col_width) << cell_to_char(grid_[r * size_ + c]);
        }
        os << '\n';
    }
}
