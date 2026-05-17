#include "game/towerdefense/TowerDefenseView.hpp"

#include "core/SfmlCompat.hpp"
#include "core/SystemFont.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <string>

namespace mc {

namespace {

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 10u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

TowerDefenseView::TowerDefenseView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_ = makeText(hudFont_, "", 15u);
        hud_->setFillColor(sf::Color(220, 230, 240));
    }
}

void TowerDefenseView::setHudLine(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

void TowerDefenseView::draw(sf::RenderTarget& target,
                            const TowerDefenseWorld& world,
                            TowerDefenseWorld::Cell cursorCell,
                            bool showPathDebug) {
    worldView_.setSize({TowerDefenseWorld::Width, TowerDefenseWorld::Height});
    worldView_.setCenter({TowerDefenseWorld::Width * 0.5f, TowerDefenseWorld::Height * 0.5f});
    worldView_.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    target.setView(worldView_);

    sf::RectangleShape bg({TowerDefenseWorld::Width, TowerDefenseWorld::Height});
    bg.setFillColor(sf::Color(18, 22, 30));
    target.draw(bg);

    for (int y = 0; y < TowerDefenseWorld::Rows; ++y) {
        for (int x = 0; x < TowerDefenseWorld::Cols; ++x) {
            sf::RectangleShape cell({TowerDefenseWorld::TileSize - 1.f, TowerDefenseWorld::TileSize - 1.f});
            cell.setPosition({x * static_cast<float>(TowerDefenseWorld::TileSize),
                              y * static_cast<float>(TowerDefenseWorld::TileSize)});
            cell.setFillColor(sf::Color(40, 50, 63));
            target.draw(cell);
        }
    }

    for (const auto& c : world.path()) {
        sf::RectangleShape p({TowerDefenseWorld::TileSize - 2.f, TowerDefenseWorld::TileSize - 2.f});
        p.setPosition({c.x * static_cast<float>(TowerDefenseWorld::TileSize) + 1.f,
                       c.y * static_cast<float>(TowerDefenseWorld::TileSize) + 1.f});
        p.setFillColor(sf::Color(90, 95, 65));
        target.draw(p);
    }

    if (showPathDebug) {
        for (const auto& c : world.debugClosed()) {
            sf::RectangleShape box({10.f, 10.f});
            box.setOrigin({5.f, 5.f});
            box.setPosition({c.x * TowerDefenseWorld::TileSize + 7.f, c.y * TowerDefenseWorld::TileSize + 7.f});
            box.setFillColor(sf::Color(230, 80, 80));
            target.draw(box);
        }
        for (const auto& c : world.debugOpen()) {
            sf::RectangleShape box({10.f, 10.f});
            box.setOrigin({5.f, 5.f});
            box.setPosition({c.x * TowerDefenseWorld::TileSize + 24.f, c.y * TowerDefenseWorld::TileSize + 7.f});
            box.setFillColor(sf::Color(90, 140, 255));
            target.draw(box);
        }
    }

    for (const auto& t : world.towers()) {
        sf::RectangleShape box({22.f, 22.f});
        box.setOrigin({11.f, 11.f});
        box.setPosition({t.cell.x * TowerDefenseWorld::TileSize + TowerDefenseWorld::TileSize * 0.5f,
                         t.cell.y * TowerDefenseWorld::TileSize + TowerDefenseWorld::TileSize * 0.5f});
        if (t.type == TowerDefenseWorld::TowerType::Frost) {
            box.setFillColor(sf::Color(120, 220, 255));
        } else if (t.type == TowerDefenseWorld::TowerType::Ember) {
            box.setFillColor(sf::Color(255, 150, 95));
        } else if (t.level == 1) {
            box.setFillColor(sf::Color(100, 170, 255));
        } else if (t.level == 2) {
            box.setFillColor(sf::Color(130, 210, 255));
        } else {
            box.setFillColor(sf::Color(160, 245, 255));
        }
        target.draw(box);
    }

    for (const auto& p : world.projectiles()) {
        if (!p.alive) {
            continue;
        }
        sf::CircleShape bullet(p.radius);
        bullet.setOrigin({p.radius, p.radius});
        bullet.setPosition({p.pos.x, p.pos.y});
        if (p.sourceType == TowerDefenseWorld::TowerType::Frost) {
            bullet.setFillColor(sf::Color(110, 185, 255));
        } else if (p.sourceType == TowerDefenseWorld::TowerType::Ember) {
            bullet.setFillColor(sf::Color(255, 95, 90));
        } else {
            bullet.setFillColor(sf::Color(240, 240, 250));
        }
        target.draw(bullet);
    }

    for (const auto& e : world.enemies()) {
        sf::CircleShape body(10.f);
        body.setOrigin({10.f, 10.f});
        body.setPosition({e.pos.x, e.pos.y});
        body.setFillColor(sf::Color(255, 110, 110));
        target.draw(body);

        const float hpRatio = (e.maxHp > 0.f) ? (e.hp / e.maxHp) : 0.f;
        sf::RectangleShape hpBack({20.f, 4.f});
        hpBack.setPosition({e.pos.x - 10.f, e.pos.y - 17.f});
        hpBack.setFillColor(sf::Color(30, 30, 30));
        target.draw(hpBack);
        sf::RectangleShape hpFill({20.f * hpRatio, 4.f});
        hpFill.setPosition({e.pos.x - 10.f, e.pos.y - 17.f});
        hpFill.setFillColor(sf::Color(90, 240, 120));
        target.draw(hpFill);
    }

    sf::RectangleShape cursor({TowerDefenseWorld::TileSize - 3.f, TowerDefenseWorld::TileSize - 3.f});
    cursor.setPosition({cursorCell.x * static_cast<float>(TowerDefenseWorld::TileSize) + 1.5f,
                        cursorCell.y * static_cast<float>(TowerDefenseWorld::TileSize) + 1.5f});
    cursor.setFillColor(sf::Color(0, 0, 0, 0));
    cursor.setOutlineThickness(2.f);
    cursor.setOutlineColor(sf::Color(255, 220, 120));
    target.draw(cursor);

    target.setView(target.getDefaultView());
    if (hud_) {
        sf::RectangleShape hudTop({static_cast<float>(target.getSize().x) - 16.f, 34.f});
        hudTop.setPosition({8.f, 6.f});
        hudTop.setFillColor(sf::Color(8, 10, 14, 185));
        hudTop.setOutlineColor(sf::Color(82, 96, 124, 145));
        hudTop.setOutlineThickness(1.f);
        target.draw(hudTop);

        hud_->setPosition({10.f, 8.f});
        fitTextToWidth(*hud_, static_cast<float>(target.getSize().x) - 28.f, 15u);
        target.draw(*hud_);

        sf::RectangleShape panel({static_cast<float>(target.getSize().x) - 16.f, 74.f});
        panel.setPosition({8.f, static_cast<float>(target.getSize().y) - 82.f});
        panel.setFillColor(sf::Color(8, 10, 14, 185));
        target.draw(panel);

        sf::Text controls1 = makeText(hudFont_,
                                      "Move Arrows/WASD | Build B (" + std::to_string(world.buildCost()) +
                                          "g) | Type 1:C 2:F 3:E | Upgrade U",
                                      12u);
        controls1.setFillColor(sf::Color(210, 218, 230));
        controls1.setPosition({14.f, static_cast<float>(target.getSize().y) - 76.f});
        fitTextToWidth(controls1, static_cast<float>(target.getSize().x) - 32.f, 12u);
        target.draw(controls1);

        sf::Text controls2 = makeText(hudFont_, "Target mode T | Pause P | Restart R | Menu Esc | Debug Tab", 12u);
        controls2.setFillColor(sf::Color(180, 190, 205));
        controls2.setPosition({14.f, static_cast<float>(target.getSize().y) - 56.f});
        fitTextToWidth(controls2, static_cast<float>(target.getSize().x) - 32.f, 12u);
        target.draw(controls2);

        sf::Text controls3 = makeText(hudFont_, "C=Cannon F=Frost E=Ember", 12u);
        controls3.setFillColor(sf::Color(160, 170, 185));
        controls3.setPosition({14.f, static_cast<float>(target.getSize().y) - 36.f});
        fitTextToWidth(controls3, static_cast<float>(target.getSize().x) - 32.f, 12u);
        target.draw(controls3);
    }
}

} // namespace mc
