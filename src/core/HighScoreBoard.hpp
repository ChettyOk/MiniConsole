#pragma once

#include <string>
#include <vector>

namespace mc {

class HighScoreBoard {
public:
    struct Entry {
        std::string game;
        int score = 0;
        std::string stamp;
    };

    explicit HighScoreBoard(std::string filePath);

    void load();
    void save() const;

    bool submit(const std::string& game, int score);
    int topScore() const;
    const std::vector<Entry>& entries() const { return entries_; }

private:
    static constexpr std::size_t MaxEntries = 5;
    static std::string nowStamp();

    std::string filePath_;
    std::vector<Entry> entries_;
};

} // namespace mc
