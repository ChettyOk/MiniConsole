#pragma once

#include "core/GameState.hpp"

#include "game/breakout/BreakoutView.hpp"
#include "game/breakout/BreakoutWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class BreakoutState final : public GameState {
public:
    BreakoutState() = default;

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshHud();

    BreakoutWorld world_;
    BreakoutView view_;
    Application* app_{nullptr};
    bool moveLeft_{false};
    bool moveRight_{false};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
