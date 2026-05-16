#include "game/minesweeper/MinesweeperWorld.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace mc {

MinesweeperWorld::MinesweeperWorld() {
    loadBestTimes();
    reset();
}

void MinesweeperWorld::setDifficulty(Difficulty preset) {
    difficulty_ = preset;
    if (difficulty_ == Difficulty::Beginner) {
        cols_ = 9;
        rows_ = 9;
        mineCount_ = 10;
    } else if (difficulty_ == Difficulty::Intermediate) {
        cols_ = 16;
        rows_ = 16;
        mineCount_ = 40;
    } else {
        cols_ = 30;
        rows_ = 16;
        mineCount_ = 99;
    }
    reset();
}

void MinesweeperWorld::reset() {
    status_ = Status::Playing;
    flaggedCount_ = 0;
    revealedCount_ = 0;
    remainingSafe_ = rows_ * cols_ - mineCount_;
    firstClickPending_ = true;
    elapsedSeconds_ = 0.f;
    cells_.assign(static_cast<std::size_t>(rows_ * cols_), Cell{});
}

void MinesweeperWorld::fixedUpdate(float dt) {
    if (status_ == Status::Playing && !firstClickPending_) {
        elapsedSeconds_ += dt;
    }
}

const MinesweeperWorld::Cell& MinesweeperWorld::cell(int x, int y) const {
    static const Cell kEmpty{};
    if (!inBounds(x, y)) {
        return kEmpty;
    }
    return cells_[static_cast<std::size_t>(index(x, y))];
}

void MinesweeperWorld::placeMines(int safeX, int safeY) {
    std::vector<int> indices;
    indices.reserve(static_cast<std::size_t>(rows_ * cols_));
    for (int i = 0; i < rows_ * cols_; ++i) {
        indices.push_back(i);
    }
    indices.erase(std::remove_if(indices.begin(), indices.end(), [&](int idx) {
                      const int x = idx % cols_;
                      const int y = idx / cols_;
                      return std::abs(x - safeX) <= 1 && std::abs(y - safeY) <= 1;
                  }),
                  indices.end());

    std::mt19937 rng{std::random_device{}()};
    std::shuffle(indices.begin(), indices.end(), rng);
    const int count = std::min(mineCount_, static_cast<int>(indices.size()));
    for (int i = 0; i < count; ++i) {
        const int idx = indices[static_cast<std::size_t>(i)];
        cells_[static_cast<std::size_t>(idx)].isMine = true;
    }
    computeAdjacentCounts();
}

void MinesweeperWorld::computeAdjacentCounts() {
    for (int y = 0; y < rows_; ++y) {
        for (int x = 0; x < cols_; ++x) {
            Cell& c = cells_[static_cast<std::size_t>(index(x, y))];
            if (c.isMine) {
                c.adjMines = 0;
                continue;
            }
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }
                    const int nx = x + dx;
                    const int ny = y + dy;
                    if (!inBounds(nx, ny)) {
                        continue;
                    }
                    if (cells_[static_cast<std::size_t>(index(nx, ny))].isMine) {
                        ++count;
                    }
                }
            }
            c.adjMines = count;
        }
    }
}

void MinesweeperWorld::revealMineField() {
    for (auto& c : cells_) {
        if (c.isMine) {
            c.isRevealed = true;
        }
    }
}

void MinesweeperWorld::checkWinAfterReveal() {
    if (remainingSafe_ > 0 || status_ != Status::Playing) {
        return;
    }
    status_ = Status::Win;
    const int elapsed = static_cast<int>(std::round(elapsedSeconds_));
    int* best = nullptr;
    if (difficulty_ == Difficulty::Beginner) {
        best = &bestBeginner_;
    } else if (difficulty_ == Difficulty::Intermediate) {
        best = &bestIntermediate_;
    } else {
        best = &bestExpert_;
    }
    if (best && (*best == 0 || elapsed < *best)) {
        *best = elapsed;
        saveBestTimes();
    }
}

