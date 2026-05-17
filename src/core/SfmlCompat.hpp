#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <string>
#include <utility>

namespace mc {

#if __has_include(<SFML/Base/Optional.hpp>)
#define MC_VRSFML_COMPAT 1
#else
#define MC_VRSFML_COMPAT 0
#endif

inline bool loadFontFromFile(sf::Font& font, const char* path) {
#if MC_VRSFML_COMPAT
    const auto loaded = sf::Font::openFromFile(path);
    if (!loaded) {
        return false;
    }
    font = std::move(*loaded);
    return true;
#else
    return font.openFromFile(path);
#endif
}

inline sf::Text makeText(const sf::Font& font, const sf::String& string, unsigned int characterSize) {
#if MC_VRSFML_COMPAT
    sf::Text::Data data{};
    data.string = string;
    data.characterSize = characterSize;
    return sf::Text(font, data);
#else
    return sf::Text(font, string, characterSize);
#endif
}

inline sf::Text makeText(const sf::Font& font, const std::string& string, unsigned int characterSize) {
    return makeText(font, sf::String(string), characterSize);
}

inline sf::Text makeText(const sf::Font& font, const char* string, unsigned int characterSize) {
    return makeText(font, sf::String(string), characterSize);
}

} // namespace mc
