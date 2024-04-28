#include <iostream>
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Window/Event.hpp"
#include <cellaut-cpp/CellularAutomata.h>
#include "Controls.h"
#include <random>

struct Air;
struct Water;
struct Sand;
struct Dirt;
struct Grass;
struct Stone;
struct Fire;


struct Sand {
    void Process(auto& automata, const Cell& cell) {
        if (cell.y + 1 >= automata.GetHeight()) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.PlusY())) {
            return;
        }

        if (automata.template IsAt<Water>(cell.PlusY())) {
        automata.template Set<Dirt>(cell.PlusY());
        automata.template Set<Air>(cell);
        return;
        }

        if (cell.x + 1 >= automata.GetWidth()) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.PlusY().PlusX())) {
            return;
        }

        if (cell.x - 1 < 0) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.MinusX().PlusY())) {
            return;
        }
    }

};
struct Air {
    void Process(auto&, const Cell&) {}
};
double generateRandomNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    return dis(gen);
}

struct Dirt {
    void Process(auto& automata, const Cell& cell) {
        if (cell.y + 1 >= automata.GetHeight()) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.PlusY())) {
            return;
        }

        if (automata.template IsAt<Dirt>(cell.PlusY()) &&
            automata.template IsAt<Air>(cell.MinusY())) {
            if (generateRandomNumber() < 0.001) {
                automata.template Set<Grass>(cell);
            }
            return;
        }

        if (automata.template SwapIfTargetIs<Water>(cell, cell.PlusY())) {
            return;
        }
    }
};
struct Grass {
    void Process(auto& automata, const Cell& cell) {
        if (cell.y + 1 >= automata.GetHeight()) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.PlusY())) {
            return;
        }
    }


};
struct Water {
    void Process(auto& automata, const Cell& cell) {
        size_t nextY = cell.y + 1;
        if (nextY >= automata.GetHeight()) {
            return;
        }

        if (automata.template SwapIfTargetIs<Air>(cell, cell.PlusY())) {
            return;
        }

        int i = 0;
        bool stopRight = false;
        bool stopLeft = false;
        while (true) {
            i++;
            size_t nextX = cell.x + i;
            size_t prevX = cell.x - i;
            if (i > automata.GetWidth()) {
                return;
            }
            if (nextX < automata.GetWidth() && !stopRight) {
                if (automata.template SwapIfTargetIs<Air>(cell, {static_cast<Cell::varSize>(nextX), static_cast<Cell::varSize>(nextY)})) {
                    return;
                } else if (!automata.template IsAt<Water>({static_cast<Cell::varSize>(nextX), static_cast<Cell::varSize>(nextY)})) {
                    stopRight = true;
                }
            }

            if (prevX > 0 && !stopLeft) {
                if (automata.template SwapIfTargetIs<Air>(cell, {static_cast<Cell::varSize>(prevX), static_cast<Cell::varSize>(nextY)})) {
                    return;
                } else if (!automata.template IsAt<Water>({static_cast<Cell::varSize>(prevX), static_cast<Cell::varSize>(nextY)})) {
                    stopLeft = true;
                }
            }
        }
    }


};
struct Stone {
    void Process(auto&, const Cell&) {}
};
struct Fire {
    void Process(auto&, const Cell&) {}
};

enum class WorldType {
    Air = 0,
    Sand = 1<<0,
    Dirt = 1<<1,
    Grass = 1<<2,
    Water = 1<<3,
    Stone = 1<<4,
    Fire = 1<<5
};

void buildWorld(auto& automata) {
    for (size_t y = 0; y < automata.GetHeight(); y++) {
        for (size_t x = 0; x < automata.GetWidth(); x++) {
            automata.template Set<Air>({static_cast<Cell::varSize>(x), static_cast<Cell::varSize>(y)});
        }
    }
    automata.Step();
}

