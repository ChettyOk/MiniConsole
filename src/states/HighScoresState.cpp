#include "states/HighScoresState.hpp"

#include "core/Application.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>
#include <sstream>

namespace mc {

namespace {

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 12u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

void HighScoresState::onEnter(Application& app) {
    app_ = &app;
    fontReady_ = tryLoadSystemFont(font_);
}

void HighScoresState::handleInput(const sf::Event& event) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key || !app_) {
        return;
    }
    if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Enter ||
        key->code == sf::Keyboard::Key::Space) {
        app_->requestState(std::make_unique<MenuState>());
    }
}

void HighScoresState::update(sf::Time fixedDt) { (void)fixedDt; }

void HighScoresState::render(sf::RenderTarget& target) {
    const float w = static_cast<float>(target.getSize().x);
    const float h = static_cast<float>(target.getSize().y);
    sf::RectangleShape backdrop;
    backdrop.setSize({w, h});
    backdrop.setFillColor(sf::Color(14, 17, 24));
    target.draw(backdrop);

    if (!fontReady_) {
        return;
    }

    sf::Text title(font_, "Top 5 High Scores", 40u);
    title.setFillColor(sf::Color(240, 245, 250));
    title.setPosition({52.f, 32.f});
    fitTextToWidth(title, w - 100.f, 40u, 24u);
    target.draw(title);

    float y = 120.f;
    if (app_->highScores().entries().empty()) {
        sf::Text empty(font_, "No high scores yet. Finish a run to create one.", 22u);
        empty.setFillColor(sf::Color(170, 180, 195));
        empty.setPosition({60.f, y});
        target.draw(empty);
    } else {
        int idx = 1;
        for (const auto& e : app_->highScores().entries()) {
            std::ostringstream os;
            os << idx << ". " << e.score << "  -  " << e.game << "  (" << e.stamp << ")";
            sf::Text line(font_, os.str(), 24u);
            line.setFillColor(idx == 1 ? sf::Color(255, 220, 120) : sf::Color(210, 215, 225));
            line.setPosition({60.f, y});
            fitTextToWidth(line, w - 120.f, 24u, 14u);
            target.draw(line);
            y += 46.f;
            ++idx;
        }
    }

    sf::Text hint(font_, "Press Enter, Space, or Esc to return to menu.", 18u);
    hint.setFillColor(sf::Color(140, 150, 168));
    hint.setPosition({52.f, h - 62.f});
    fitTextToWidth(hint, w - 100.f, 18u, 12u);
    target.draw(hint);
}

} // namespace mc
