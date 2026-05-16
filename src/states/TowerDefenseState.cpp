#include "states/TowerDefenseState.hpp"

#include "core/Application.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

namespace mc {

void TowerDefenseState::onEnter(Application& app) {
    app_ = &app;
    world_.setDifficulty(app.difficulty());
    world_.reset();
    cursor_ = {7, 10};
    buildMode_ = TowerDefenseWorld::TowerType::Cannon;
    paused_ = false;
    pauseSelection_ = 0;
    debugPath_ = false;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    refreshHud();
}

void TowerDefenseState::handlePauseInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape) {
        paused_ = false;
        return;
    }
    if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W) {
        pauseSelection_ = (pauseSelection_ + 2) % 3;
        return;
    }
    if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S) {
        pauseSelection_ = (pauseSelection_ + 1) % 3;
        return;
    }
    if (key != sf::Keyboard::Key::Enter && key != sf::Keyboard::Key::Space) {
        return;
    }
    if (pauseSelection_ == 0) {
        paused_ = false;
    } else if (pauseSelection_ == 1) {
        world_.setDifficulty(app_->difficulty());
        world_.reset();
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
        paused_ = false;
    } else if (pauseSelection_ == 2 && app_) {
        app_->requestState(std::make_unique<MenuState>());
    }
}

void TowerDefenseState::moveCursor(int dx, int dy) {
    cursor_.x = std::max(0, std::min(TowerDefenseWorld::Cols - 1, cursor_.x + dx));
    cursor_.y = std::max(0, std::min(TowerDefenseWorld::Rows - 1, cursor_.y + dy));
}

void TowerDefenseState::handleInput(const sf::Event& event) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) {
        return;
    }

    if (paused_) {
        handlePauseInput(key->code);
        return;
    }
    if (key->code == sf::Keyboard::Key::P) {
        paused_ = true;
        pauseSelection_ = 0;
        return;
    }
    if (key->code == sf::Keyboard::Key::Escape && app_) {
        app_->requestState(std::make_unique<MenuState>());
        return;
    }
    if (key->code == sf::Keyboard::Key::R) {
        world_.setDifficulty(app_->difficulty());
        world_.reset();
        buildMode_ = TowerDefenseWorld::TowerType::Cannon;
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
        return;
    }
    if (key->code == sf::Keyboard::Key::Tab) {
        debugPath_ = !debugPath_;
        return;
    }
    if (key->code == sf::Keyboard::Key::Num1) {
        buildMode_ = TowerDefenseWorld::TowerType::Cannon;
    } else if (key->code == sf::Keyboard::Key::Num2) {
        buildMode_ = TowerDefenseWorld::TowerType::Frost;
    } else if (key->code == sf::Keyboard::Key::Num3) {
        buildMode_ = TowerDefenseWorld::TowerType::Ember;
    } else if (key->code == sf::Keyboard::Key::T) {
        world_.cycleTowerTargeting(cursor_);
    } else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) {
        moveCursor(-1, 0);
    } else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
        moveCursor(1, 0);
    } else if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
        moveCursor(0, -1);
    } else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
        moveCursor(0, 1);
    } else if (key->code == sf::Keyboard::Key::B) {
        world_.placeTower(cursor_, buildMode_);
    } else if (key->code == sf::Keyboard::Key::U) {
        world_.upgradeTower(cursor_);
    }
}

void TowerDefenseState::update(sf::Time fixedDt) {
    if (paused_) {
        refreshHud();
        return;
    }
    world_.fixedUpdate(fixedDt.asSeconds());
    if (world_.gameOver() || world_.victory()) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_) {
            const std::string name =
                std::string("TowerDefense-") + difficultyName(app_->difficulty());
            app_->highScores().submit(name, world_.score());
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void TowerDefenseState::refreshHud() {
    std::ostringstream os;
    const int top = app_ ? app_->highScores().topScore() : 0;
    os << "TD[" << difficultyName(world_.difficulty()) << "] S:" << world_.score() << " T:" << top << " G:"
       << world_.gold() << " HP:" << world_.lives() << " W:" << world_.wave() << "/"
       << world_.maxWaves();
    os << " | Bld:" << TowerDefenseWorld::towerTypeName(buildMode_);
    if (const auto* t = world_.towerAt(cursor_)) {
        os << " | C:" << TowerDefenseWorld::towerTypeName(t->type) << " L" << t->level << " "
           << (t->strategy ? t->strategy->name() : "-");
    }
    if (world_.victory()) {
        os << " | WIN";
    } else if (world_.gameOver()) {
        os << " | LOSE";
    } else if (paused_) {
        os << " | PAUSE";
    } else if (debugPath_) {
        os << " | DBG";
    }
    view_.setHudLine(os.str());
}

void TowerDefenseState::render(sf::RenderTarget& target) {
    view_.draw(target, world_, cursor_, debugPath_);
    if (overlayFontReady_ && paused_ && !(world_.gameOver() || world_.victory())) {
        sf::RectangleShape veil;
        veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
        veil.setFillColor(sf::Color(0, 0, 0, 150));
        target.draw(veil);

        sf::Text title(overlayFont_, "PAUSED", 54u);
        title.setFillColor(sf::Color(235, 235, 245));
        title.setPosition({static_cast<float>(target.getSize().x) * 0.5f - 110.f, 180.f});
        target.draw(title);

        const char* opts[3] = {"Resume", "Restart", "Back to Menu"};
        for (int i = 0; i < 3; ++i) {
            sf::Text t(overlayFont_, std::string(i == pauseSelection_ ? "> " : "  ") + opts[i], 30u);
            t.setFillColor(i == pauseSelection_ ? sf::Color(120, 210, 255) : sf::Color(190, 195, 210));
            t.setPosition({static_cast<float>(target.getSize().x) * 0.5f - 130.f, 280.f + i * 46.f});
            target.draw(t);
        }
        return;
    }
    if (!overlayFontReady_ || !(world_.gameOver() || world_.victory())) {
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }

    sf::RectangleShape veil;
    veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(veil);

    const char* label = world_.victory() ? "VICTORY!" : "GAME OVER";
    sf::Text text(overlayFont_, label, 64u);
    text.setFillColor(world_.victory() ? sf::Color(120, 255, 150) : sf::Color(255, 120, 120));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) * 0.5f});
    target.draw(text);
}

} // namespace mc