int main() {
    size_t width = 2000;
    size_t height = 1200;
    sf::RenderWindow sfmlWin(sf::VideoMode(width, height),
                             "Cellular Automata Simulation");
    using CellularAutomataT = CellularAutomata<Air, Water, Sand, Dirt, Grass, Stone, Fire>;
    CellularAutomataT automata(static_cast<Cell::varSize>(width), static_cast<Cell::varSize>(height));
    Controls controls;
    WorldType blockSelected = WorldType::Sand;
    bool addBlocks = false;

    controls.RegisterEvent(sf::Event::EventType::Closed, [&sfmlWin](const sf::Event&) { sfmlWin.close(); });
    controls.RegisterEvent(sf::Event::EventType::MouseButtonPressed, [&](const sf::Event& e) {
        if (e.mouseButton.button == sf::Mouse::Button::Left) {
            addBlocks = true;
        }
    });
    controls.RegisterEvent(sf::Event::EventType::MouseButtonReleased, [&](const sf::Event& e) {
        if (e.mouseButton.button == sf::Mouse::Button::Left) {
            addBlocks = false;
        }
    });
    controls.RegisterEvent(sf::Event::EventType::KeyPressed, [&](const sf::Event& e) {
        if (e.key.code == sf::Keyboard::Num1) {
            blockSelected = WorldType::Sand;
        }
        if (e.key.code == sf::Keyboard::Num2) {
            blockSelected = WorldType::Dirt;
        }
        if (e.key.code == sf::Keyboard::Num3) {
            blockSelected = WorldType::Air;
        }
        if (e.key.code == sf::Keyboard::Num4) {
            blockSelected = WorldType::Water;
        }
        if (e.key.code == sf::Keyboard::Num5) {
            blockSelected = WorldType::Stone;
        }
    });
    buildWorld(automata);
    while (sfmlWin.isOpen()) {
        controls.HandleEvents(sfmlWin);

        if (true) {
            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 + 1), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 + 1), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 + 2), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 + 2), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 + 3), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 + 3), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 + 4), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 + 4), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 - 1), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 - 1), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 - 2), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 - 2), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 - 3), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 - 3), 0});

            automata.Set<Sand>({static_cast<Cell::varSize>(automata.GetWidth() / 3 - 4), 0});
            automata.Set<Water>({static_cast<Cell::varSize>(automata.GetWidth() * 2 / 3 - 4), 0});
        }
        if (addBlocks) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(sfmlWin);
            Cell::varSize x = mousePos.x / (width / automata.GetWidth());
            Cell::varSize y = mousePos.y / (height / automata.GetHeight());
            switch (blockSelected) {
                case WorldType::Sand:
                    automata.Set<Sand>({x, y});
                    break;
                case WorldType::Dirt:
                    automata.Set<Dirt>({x, y});
                    break;
                case WorldType::Air:
                    automata.Set<Air>({x, y});
                    break;
                case WorldType::Water:
                    automata.Set<Water>({x, y});
                    break;
                case WorldType::Stone:
                    automata.Set<Stone>({x, y});
                    break;
                case WorldType::Fire:
                    automata.Set<Fire>({x, y});
                    break;
                case WorldType::Grass:
                    break;
            }
        }
        {
            auto start = std::chrono::high_resolution_clock::now();
            automata.Step();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            //std::cout << "Step time: " << duration.count() << " microseconds\n";
        }

        {
            auto start = std::chrono::high_resolution_clock::now();
            sfmlWin.clear(sf::Color(173, 216, 230));
            for (Cell::varSize y = 0; y < automata.GetHeight(); y++) {
                for (Cell::varSize x = 0; x < automata.GetWidth(); x++) {
                    if (automata.template IsAt<Air>({x, y})) {
                        continue;
                    }
                    sf::RectangleShape cell({static_cast<float>(height) / static_cast<float>(automata.GetHeight()), static_cast<float>(width) / static_cast<float>(automata.GetWidth())});
                    cell.setPosition(static_cast<float>(x * height) / static_cast<float>(automata.GetHeight()), static_cast<float>(y * width) / static_cast<float>(automata.GetWidth()));

                    if (automata.template IsAt<Sand>({x, y})) {
                        cell.setFillColor(sf::Color::Yellow);
                    }
                    else if (automata.template IsAt<Dirt>({x, y})) {
                        sf::Color brown = sf::Color(139, 69, 19);
                        cell.setFillColor(brown);
                    }
                    else if (automata.template IsAt<Grass>({x, y})) {
                        cell.setFillColor(sf::Color::Green);
                    }
                    else if (automata.template IsAt<Water>({x, y})) {
                        cell.setFillColor(sf::Color::Blue);
                    }
                    else if (automata.template IsAt<Stone>({x, y})) {
                        sf::Color gray = sf::Color(128, 128, 128);
                        cell.setFillColor(gray);
                    }
                    else if (automata.template IsAt<Fire>({x, y})) {
                        cell.setFillColor(sf::Color::Red);
                    }

                    sfmlWin.draw(cell);
                }
            }
            sfmlWin.display();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            //std::cout << "Rendering time: " << duration.count() << " ms\n";
        }
    }
    return 0;
}
