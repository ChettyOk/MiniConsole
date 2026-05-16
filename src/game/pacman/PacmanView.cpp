#include "game/pacman/PacmanView.hpp"

#include "core/SystemFont.hpp"
#include "game/pacman/PacmanWorld.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace mc {

namespace {

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 11u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

PacmanView::PacmanView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_.emplace(hudFont_, "", 16u);
        hud_->setFillColor(sf::Color(220, 230, 240));
    }
}

void PacmanView::setHudLine(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

void PacmanView::draw(sf::RenderTarget& target, const PacmanWorld& world) {
    worldView_.setSize({PacmanWorld::Width, PacmanWorld::Height});
    worldView_.setCenter({PacmanWorld::Width * 0.5f, PacmanWorld::Height * 0.5f});
    worldView_.setViewport(sf::FloatRect({0.15f, 0.11f}, {0.7f, 0.78f}));

    sf::RectangleShape frame({static_cast<float>(target.getSize().x) * 0.74f, static_cast<float>(target.getSize().y) * 0.82f});
    frame.setPosition({static_cast<float>(target.getSize().x) * 0.13f, static_cast<float>(target.getSize().y) * 0.09f});
    frame.setFillColor(sf::Color(9, 11, 18, 205));
    frame.setOutlineColor(sf::Color(70, 88, 132, 175));
    frame.setOutlineThickness(1.f);
    target.draw(frame);

    target.setView(worldView_);

    sf::RectangleShape bg({PacmanWorld::Width, PacmanWorld::Height});
    bg.setFillColor(sf::Color(10, 10, 20));
    target.draw(bg);

    for (int y = 0; y < PacmanWorld::Rows; ++y) {
        for (int x = 0; x < PacmanWorld::Cols; ++x) {
            const char c = world.grid()[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const sf::Vector2f p(x * static_cast<float>(PacmanWorld::TileSize),
                                 y * static_cast<float>(PacmanWorld::TileSize));
            if (c == '#') {
                sf::RectangleShape wall(
                    {static_cast<float>(PacmanWorld::TileSize - 1), static_cast<float>(PacmanWorld::TileSize - 1)});
                wall.setPosition(p);
                wall.setFillColor(sf::Color(30, 65, 210));
                target.draw(wall);
            } else if (c == '.' || c == 'o') {
                const float r = (c == 'o') ? 5.f : 2.8f;
                sf::CircleShape pellet(r);
                pellet.setOrigin({r, r});
                pellet.setPosition({p.x + PacmanWorld::TileSize * 0.5f, p.y + PacmanWorld::TileSize * 0.5f});
                pellet.setFillColor(sf::Color(250, 230, 150));
                target.draw(pellet);
            }
        }
    }

    sf::CircleShape pac(12.f);
    pac.setOrigin({12.f, 12.f});
    pac.setPosition({world.pacman().pos.x, world.pacman().pos.y});
    pac.setFillColor(sf::Color(255, 230, 70));
    target.draw(pac);

    for (const auto& g : world.ghosts()) {
        sf::CircleShape body(11.f);
        body.setOrigin({11.f, 11.f});
        body.setPosition({g.pos.x, g.pos.y});
        if (g.mode == PacmanWorld::GhostMode::Frightened) {
            body.setFillColor(sf::Color(70, 90, 255));
        } else if (g.type == PacmanWorld::GhostType::Blinky) {
            body.setFillColor(sf::Color(255, 80, 80));
        } else if (g.type == PacmanWorld::GhostType::Pinky) {
            body.setFillColor(sf::Color(255, 120, 210));
        } else if (g.type == PacmanWorld::GhostType::Inky) {
            body.setFillColor(sf::Color(100, 240, 255));
        } else {
            body.setFillColor(sf::Color(255, 180, 80));
        }
        target.draw(body);
    }

    target.setView(target.getDefaultView());
    if (hud_) {
        sf::RectangleShape hudBg({static_cast<float>(target.getSize().x) - 20.f, 34.f});
        hudBg.setPosition({10.f, 8.f});
        hudBg.setFillColor(sf::Color(10, 12, 18, 175));
        hudBg.setOutlineColor(sf::Color(80, 94, 124, 150));
        hudBg.setOutlineThickness(1.f);
        target.draw(hudBg);

        hud_->setPosition({10.f, 10.f});
        fitTextToWidth(*hud_, static_cast<float>(target.getSize().x) - 34.f, 16u);
        target.draw(*hud_);
    }
}

} // namespace mc