void MinesweeperWorld::reveal(int x, int y) {
    if (!inBounds(x, y) || status_ != Status::Playing) {
        return;
    }
    Cell& start = cells_[static_cast<std::size_t>(index(x, y))];
    if (start.isFlagged || start.isRevealed) {
        return;
    }
    if (firstClickPending_) {
        placeMines(x, y);
        firstClickPending_ = false;
    }
    if (start.isMine) {
        start.isRevealed = true;
        status_ = Status::Lose;
        revealMineField();
        return;
    }

    std::queue<std::pair<int, int>> q;
    q.push({x, y});
    while (!q.empty()) {
        const auto [cx, cy] = q.front();
        q.pop();
        if (!inBounds(cx, cy)) {
            continue;
        }
        Cell& c = cells_[static_cast<std::size_t>(index(cx, cy))];
        if (c.isRevealed || c.isFlagged || c.isMine) {
            continue;
        }
        c.isRevealed = true;
        ++revealedCount_;
        --remainingSafe_;
        if (c.adjMines != 0) {
            continue;
        }
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                const int nx = cx + dx;
                const int ny = cy + dy;
                if (!inBounds(nx, ny)) {
                    continue;
                }
                const Cell& n = cells_[static_cast<std::size_t>(index(nx, ny))];
                if (!n.isRevealed && !n.isMine && !n.isFlagged) {
                    q.push({nx, ny});
                }
            }
        }
    }
    checkWinAfterReveal();
}

void MinesweeperWorld::toggleFlag(int x, int y) {
    if (!inBounds(x, y) || status_ != Status::Playing) {
        return;
    }
    Cell& c = cells_[static_cast<std::size_t>(index(x, y))];
    if (c.isRevealed) {
        return;
    }
    c.isFlagged = !c.isFlagged;
    flaggedCount_ += c.isFlagged ? 1 : -1;
}

void MinesweeperWorld::chordReveal(int x, int y) {
    if (!inBounds(x, y) || status_ != Status::Playing) {
        return;
    }
    const Cell& center = cells_[static_cast<std::size_t>(index(x, y))];
    if (!center.isRevealed || center.adjMines <= 0) {
        return;
    }

    int flaggedNeighbors = 0;
    std::vector<std::pair<int, int>> hidden;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            const int nx = x + dx;
            const int ny = y + dy;
            if (!inBounds(nx, ny)) {
                continue;
            }
            const Cell& n = cells_[static_cast<std::size_t>(index(nx, ny))];
            if (n.isFlagged) {
                ++flaggedNeighbors;
            } else if (!n.isRevealed) {
                hidden.push_back({nx, ny});
            }
        }
    }
    if (flaggedNeighbors != center.adjMines) {
        return;
    }
    for (const auto& p : hidden) {
        reveal(p.first, p.second);
        if (status_ == Status::Lose) {
            return;
        }
    }
}

void MinesweeperWorld::loadBestTimes() {
    bestBeginner_ = bestIntermediate_ = bestExpert_ = 0;
    std::ifstream in("minesweeper_times.txt");
    if (!in.is_open()) {
        return;
    }
    std::string key;
    int value = 0;
    while (in >> key >> value) {
        if (key == "beginner") {
            bestBeginner_ = value;
        } else if (key == "intermediate") {
            bestIntermediate_ = value;
        } else if (key == "expert") {
            bestExpert_ = value;
        }
    }
}

void MinesweeperWorld::saveBestTimes() const {
    std::ofstream out("minesweeper_times.txt", std::ios::trunc);
    if (!out.is_open()) {
        return;
    }
    out << "beginner " << bestBeginner_ << "\n";
    out << "intermediate " << bestIntermediate_ << "\n";
    out << "expert " << bestExpert_ << "\n";
}

int MinesweeperWorld::bestTime(Difficulty d) const {
    if (d == Difficulty::Beginner) {
        return bestBeginner_;
    }
    if (d == Difficulty::Intermediate) {
        return bestIntermediate_;
    }
    return bestExpert_;
}

} // namespace mc
