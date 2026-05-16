#pragma once

#include "core/GameState.hpp"
#include "game/towerdefense/TowerDefenseView.hpp"
#include "game/towerdefense/TowerDefenseWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class TowerDefenseState final : public GameState {
public:
    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void moveCursor(int dx, int dy);
    void refreshHud();
    void handlePauseInput(sf::Keyboard::Key key);

    TowerDefenseWorld world_;
    TowerDefenseView view_;
    TowerDefenseWorld::Cell cursor_{7, 10};
    TowerDefenseWorld::TowerType buildMode_{TowerDefenseWorld::TowerType::Cannon};
    Application* app_{nullptr};
    bool paused_{false};
    int pauseSelection_{0};
    bool debugPath_{false};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
