#include "states/MenuState.hpp"

#include "core/Application.hpp"
#include "core/SystemFont.hpp"
#include "states/BreakoutState.hpp"
#include "states/HighScoresState.hpp"
#include "states/MinesweeperState.hpp"
#include "states/PacmanState.hpp"
#include "states/PlatformerState.hpp"
#include "states/ShooterState.hpp"
#include "states/TowerDefenseState.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace mc {

namespace {

struct MenuEntry {
    const char* title;
    const char* blurb;
};

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 11u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

std::string wrapTextToWidth(const sf::Font& font, const std::string& input, unsigned int size, float maxWidth) {
    sf::Text probe(font, "", size);
    std::istringstream words(input);
    std::string word;
    std::string line;
    std::string out;
    bool firstLine = true;
    while (words >> word) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        probe.setString(candidate);
        if (!line.empty() && probe.getLocalBounds().size.x > maxWidth) {
            if (!firstLine) {
                out += "\n";
            }
            out += line;
            line = word;
            firstLine = false;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) {
        if (!firstLine) {
            out += "\n";
        }
        out += line;
    }
    return out.empty() ? input : out;
}

const std::vector<MenuEntry> kEntries = {
    {"Breakout / Arkanoid",
     "Circle-vs-AABB collisions, paddle aim, lives, score — full logic in BreakoutWorld."},
    {"Top-down shooter",
     "Pooled bullets, seek steering on enemies, spawn cadence, invulnerability frames — logic in "
     "ShooterWorld."},
    {"Platformer",
     "Gravity, jump mechanics, tile collisions, coins and goal."},
    {"Tower defense",
     "Grid pathfinding, wave spawns, build and upgrade towers."},
    {"Minesweeper",
     "Deferred mine placement, first-click safety, BFS flood reveal, and chord clicking."},
    {"Pac-Man",
     "Mode-based ghost AI with personality targets and greedy intersection decisions."},
    {"High Scores",
     "View persistent top-5 scores across games."},
};

} // namespace

void MenuState::onEnter(Application& app) {
    app_ = &app;
    fontReady_ = tryLoadSystemFont(font_);
    if (selection_ < 0 || selection_ >= static_cast<int>(kEntries.size())) {
        selection_ = 0;
    }
    selectingDifficulty_ = false;
    selectingMinesweeperPreset_ = false;
    pendingGameSelection_ = -1;
    pendingDifficulty_ = app.difficulty();
    pendingMinesweeperPreset_ = 0;
}

void MenuState::launchGameForSelection(int gameSelection) {
    if (!app_) {
        return;
    }
    switch (gameSelection) {
    case 0:
        app_->requestState(std::make_unique<BreakoutState>());
        break;
    case 1:
        app_->requestState(std::make_unique<ShooterState>());
        break;
    case 2:
        app_->requestState(std::make_unique<PlatformerState>());
        break;
    case 3:
        app_->requestState(std::make_unique<TowerDefenseState>());
        break;
    case 4:
        if (pendingMinesweeperPreset_ == 1) {
            app_->requestState(std::make_unique<MinesweeperState>(MinesweeperWorld::Difficulty::Intermediate));
        } else if (pendingMinesweeperPreset_ == 2) {
            app_->requestState(std::make_unique<MinesweeperState>(MinesweeperWorld::Difficulty::Expert));
        } else {
            app_->requestState(std::make_unique<MinesweeperState>(MinesweeperWorld::Difficulty::Beginner));
        }
        break;
    case 5:
        app_->requestState(std::make_unique<PacmanState>());
        break;
    case 6:
        app_->requestState(std::make_unique<HighScoresState>());
        break;
    default:
        break;
    }
}

void MenuState::beginDifficultySelection(int gameSelection) {
    selectingDifficulty_ = true;
    selectingMinesweeperPreset_ = false;
    pendingGameSelection_ = gameSelection;
    pendingDifficulty_ = app_ ? app_->difficulty() : GameDifficulty::Normal;
}

