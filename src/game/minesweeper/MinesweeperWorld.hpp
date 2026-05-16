#pragma once

#include <string>
#include <vector>

namespace mc {

class MinesweeperWorld {
public:
    enum class Difficulty { Beginner, Intermediate, Expert };
    enum class Status { Playing, Win, Lose };

    struct Cell {
        bool isMine = false;
        bool isRevealed = false;
        bool isFlagged = false;
        int adjMines = 0;
    };

    MinesweeperWorld();

    void reset();
    void setDifficulty(Difficulty preset);
    void fixedUpdate(float dt);

    void reveal(int x, int y);
    void toggleFlag(int x, int y);
    void chordReveal(int x, int y);

    Difficulty difficulty() const { return difficulty_; }
    Status status() const { return status_; }
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int mineCount() const { return mineCount_; }
    int flaggedCount() const { return flaggedCount_; }
    int revealedCount() const { return revealedCount_; }
    int remainingSafeCells() const { return remainingSafe_; }
    bool firstClickPending() const { return firstClickPending_; }
    float elapsedSeconds() const { return elapsedSeconds_; }
    int bestTime(Difficulty d) const;

    const Cell& cell(int x, int y) const;

private:
    int index(int x, int y) const { return y * cols_ + x; }
    bool inBounds(int x, int y) const { return x >= 0 && x < cols_ && y >= 0 && y < rows_; }
    void placeMines(int safeX, int safeY);
    void computeAdjacentCounts();
    void revealMineField();
    void checkWinAfterReveal();
    void loadBestTimes();
    void saveBestTimes() const;

    Difficulty difficulty_{Difficulty::Beginner};
    Status status_{Status::Playing};
    int rows_ = 9;
    int cols_ = 9;
    int mineCount_ = 10;
    int flaggedCount_ = 0;
    int revealedCount_ = 0;
    int remainingSafe_ = 0;
    bool firstClickPending_ = true;
    float elapsedSeconds_ = 0.f;
    int bestBeginner_ = 0;
    int bestIntermediate_ = 0;
    int bestExpert_ = 0;
    std::vector<Cell> cells_;
};

} // namespace mc
