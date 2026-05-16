#pragma once

#include "core/GameState.hpp"

#include <SFML/Graphics/Font.hpp>

namespace mc {

class Application;

class HighScoresState final : public GameState {
public:
    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    Application* app_{nullptr};
    sf::Font font_{};
    bool fontReady_{false};
};

} // namespace mc
