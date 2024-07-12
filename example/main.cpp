#include <iostream>
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
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
    static std::random_device rd;
    static std::mt19937 gen(rd());
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
                if (automata.template SwapIfTargetIs<Air>(cell, {static_cast<ShortInt>(nextX), static_cast<ShortInt>(nextY)})) {
                    return;
                } else if (!automata.template IsAt<Water>({static_cast<ShortInt>(nextX), static_cast<ShortInt>(nextY)})) {
                    stopRight = true;
                }
            }

            if (prevX > 0 && !stopLeft) {
                if (automata.template SwapIfTargetIs<Air>(cell, {static_cast<ShortInt>(prevX), static_cast<ShortInt>(nextY)})) {
                    return;
                } else if (!automata.template IsAt<Water>({static_cast<ShortInt>(prevX), static_cast<ShortInt>(nextY)})) {
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
    for (size_t y = 1; y < automata.GetHeight() - 1; y++) {
        for (size_t x = 1; x < automata.GetWidth() - 1; x++) {
            auto val = generateRandomNumber();
            if (val > 0.8 /* Set to 0 to have a clean slate */) {
                automata.template Set<Air>({static_cast<ShortInt>(x), static_cast<ShortInt>(y)});
            }
            else if (val > 0.6) {
                automata.template Set<Water>({static_cast<ShortInt>(x), static_cast<ShortInt>(y)});
            }
            else if (val > 0.3) {
                automata.template Set<Dirt>({static_cast<ShortInt>(x), static_cast<ShortInt>(y)});
            }
            else if (val > 0.1) {
                automata.template Set<Sand>({static_cast<ShortInt>(x), static_cast<ShortInt>(y)});
            }
            else if (val > 0.05) {
                automata.template Set<Stone>({static_cast<ShortInt>(x), static_cast<ShortInt>(y)});
            }
        }
    }
    automata.Step();
}

int main() {
    size_t width = 3000;
    size_t height = 1200;
    sf::RenderWindow sfmlWin(sf::VideoMode(width, height),
                             "Cellular Automata Simulation");
    using CellularAutomataT = CellularAutomata<Air, Water, Sand, Dirt, Grass, Stone, Fire>;
    CellularAutomataT automata(static_cast<ShortInt>(width), static_cast<ShortInt>(height));
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

        if (false /* Points pouring in */) {
            for (size_t i = 0; i < 50; i++) {
                automata.Set<Sand>({static_cast<ShortInt>(automata.GetWidth() / 3 + i), 0});
                automata.Set<Water>({static_cast<ShortInt>(automata.GetWidth() * 2 / 3 + i), 0});

                automata.Set<Sand>({static_cast<ShortInt>(automata.GetWidth() / 3 - i), 0});
                automata.Set<Water>({static_cast<ShortInt>(automata.GetWidth() * 2 / 3 - i), 0});
            }
        }
        if (addBlocks) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(sfmlWin);
            ShortInt x = mousePos.x / (width / automata.GetWidth());
            ShortInt y = mousePos.y / (height / automata.GetHeight());
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
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Step time: " << duration.count() << " ms\n";
        }

        {
            auto start = std::chrono::high_resolution_clock::now();
            sfmlWin.clear(sf::Color(173, 216, 230));
            std::vector<sf::Vertex> cells;
            for (ShortInt y = 0; y < automata.GetHeight(); y++) {
                for (ShortInt x = 0; x < automata.GetWidth(); x++) {
                    Cell cell = {x, y};
                    if (automata.template IsAt<Air>(cell)) {
                        continue;
                    }
                    sf::Color color;
                    if (automata.template IsAt<Sand>(cell)) {
                        color = sf::Color::Yellow;
                    }
                    else if (automata.template IsAt<Dirt>(cell)) {
                        static const sf::Color brown = sf::Color(139, 69, 19);
                        color = brown;
                    }
                    else if (automata.template IsAt<Grass>(cell)) {
                        color = sf::Color::Green;
                    }
                    else if (automata.template IsAt<Water>(cell)) {
                        color = sf::Color::Blue;
                    }
                    else if (automata.template IsAt<Stone>(cell)) {
                        static const sf::Color gray = sf::Color(128, 128, 128);
                        color = gray;
                    }
                    else if (automata.template IsAt<Fire>(cell)) {
                        color = sf::Color::Red;
                    }
                    cells.emplace_back(sf::Vector2f(x, y), color);
                }
            }
            sfmlWin.draw(&cells[0], cells.size(), sf::Points);
            sfmlWin.display();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Rendering time: " << duration.count() << " ms" << std::endl;
            std::cout << "Nr cells: " << cells.size() << std::endl;
        }
    }
    return 0;
}
