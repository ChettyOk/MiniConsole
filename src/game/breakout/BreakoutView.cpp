#include "game/breakout/BreakoutView.hpp"
#include "game/breakout/BreakoutWorld.hpp"
#include "core/SfmlCompat.hpp"
#include "core/SystemFont.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>

namespace mc {

namespace {

sf::Color brickColorForRow(int row) {
    // Top to bottom: red, orange, yellow, green, blue.
    static const sf::Color palette[] = {
        sf::Color(225, 80, 80),
        sf::Color(240, 150, 70),
        sf::Color(235, 215, 90),
        sf::Color(90, 205, 120),
        sf::Color(95, 150, 240),
    };
    const int idx = std::max(0, std::min(4, row));
    return palette[idx];
}

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 11u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

BreakoutView::BreakoutView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_ = makeText(hudFont_, "", 18u);
        hud_->setFillColor(sf::Color(220, 220, 230));
    }
}

void BreakoutView::setStatusText(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

void BreakoutView::draw(sf::RenderTarget& target, const BreakoutWorld& world) {
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

    sf::RectangleShape background({world.Width, world.Height});
    background.setFillColor(sf::Color(26, 28, 36));
    target.draw(background);

    for (const auto& brick : world.bricks()) {
        if (!brick.alive) {
            continue;
        }
        sf::RectangleShape rect({brick.w - 2.f, brick.h - 2.f});
        rect.setPosition({brick.x + 1.f, brick.y + 1.f});
        const sf::Color fill = brickColorForRow(brick.row);
        rect.setFillColor(fill);
        rect.setOutlineColor(sf::Color(fill.r / 2, fill.g / 2, fill.b / 2));
        rect.setOutlineThickness(1.f);
        target.draw(rect);
    }

    const Vec2 paddle = world.paddleCenter();
    const Vec2 ph = world.paddleHalfExtents();
    sf::RectangleShape paddleShape({ph.x * 2.f, ph.y * 2.f});
    paddleShape.setOrigin({ph.x, ph.y});
    paddleShape.setPosition({paddle.x, paddle.y});
    paddleShape.setFillColor(sf::Color(240, 240, 245));
    target.draw(paddleShape);

    sf::CircleShape ball(world.ballRadius());
    ball.setOrigin({world.ballRadius(), world.ballRadius()});
    ball.setPosition({world.ballCenter().x, world.ballCenter().y});
    ball.setFillColor(sf::Color(255, 210, 120));
    target.draw(ball);

    for (const auto& p : world.powerUps()) {
        if (!p.alive) {
            continue;
        }
        sf::RectangleShape token({16.f, 16.f});
        token.setOrigin({8.f, 8.f});
        token.setPosition({p.pos.x, p.pos.y});
        if (p.type == BreakoutWorld::PowerUpType::WidePaddle) {
            token.setFillColor(sf::Color(150, 120, 255));
        } else {
            token.setFillColor(sf::Color(120, 245, 230));
        }
        target.draw(token);
    }

    target.setView(target.getDefaultView());
    if (hud_) {
        sf::RectangleShape hudBg({static_cast<float>(target.getSize().x) - 20.f, 36.f});
        hudBg.setPosition({10.f, 8.f});
        hudBg.setFillColor(sf::Color(10, 12, 18, 175));
        hudBg.setOutlineColor(sf::Color(80, 94, 124, 150));
        hudBg.setOutlineThickness(1.f);
        target.draw(hudBg);

        hud_->setPosition({16.f, 12.f});
        fitTextToWidth(*hud_, static_cast<float>(target.getSize().x) - 36.f, 18u);
        target.draw(*hud_);
    }
}

} // namespace mc
