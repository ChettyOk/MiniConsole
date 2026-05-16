#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

#include <optional>
#include <string>

namespace mc {

class PlatformerWorld;

class PlatformerView {
public:
    PlatformerView();

    void setHudLine(const std::string& line);
    void draw(sf::RenderTarget& target, const PlatformerWorld& world);

private:
    sf::View worldView_;
    sf::Font hudFont_{};
    std::optional<sf::Text> hud_;
};

} // namespace mc
