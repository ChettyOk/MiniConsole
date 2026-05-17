#include "game/platformer/PlatformerView.hpp"

#include "core/SfmlCompat.hpp"
#include "core/SystemFont.hpp"
#include "game/platformer/PlatformerWorld.hpp"

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

PlatformerView::PlatformerView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_ = makeText(hudFont_, "", 18u);
        hud_->setFillColor(sf::Color(225, 235, 245));
    }
}

void PlatformerView::setHudLine(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

void PlatformerView::draw(sf::RenderTarget& target, const PlatformerWorld& world) {
    worldView_.setSize({PlatformerWorld::Width, PlatformerWorld::Height});
    worldView_.setCenter({PlatformerWorld::Width * 0.5f, PlatformerWorld::Height * 0.5f});
    // Reserve top strip for HUD so gameplay entities are never hidden behind text.
    worldView_.setViewport(sf::FloatRect({0.f, 0.07f}, {1.f, 0.93f}));
    target.setView(worldView_);

    sf::RectangleShape bg({PlatformerWorld::Width, PlatformerWorld::Height});
    bg.setFillColor(sf::Color(24, 30, 44));
    target.draw(bg);

    for (int y = 0; y < PlatformerWorld::Rows; ++y) {
        for (int x = 0; x < PlatformerWorld::Cols; ++x) {
            const char t = world.tiles()[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const sf::Vector2f pos(x * PlatformerWorld::TileSize, y * PlatformerWorld::TileSize);
            if (t == '#') {
                sf::RectangleShape tile({static_cast<float>(PlatformerWorld::TileSize - 1),
                                         static_cast<float>(PlatformerWorld::TileSize - 1)});
                tile.setPosition(pos);
                tile.setFillColor(sf::Color(90, 105, 125));
                target.draw(tile);
            } else if (t == 'C') {
                sf::CircleShape coin(6.f);
                coin.setOrigin({6.f, 6.f});
                coin.setPosition({pos.x + PlatformerWorld::TileSize * 0.5f, pos.y + PlatformerWorld::TileSize * 0.5f});
                coin.setFillColor(sf::Color(245, 220, 90));
                target.draw(coin);
            } else if (t == 'G') {
                sf::RectangleShape goal({18.f, 26.f});
                goal.setPosition({pos.x + 7.f, pos.y + 3.f});
                goal.setFillColor(sf::Color(120, 240, 150));
                target.draw(goal);
            } else if (t == 'X') {
                sf::RectangleShape spike({24.f, 10.f});
                spike.setPosition({pos.x + 4.f, pos.y + 18.f});
                spike.setFillColor(sf::Color(255, 120, 120));
                target.draw(spike);
            }
        }
    }

    for (const auto& shot : world.hazardShots()) {
        if (!shot.alive) {
            continue;
        }
        sf::CircleShape orb(shot.radius);
        orb.setOrigin({shot.radius, shot.radius});
        orb.setPosition({shot.pos.x, shot.pos.y});
        orb.setFillColor(sf::Color(255, 130, 120));
        target.draw(orb);
    }

    const Vec2 p = world.playerPos();
    const Vec2 s = world.playerSize();
    sf::RectangleShape player({s.x, s.y});
    player.setPosition({p.x, p.y});
    player.setFillColor(sf::Color(130, 210, 255));
    target.draw(player);

    target.setView(target.getDefaultView());
    if (hud_) {
        sf::RectangleShape hudBg({static_cast<float>(target.getSize().x) - 20.f, 40.f});
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
