#pragma once

#include "core/GameState.hpp"

#include "game/shooter/ShooterView.hpp"
#include "game/shooter/ShooterWorld.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class ShooterState final : public GameState {
public:
    ShooterState() = default;

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    void refreshHud();
    void recomputeMoveInput();

    ShooterWorld world_;
    ShooterView view_;
    Application* app_{nullptr};

    bool up_{false};
    bool down_{false};
    bool left_{false};
    bool right_{false};
    bool fire_{false};
    bool scoreSubmitted_{false};
    float endFlashTime_{0.f};
    sf::Font overlayFont_{};
    bool overlayFontReady_{false};
};

} // namespace mc
