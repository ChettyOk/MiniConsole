#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <optional>
#include <string>

namespace mc {

class MinesweeperWorld;

class MinesweeperView {
public:
    MinesweeperView();

    void setHudLine(const std::string& line);
    void draw(sf::RenderTarget& target,
              const MinesweeperWorld& world,
              float originX,
              float originY,
              float cellSize,
              bool drawHover,
              int hoverX,
              int hoverY);

private:
    sf::Color numberColor(int n) const;

    sf::Font hudFont_{};
    std::optional<sf::Text> hud_;
};

} // namespace mc
