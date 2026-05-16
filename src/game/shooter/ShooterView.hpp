#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

#include <optional>
#include <string>

namespace mc {

class ShooterWorld;

class ShooterView {
public:
    ShooterView();

    void setHudLine(const std::string& line);
    void draw(sf::RenderTarget& target, const ShooterWorld& world);

private:
    sf::View worldView_;
    sf::Font hudFont_{};
    std::optional<sf::Text> hud_;
};

} // namespace mc
