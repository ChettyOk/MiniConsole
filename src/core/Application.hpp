#pragma once

#include "core/GameClock.hpp"
#include "core/GameDifficulty.hpp"
#include "core/GameState.hpp"
#include "core/HighScoreBoard.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <optional>

namespace mc {

class Application {
public:
    Application();

    void run();
    void quit();

    void requestState(std::unique_ptr<GameState> nextState);
    HighScoreBoard& highScores() { return scores_; }
    const HighScoreBoard& highScores() const { return scores_; }
    void setDifficulty(GameDifficulty d) { difficulty_ = d; }
    void toggleDifficulty() { difficulty_ = toggledDifficulty(difficulty_); }
    GameDifficulty difficulty() const { return difficulty_; }

    sf::RenderWindow& window() { return window_; }
    const sf::RenderWindow& window() const { return window_; }

private:
    void applyPendingState();
    bool runFrame();

    sf::RenderWindow window_;
    GameClock fixedClock_;
    sf::Clock frameClock_;
    std::unique_ptr<GameState> state_;
    std::optional<std::unique_ptr<GameState>> pending_;
    HighScoreBoard scores_;
    GameDifficulty difficulty_{GameDifficulty::Normal};
    bool running_{true};
};

} // namespace mc
