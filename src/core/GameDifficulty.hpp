#pragma once

namespace mc {

enum class GameDifficulty { Normal, Hard };

inline const char* difficultyName(GameDifficulty d) {
    return d == GameDifficulty::Hard ? "Hard" : "Normal";
}

inline GameDifficulty toggledDifficulty(GameDifficulty d) {
    return d == GameDifficulty::Hard ? GameDifficulty::Normal : GameDifficulty::Hard;
}

} // namespace mc
