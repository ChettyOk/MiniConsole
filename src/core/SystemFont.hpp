#pragma once

#include <SFML/Graphics/Font.hpp>

#include <array>

namespace mc {

inline bool tryLoadSystemFont(sf::Font& font) {
    const std::array<const char*, 5> candidates = {{
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
    }};
    for (const char* path : candidates) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}

} // namespace mc
