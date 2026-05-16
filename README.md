# MiniConsole

A small C++ “game console” host built with **SFML 3**: one window, a menu, and multiple 2D games. Playable games now include **Breakout**, **Top-down shooter**, **Platformer**, and **Tower defense**. Persistent **top-5 highscores** are tracked across runs and viewable from a dedicated menu option.

The menu includes a **Difficulty** selector (**Normal/Hard**) that applies to Platformer and Tower Defense.

## Why SFML (and not raw OpenGL)

SFML covers windowing, input, timing, and 2D drawing with a small surface area, which keeps the project focused on **simulation and structure** instead of boilerplate. SDL2 would be an equally reasonable swap at the platform layer; the important part is that **gameplay code never depends on those APIs directly**.

## Build

Install **SFML 3** (Graphics + Window + System CMake components), then:

```bash
cmake -S . -B build
cmake --build build
./build/miniconsole
```

On macOS with Homebrew you typically run `brew install sfml` and, if CMake cannot find the package, point `SFML_DIR` at the CMake config path printed by Homebrew.

### Troubleshooting

- **`find_package(SFML)` version mismatch:** This tree targets **SFML 3** (`SFML::Graphics`, `std::optional` + `sf::Event::getIf` API). Homebrew’s `sfml` formula is 3.x; use SFML 2.5 only if you intentionally downgrade the code and CMake back to 2.5.
- **`fatal error: 'chrono' file not found`:** On some macOS setups **Command Line Tools** ship an **incomplete libc++** under `/Library/Developer/CommandLineTools/usr/include/c++/v1`, while complete headers live in the SDK (`…/MacOSX.sdk/usr/include/c++/v1`). CMake here sets **`CMAKE_OSX_SYSROOT`** via `xcrun` **before** `project()` and adds **`-isystem${SDK}/usr/include/c++/v1`** so `<chrono>` resolves. Then:
  1. **`rm -rf build`** and re-run `cmake -S . -B build`.
  2. On **Apple Silicon**, avoid Rosetta CMake (`CMAKE_OSX_ARCHITECTURES=x86_64`); pass **`-DCMAKE_OSX_ARCHITECTURES=arm64`** if needed so it matches Homebrew SFML.
  3. If headers are still broken: **`sudo xcode-select -s /Applications/Xcode.app/Contents/Developer`** (full Xcode), update **Command Line Tools**, verify **`$(xcrun --show-sdk-path)/usr/include/c++/v1/chrono`** exists.

`./build/miniconsole` appears only **after a successful link**; if compile fails, that path will not exist yet.

## Controls

- **Menu:** arrow keys to move selection, Enter / Space to launch, `1`–`5` for quick picks, Esc quits.
- **Menu:** arrow keys to move selection, Enter / Space to launch, `1`–`6` quick picks, Left/Right toggles difficulty, Esc quits.
- **Breakout:** A / D or arrows to move, Space to serve the ball, R resets, Esc returns to the menu.
- **Shooter:** WASD or arrows to move, Space or Z to fire (hold to repeat), R restarts, Esc returns to the menu.
- **Platformer:** A / D or arrows move, Space / W / Up jump, release jump for short hop, `P` pause menu, `R` restart.
- **Tower Defense:** Cursor with arrows/WASD, `B` build, `U` upgrade, `Tab` path debug, `P` pause menu, `R` reset.

## Breakout Notes

- Levels load from `levels/breakout_level*.txt` where each brick character (`#`, `X`, `1`, `B`) is a block and other characters are empty.
- Bricks are colored by row (top-to-bottom red, orange, yellow, green, blue).
- Two falling powerups are implemented: **Wide Paddle** and **Big Ball**.
- End states flash an overlay for **VICTORY** and **GAME OVER**.

## Platformer Notes

- Uses coyote time, jump buffering, variable jump cut, horizontal friction, and fall gravity multiplier.
- Loads level layouts from `levels/platformer_level*.txt` (`#` solid, `X` hazard, `C` coin, `G` goal, `S` spawn).
- Completes across multiple levels before final victory.

## Architecture (what interviewers look for)

The loop in `Application` runs rendering as fast as the display allows, but **simulation is stepped at a fixed 60 Hz** using `GameClock`’s accumulator. Variable frame rates therefore change how many fixed steps run per rendered frame—not the physics integrator itself—so behavior stays consistent across machines.

Each screen is a `GameState` (menu, breakout, stubs). Gameplay lives in **world** types (for example `BreakoutWorld`) that only know about math and rules, while **view** types (`BreakoutView`) translate model state into SFML draw calls. Collisions in Breakout are **circle vs. axis-aligned boxes** for bricks and the paddle; that is fast, stable, and easy to reason about compared with pixel-perfect tests. The tradeoff is subtle tunneling at very high speeds, which is mitigated lightly with substeps and could be improved further with swept tests or smaller fixed deltas if the design space demands it.

The entity-heavy games you add next should keep that same split: a registry or small ECS for logic, pooling for bullets, and a renderer that only observes data. A short, honest paragraph in this README about what you tried, what you skipped, and what you would revisit is usually more credible than a long feature checklist.

## What to build next

- **Shooter:** extend with weapon patterns, enemy types / FSM, and a coarse spatial grid if entity counts grow.
- **Platformer:** gravity, jump buffering, tilemap collision with swept AABB.
- **Tower defense:** grid pathing, wave controller, economy + build/upgrade rules.

Keep each game “small but finished” before expanding scope.
