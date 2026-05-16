#pragma once

#include "core/GameState.hpp"
#include "game/platformer/PlatformerView.hpp"
#include "game/platformer/PlatformerWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class PlatformerState final : public GameState {
public:
    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshHud();
    void handlePauseInput(sf::Keyboard::Key key);

    PlatformerWorld world_;
    PlatformerView view_;
    Application* app_{nullptr};

    bool left_{false};
    bool right_{false};
    bool paused_{false};
    int pauseSelection_{0};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
