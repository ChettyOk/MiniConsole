#pragma once

#include "core/GameState.hpp"

#include <SFML/Graphics/Font.hpp>

#include <string>

namespace mc {

class Application;

class StubGameState final : public GameState {
public:
    StubGameState(std::string title, std::string designNotes);

    void onEnter(Application& app) override;
    void handleInput(const sf::Event& event) override;
    void update(sf::Time fixedDt) override;
    void render(sf::RenderTarget& target) override;

private:
    std::string title_;
    std::string designNotes_;
    Application* app_{nullptr};
    sf::Font font_{};
    bool fontReady_{false};
};

} // namespace mc
