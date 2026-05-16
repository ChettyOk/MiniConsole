#include "core/HighScoreBoard.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace mc {

namespace {

bool betterScore(const HighScoreBoard::Entry& a, const HighScoreBoard::Entry& b) {
    if (a.score != b.score) {
        return a.score > b.score;
    }
    return a.stamp > b.stamp;
}

} // namespace

HighScoreBoard::HighScoreBoard(std::string filePath) : filePath_(std::move(filePath)) {}

void HighScoreBoard::load() {
    entries_.clear();
    std::ifstream in(filePath_);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        const std::size_t p1 = line.find('|');
        const std::size_t p2 = (p1 == std::string::npos) ? std::string::npos : line.find('|', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) {
            continue;
        }

        Entry e;
        e.game = line.substr(0, p1);
        try {
            e.score = std::stoi(line.substr(p1 + 1, p2 - (p1 + 1)));
        } catch (...) {
            continue;
        }
        e.stamp = line.substr(p2 + 1);
        entries_.push_back(e);
    }

    std::sort(entries_.begin(), entries_.end(), betterScore);
    if (entries_.size() > MaxEntries) {
        entries_.resize(MaxEntries);
    }
}

void HighScoreBoard::save() const {
    std::ofstream out(filePath_, std::ios::trunc);
    if (!out.is_open()) {
        return;
    }
    for (const auto& e : entries_) {
        out << e.game << "|" << e.score << "|" << e.stamp << '\n';
    }
}

bool HighScoreBoard::submit(const std::string& game, int score) {
    if (score <= 0) {
        return false;
    }
    if (entries_.size() >= MaxEntries && score <= entries_.back().score) {
        return false;
    }

    entries_.push_back(Entry{game, score, nowStamp()});
    std::sort(entries_.begin(), entries_.end(), betterScore);
    if (entries_.size() > MaxEntries) {
        entries_.resize(MaxEntries);
    }
    save();
    return true;
}

int HighScoreBoard::topScore() const {
    if (entries_.empty()) {
        return 0;
    }
    return entries_.front().score;
}

std::string HighScoreBoard::nowStamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return os.str();
}

} // namespace mc
