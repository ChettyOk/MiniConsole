#include "game/minesweeper/MinesweeperView.hpp"

#include "core/SystemFont.hpp"
#include "game/minesweeper/MinesweeperWorld.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <array>
#include <string>

namespace mc {

namespace {

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 11u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

MinesweeperView::MinesweeperView() {
    if (tryLoadSystemFont(hudFont_)) {
        hud_.emplace(hudFont_, "", 18u);
        hud_->setFillColor(sf::Color(220, 230, 240));
    }
}

void MinesweeperView::setHudLine(const std::string& line) {
    if (hud_) {
        hud_->setString(line);
    }
}

sf::Color MinesweeperView::numberColor(int n) const {
    static constexpr std::array<sf::Color, 9> kColors = {
        sf::Color(0, 0, 0),
        sf::Color(60, 120, 255),
        sf::Color(50, 160, 80),
        sf::Color(230, 70, 70),
        sf::Color(30, 40, 160),
        sf::Color(130, 20, 20),
        sf::Color(60, 150, 150),
        sf::Color(40, 40, 40),
        sf::Color(110, 110, 110),
    };
    return kColors[static_cast<std::size_t>(std::max(0, std::min(8, n)))];
}

void MinesweeperView::draw(sf::RenderTarget& target,
                           const MinesweeperWorld& world,
                           float originX,
                           float originY,
                           float cellSize,
                           bool drawHover,
                           int hoverX,
                           int hoverY) {
    sf::RectangleShape bg({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    bg.setFillColor(sf::Color(20, 24, 34));
    target.draw(bg);

    sf::RectangleShape boardPlate({world.cols() * cellSize + 8.f, world.rows() * cellSize + 8.f});
    boardPlate.setPosition({originX - 4.f, originY - 4.f});
    boardPlate.setFillColor(sf::Color(10, 12, 18, 210));
    boardPlate.setOutlineColor(sf::Color(86, 98, 128, 160));
    boardPlate.setOutlineThickness(1.f);
    target.draw(boardPlate);

    for (int y = 0; y < world.rows(); ++y) {
        for (int x = 0; x < world.cols(); ++x) {
            const auto& c = world.cell(x, y);
            sf::RectangleShape tile({cellSize - 1.f, cellSize - 1.f});
            tile.setPosition({originX + x * cellSize, originY + y * cellSize});
            if (c.isRevealed) {
                tile.setFillColor(c.isMine ? sf::Color(170, 70, 70) : sf::Color(186, 192, 205));
            } else {
                tile.setFillColor(sf::Color(90, 100, 118));
            }
            target.draw(tile);

            if (drawHover && x == hoverX && y == hoverY && !c.isRevealed) {
                sf::RectangleShape hover({cellSize - 3.f, cellSize - 3.f});
                hover.setPosition({originX + x * cellSize + 1.f, originY + y * cellSize + 1.f});
                hover.setFillColor(sf::Color(255, 255, 255, 24));
                target.draw(hover);
            }

            if (c.isFlagged && !c.isRevealed) {
                sf::Text flag(hudFont_, "F", static_cast<unsigned int>(cellSize * 0.62f));
                flag.setFillColor(sf::Color(255, 90, 90));
                flag.setPosition({originX + x * cellSize + cellSize * 0.26f, originY + y * cellSize + cellSize * 0.08f});
                target.draw(flag);
            } else if (c.isRevealed && c.isMine) {
                sf::CircleShape mine(cellSize * 0.2f);
                mine.setOrigin({cellSize * 0.2f, cellSize * 0.2f});
                mine.setPosition({originX + x * cellSize + cellSize * 0.5f, originY + y * cellSize + cellSize * 0.5f});
                mine.setFillColor(sf::Color(32, 32, 36));
                target.draw(mine);
            } else if (c.isRevealed && c.adjMines > 0) {
                sf::Text n(hudFont_, std::to_string(c.adjMines), static_cast<unsigned int>(cellSize * 0.62f));
                n.setFillColor(numberColor(c.adjMines));
                n.setPosition({originX + x * cellSize + cellSize * 0.28f, originY + y * cellSize + cellSize * 0.08f});
                target.draw(n);
            }
        }
    }

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
