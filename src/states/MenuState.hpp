#pragma once

#include "core/GameDifficulty.hpp"
#include "core/GameState.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class MenuState final : public GameState {
public:
    MenuState() = default;

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void launchSelection();
    void beginDifficultySelection(int gameSelection);
    void beginMinesweeperSelection();
    void launchGameForSelection(int gameSelection);

    Application* app_{nullptr};
    int selection_{0};
    bool selectingDifficulty_{false};
    bool selectingMinesweeperPreset_{false};
    int pendingGameSelection_{-1};
    GameDifficulty pendingDifficulty_{GameDifficulty::Normal};
    int pendingMinesweeperPreset_{0};
    sf::Font font_{};
    bool fontReady_{false};
};

} // namespace mc
