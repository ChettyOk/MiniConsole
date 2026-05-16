#pragma once

#include "game/towerdefense/TowerDefenseWorld.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

#include <optional>
#include <string>

namespace mc {

class TowerDefenseView {
public:
    TowerDefenseView();

    void setHudLine(const std::string& line);
    void draw(sf::RenderTarget& target,
              const TowerDefenseWorld& world,
              TowerDefenseWorld::Cell cursorCell,
              bool showPathDebug);

private:
    sf::View worldView_;
    sf::Font hudFont_{};
    std::optional<sf::Text> hud_;
};

} // namespace mc
