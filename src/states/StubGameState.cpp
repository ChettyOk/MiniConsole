#include "states/StubGameState.hpp"

#include "core/Application.hpp"
#include "core/SfmlCompat.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>

namespace mc {

StubGameState::StubGameState(std::string title, std::string designNotes)
    : title_(std::move(title)), designNotes_(std::move(designNotes)) {}

void StubGameState::onEnter(Application& app) {
    app_ = &app;
    fontReady_ = tryLoadSystemFont(font_);
}

void StubGameState::handleInput(const sf::Event& event) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key || key->code != sf::Keyboard::Key::Escape) {
        return;
    }
    if (app_) {
        app_->requestState(std::make_unique<MenuState>());
    }
}

void StubGameState::update(sf::Time fixedDt) { (void)fixedDt; }

void StubGameState::render(sf::RenderTarget& target) {
    sf::RectangleShape backdrop;
    backdrop.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    backdrop.setFillColor(sf::Color(22, 24, 32));
    target.draw(backdrop);

    if (!fontReady_) {
        return;
    }

    sf::Text titleText = makeText(font_, title_, 32u);
    titleText.setFillColor(sf::Color(240, 240, 245));
    titleText.setPosition({40.f, 40.f});
    target.draw(titleText);

    sf::Text body = makeText(font_, designNotes_, 18u);
    body.setFillColor(sf::Color(200, 205, 220));
    body.setPosition({40.f, 110.f});
    target.draw(body);

    sf::Text hint = makeText(font_, "Press Esc to return to the menu.", 18u);
    hint.setFillColor(sf::Color(160, 170, 190));
    hint.setPosition({40.f, static_cast<float>(target.getSize().y) - 80.f});
    target.draw(hint);
}

} // namespace mc