void MenuState::beginMinesweeperSelection() {
    selectingMinesweeperPreset_ = true;
    selectingDifficulty_ = false;
    pendingMinesweeperPreset_ = 0;
}

void MenuState::launchSelection() {
    if (!app_) {
        return;
    }
    switch (selection_) {
    case 0:
        launchGameForSelection(selection_);
        break;
    case 1:
        launchGameForSelection(selection_);
        break;
    case 2:
        beginDifficultySelection(selection_);
        break;
    case 3:
        beginDifficultySelection(selection_);
        break;
    case 4:
        beginMinesweeperSelection();
        break;
    case 5:
    case 6:
        launchGameForSelection(selection_);
        break;
    default:
        break;
    }
}

void MenuState::handleInput(const sf::Event& event) {
    if (selectingMinesweeperPreset_) {
        const auto* key = event.getIf<sf::Event::KeyPressed>();
        if (!key) {
            return;
        }
        if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::Right) {
            pendingMinesweeperPreset_ = (pendingMinesweeperPreset_ + 1) % 3;
        } else if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
            selectingMinesweeperPreset_ = false;
            launchGameForSelection(4);
        } else if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Backspace) {
            selectingMinesweeperPreset_ = false;
        }
        return;
    }

    if (selectingDifficulty_) {
        const auto* key = event.getIf<sf::Event::KeyPressed>();
        if (!key) {
            return;
        }
        if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::Right) {
            pendingDifficulty_ = toggledDifficulty(pendingDifficulty_);
        } else if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
            if (app_) {
                app_->setDifficulty(pendingDifficulty_);
            }
            selectingDifficulty_ = false;
            launchGameForSelection(pendingGameSelection_);
        } else if (key->code == sf::Keyboard::Key::Escape || key->code == sf::Keyboard::Key::Backspace) {
            selectingDifficulty_ = false;
            pendingGameSelection_ = -1;
        }
        return;
    }

    if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
        const char32_t ch = te->unicode;
        if (ch >= U'1' && ch <= U'7') {
            selection_ = static_cast<int>(ch - U'1');
            launchSelection();
        }
        return;
    }

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) {
        return;
    }

    switch (key->code) {
    case sf::Keyboard::Key::Up:
        selection_ = (selection_ - 1 + static_cast<int>(kEntries.size())) % static_cast<int>(kEntries.size());
        break;
    case sf::Keyboard::Key::Down:
        selection_ = (selection_ + 1) % static_cast<int>(kEntries.size());
        break;
    case sf::Keyboard::Key::Enter:
    case sf::Keyboard::Key::Space:
        launchSelection();
        break;
    case sf::Keyboard::Key::Escape:
        if (app_) {
            app_->quit();
        }
        break;
    default:
        break;
    }
}

void MenuState::update(sf::Time fixedDt) { (void)fixedDt; }

