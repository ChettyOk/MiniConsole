#pragma once

#include "core/GameState.hpp"
#include "game/pacman/PacmanView.hpp"
#include "game/pacman/PacmanWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class PacmanState final : public GameState {
public:
    PacmanState() = default;

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshHud();
    void updateAttractMode(float dt);

    PacmanWorld world_;
    PacmanView view_;
    Application* app_{nullptr};
    bool inAttractMode_{true};
    float attractTime_{0.f};
    float attractBlinkTime_{0.f};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
