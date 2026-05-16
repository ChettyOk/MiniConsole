#include "game/shooter/ShooterView.hpp"
#include "game/shooter/ShooterWorld.hpp"
#include "core/SystemFont.hpp"

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

ShooterView::ShooterView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_.emplace(hudFont_, "", 18u);
        hud_->setFillColor(sf::Color(220, 230, 240));
    }
}

void ShooterView::setHudLine(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

void ShooterView::draw(sf::RenderTarget& target, const ShooterWorld& world) {
    const sf::Vector2u win = target.getSize();
    const float worldW = world.Width;
    const float worldH = world.Height;
    const float winRatio = win.x / static_cast<float>(win.y);
    const float worldRatio = worldW / worldH;

    sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});
    if (winRatio > worldRatio) {
        const float width = worldRatio / winRatio;
        viewport.position.x = (1.f - width) * 0.5f;
        viewport.size.x = width;
    } else if (winRatio < worldRatio) {
        const float height = winRatio / worldRatio;
        viewport.position.y = (1.f - height) * 0.5f;
        viewport.size.y = height;
    }

    worldView_.setSize({worldW, worldH});
    worldView_.setCenter({worldW * 0.5f, worldH * 0.5f});
    worldView_.setViewport(viewport);
    target.setView(worldView_);

    sf::RectangleShape bg({worldW, worldH});
    bg.setFillColor(sf::Color(20, 22, 32));
    target.draw(bg);

    sf::CircleShape player(world.playerRadius());
    player.setOrigin({world.playerRadius(), world.playerRadius()});
    player.setPosition({world.playerPos().x, world.playerPos().y});
    if (world.invulnerabilityLeft() > 0.f) {
        const int blink = static_cast<int>(world.invulnerabilityLeft() * 12.f) % 2;
        player.setFillColor(blink ? sf::Color(70, 120, 160) : sf::Color(120, 220, 255));
    } else {
        player.setFillColor(sf::Color(120, 220, 255));
    }
    target.draw(player);

    for (const auto& b : world.bullets()) {
        if (!b.alive) {
            continue;
        }
        sf::RectangleShape r({4.f, 12.f});
        r.setOrigin({2.f, 12.f});
        r.setPosition({b.pos.x, b.pos.y});
        r.setFillColor(sf::Color(255, 230, 140));
        target.draw(r);
    }

    for (const auto& e : world.enemies()) {
        if (!e.alive) {
            continue;
        }
        sf::CircleShape c(e.radius);
        c.setOrigin({e.radius, e.radius});
        c.setPosition({e.pos.x, e.pos.y});
        c.setFillColor(sf::Color(255, 110, 110));
        c.setOutlineColor(sf::Color(160, 40, 40));
        c.setOutlineThickness(1.f);
        target.draw(c);
    }

    target.setView(target.getDefaultView());
    if (hud_) {
        sf::RectangleShape hudBg({static_cast<float>(target.getSize().x) - 20.f, 36.f});
        hudBg.setPosition({10.f, 8.f});
        hudBg.setFillColor(sf::Color(10, 12, 18, 175));
        hudBg.setOutlineColor(sf::Color(80, 94, 124, 150));
        hudBg.setOutlineThickness(1.f);
        target.draw(hudBg);

        hud_->setPosition({14.f, 10.f});
        fitTextToWidth(*hud_, static_cast<float>(target.getSize().x) - 36.f, 18u);
        target.draw(*hud_);
    }
}

} // namespace mc