void MenuState::render(sf::RenderTarget& target) {
    const float w = static_cast<float>(target.getSize().x);
    const float h = static_cast<float>(target.getSize().y);

    sf::RectangleShape backdrop({w, h});
    backdrop.setFillColor(sf::Color(14, 18, 28));
    target.draw(backdrop);

    sf::CircleShape glowA(260.f);
    glowA.setOrigin({260.f, 260.f});
    glowA.setPosition({w - 140.f, 90.f});
    glowA.setFillColor(sf::Color(80, 140, 220, 35));
    target.draw(glowA);

    sf::CircleShape glowB(320.f);
    glowB.setOrigin({320.f, 320.f});
    glowB.setPosition({110.f, h - 50.f});
    glowB.setFillColor(sf::Color(120, 90, 200, 28));
    target.draw(glowB);

    sf::RectangleShape panel({w - 80.f, h - 140.f});
    panel.setPosition({40.f, 28.f});
    panel.setFillColor(sf::Color(10, 12, 18, 185));
    panel.setOutlineColor(sf::Color(82, 92, 118, 150));
    panel.setOutlineThickness(1.f);
    target.draw(panel);

    if (!fontReady_) {
        return;
    }

    sf::Text heading(font_, "MiniConsole", 42u);
    heading.setFillColor(sf::Color(245, 245, 250));
    heading.setPosition({48.f, 36.f});
    target.draw(heading);

    sf::Text sub(font_, "SFML 3 host + fixed timestep + logic/render split", 18u);
    sub.setFillColor(sf::Color(170, 180, 200));
    sub.setPosition({52.f, 96.f});
    target.draw(sub);

    const float listWidth = std::max(300.f, w - 460.f);
    float y = 154.f;
    for (int i = 0; i < static_cast<int>(kEntries.size()); ++i) {
        const bool selected = i == selection_;
        const std::string title = kEntries[static_cast<std::size_t>(i)].title;

        sf::RectangleShape row({listWidth, selected ? 36.f : 30.f});
        row.setPosition({56.f, y - (selected ? 1.f : -1.f)});
        row.setFillColor(selected ? sf::Color(70, 130, 210, 62) : sf::Color(255, 255, 255, 6));
        row.setOutlineColor(selected ? sf::Color(120, 200, 255, 175) : sf::Color(0, 0, 0, 0));
        row.setOutlineThickness(selected ? 1.f : 0.f);
        target.draw(row);

        sf::Text line(font_, std::string(i == selection_ ? "> " : "  ") + title, selected ? 24u : 22u);
        line.setFillColor(selected ? sf::Color(120, 200, 255) : sf::Color(200, 205, 220));
        line.setPosition({66.f, y});
        target.draw(line);
        y += selected ? 43.f : 36.f;
    }

    const float infoCardWidth = std::max(290.f, w - 470.f);
    sf::RectangleShape infoCard({infoCardWidth, 124.f});
    infoCard.setPosition({56.f, y + 14.f});
    infoCard.setFillColor(sf::Color(18, 22, 34, 215));
    infoCard.setOutlineColor(sf::Color(95, 110, 140, 160));
    infoCard.setOutlineThickness(1.f);
    target.draw(infoCard);

    const std::string wrappedBlurb =
        wrapTextToWidth(font_, kEntries[static_cast<std::size_t>(selection_)].blurb, 16u, infoCardWidth - 24.f);
    sf::Text blurb(font_, wrappedBlurb, 16u);
    blurb.setFillColor(sf::Color(165, 175, 198));
    blurb.setPosition({72.f, y + 30.f});
    blurb.setLineSpacing(1.15f);
    target.draw(blurb);

    sf::Text help(font_,
                  "Up/Down navigate | Enter launch | 1-7 quick start | Esc quit",
                  15u);
    help.setFillColor(sf::Color(130, 140, 160));
    help.setPosition({56.f, h - 58.f});
    fitTextToWidth(help, w - 112.f, 15u);
    target.draw(help);

    if (!selectingDifficulty_) {
        if (!selectingMinesweeperPreset_) {
            return;
        }
    }

    if (selectingMinesweeperPreset_) {
        sf::RectangleShape veil;
        veil.setSize({w, h});
        veil.setFillColor(sf::Color(0, 0, 0, 150));
        target.draw(veil);

        sf::RectangleShape dialog({620.f, 340.f});
        dialog.setPosition({170.f, 188.f});
        dialog.setFillColor(sf::Color(14, 18, 26, 235));
        dialog.setOutlineColor(sf::Color(105, 125, 165, 190));
        dialog.setOutlineThickness(1.f);
        target.draw(dialog);

        sf::Text title(font_, "Minesweeper Difficulty", 34u);
        title.setFillColor(sf::Color(240, 242, 248));
        title.setPosition({198.f, 220.f});
        fitTextToWidth(title, 564.f, 34u, 22u);
        target.draw(title);

        const char* labels[3] = {"Beginner 9x9 / 10", "Intermediate 16x16 / 40", "Expert 30x16 / 99"};
        for (int i = 0; i < 3; ++i) {
            sf::Text line(font_, std::string(i == pendingMinesweeperPreset_ ? "> " : "  ") + labels[i], 25u);
            line.setFillColor(i == pendingMinesweeperPreset_ ? sf::Color(120, 210, 255) : sf::Color(180, 188, 204));
            line.setPosition({222.f, 302.f + i * 46.f});
            fitTextToWidth(line, 544.f, 25u, 16u);
            target.draw(line);
        }

        sf::Text prompt(font_, "Left/Right change | Enter start | Esc cancel", 17u);
        prompt.setFillColor(sf::Color(170, 178, 193));
        prompt.setPosition({222.f, 474.f});
        fitTextToWidth(prompt, 544.f, 17u, 12u);
        target.draw(prompt);
        return;
    }

    sf::RectangleShape veil;
    veil.setSize({w, h});
    veil.setFillColor(sf::Color(0, 0, 0, 150));
    target.draw(veil);

    sf::RectangleShape dialog({620.f, 360.f});
    dialog.setPosition({170.f, 176.f});
    dialog.setFillColor(sf::Color(14, 18, 26, 235));
    dialog.setOutlineColor(sf::Color(105, 125, 165, 190));
    dialog.setOutlineThickness(1.f);
    target.draw(dialog);

    const char* gameName = pendingGameSelection_ == 3 ? "Tower Defense" : "Platformer";
    sf::Text title(font_, std::string("Select Difficulty - ") + gameName, 34u);
    title.setFillColor(sf::Color(240, 242, 248));
    title.setPosition({198.f, 206.f});
    fitTextToWidth(title, 564.f, 34u, 22u);
    target.draw(title);

    sf::Text normal(font_, std::string(pendingDifficulty_ == GameDifficulty::Normal ? "> " : "  ") + "Normal", 28u);
    normal.setFillColor(pendingDifficulty_ == GameDifficulty::Normal ? sf::Color(120, 210, 255)
                                                                      : sf::Color(180, 188, 204));
    normal.setPosition({256.f, 292.f});
    fitTextToWidth(normal, 530.f, 28u, 18u);
    target.draw(normal);

    sf::Text hard(font_, std::string(pendingDifficulty_ == GameDifficulty::Hard ? "> " : "  ") + "Hard", 28u);
    hard.setFillColor(pendingDifficulty_ == GameDifficulty::Hard ? sf::Color(255, 150, 130)
                                                                  : sf::Color(180, 188, 204));
    hard.setPosition({256.f, 338.f});
    fitTextToWidth(hard, 530.f, 28u, 18u);
    target.draw(hard);

    sf::Text prompt(font_, "Left/Right change | Enter start | Esc cancel", 17u);
    prompt.setFillColor(sf::Color(170, 178, 193));
    prompt.setPosition({256.f, 398.f});
    fitTextToWidth(prompt, 530.f, 17u, 12u);
    target.draw(prompt);

    const bool towerDefense = pendingGameSelection_ == 3;
    const std::string normalDesc = towerDefense
                                       ? "Normal: standard waves and economy."
                                       : "Normal: standard level layout and forgiving timing.";
    const std::string hardDesc = towerDefense
                                     ? "Hard: tougher enemies, faster pace, tighter resources."
                                     : "Hard: denser hazards, attack orbs, tighter jump windows.";

    sf::Text normalInfo(font_, normalDesc, 16u);
    normalInfo.setFillColor(sf::Color(145, 198, 235));
    normalInfo.setPosition({256.f, 442.f});
    fitTextToWidth(normalInfo, 530.f, 16u, 11u);
    target.draw(normalInfo);

    sf::Text hardInfo(font_, hardDesc, 16u);
    hardInfo.setFillColor(sf::Color(236, 148, 136));
    hardInfo.setPosition({256.f, 466.f});
    fitTextToWidth(hardInfo, 530.f, 16u, 11u);
    target.draw(hardInfo);
}

} // namespace mc
