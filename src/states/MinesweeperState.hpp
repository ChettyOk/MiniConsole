#pragma once

#include "core/GameState.hpp"
#include "game/minesweeper/MinesweeperView.hpp"
#include "game/minesweeper/MinesweeperWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class MinesweeperState final : public GameState {
public:
    explicit MinesweeperState(MinesweeperWorld::Difficulty preset = MinesweeperWorld::Difficulty::Beginner)
        : preset_(preset) {}

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshHud();
    void recomputeBoardGeometry(const sf::RenderTarget& target);
    bool mouseToCell(sf::Vector2i pixel, int& outX, int& outY) const;
    static const char* difficultyName(MinesweeperWorld::Difficulty d);

    MinesweeperWorld world_;
    MinesweeperView view_;
    MinesweeperWorld::Difficulty preset_{MinesweeperWorld::Difficulty::Beginner};
    Application* app_{nullptr};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    float boardOriginX_{80.f};
    float boardOriginY_{120.f};
    float cellSize_{32.f};
    int hoverX_{-1};
    int hoverY_{-1};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
