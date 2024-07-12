#include <iostream>
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Window/Event.hpp"
#include <cellaut-cpp/CellularAutomata.h>
#include "Controls.h"

struct Zero;
struct One;

bool IsBitSet(int num, int bit) {
    return 1 == ((num>>bit) & 1);
}

bool convert(bool a, bool b, bool c) {
    int rule = 161;
    if (a == true && b == true && c == true) return IsBitSet(rule, 7);
    if (a == true && b == true && c == false) return IsBitSet(rule, 6);
    if (a == true && b == false && c == true) return IsBitSet(rule, 5);
    if (a == true && b == false && c == false) return IsBitSet(rule, 4);
    if (a == false && b == true && c == true) return IsBitSet(rule, 3);
    if (a == false && b == true && c == false) return IsBitSet(rule, 2);
    if (a == false && b == false && c == true) return IsBitSet(rule, 1);
    if (a == false && b == false && c == false) return IsBitSet(rule, 0);
    return false;
}

void ProcessBinary(auto& neighborhood, bool centralVal) {
    auto cellPlusX = neighborhood.GetCenter().PlusX();
    auto cellMinusX = neighborhood.GetCenter().MinusX();
    if (!neighborhood.IsValid(cellPlusX) ||
        !neighborhood.IsValid(cellMinusX)) {
        return;
    }
    bool c = neighborhood.template IsAt<One>(cellPlusX);
    bool b = false;
    bool a = neighborhood.template IsAt<One>(cellMinusX);
    if (convert(a, b, c)) {
        neighborhood.template Set<One>();
    }
    else {
        neighborhood.template Set<Zero>();
    }
}

struct Zero {
    void Process(auto& neighborhood) {
        ProcessBinary(neighborhood, false);
    }
};

struct One {
    void Process(auto& neighborhood) {
        ProcessBinary(neighborhood, true);
    }
};

void buildWorld(CellularAutomata<One, Zero>& automata) {
    for (ShortInt x = 0; x < automata.GetWidth(); x++) {
        if (x == static_cast<int>(automata.GetWidth() / 2)) {
            automata.template Set<One>({x, 0});
        }
        else {
            automata.template Set<Zero>({x, 0});
        }
    }
}

int main() {
    size_t width = 3000;
    size_t height = 1200;
    sf::RenderWindow sfmlWin(sf::VideoMode(width, height),
                             "Cellular Automata Simulation");
    using CellularAutomataT = CellularAutomata<One, Zero>;
    CellularAutomataT automata(static_cast<ShortInt>(width), 1);
    buildWorld(automata);
    Controls controls;
    controls.RegisterEvent(sf::Event::EventType::Closed, [&sfmlWin](const sf::Event&) { sfmlWin.close(); });

    ShortInt y = 0;
    std::vector<sf::Vertex> cells;
    while (sfmlWin.isOpen()) {
        controls.HandleEvents(sfmlWin);
        if (y >= height) {
            continue;
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
            for (ShortInt x = 0; x < automata.GetWidth(); x++) {
                Cell cell = {x, 0};
                if (automata.template IsAt<Zero>(cell)) {
                    continue;
                }
                sf::Color color;
                if (automata.template IsAt<One>(cell)) {
                    color = sf::Color::Black;
                }
                cells.emplace_back(sf::Vector2f(x, y), color);
            }
            sfmlWin.draw(&cells[0], cells.size(), sf::Points);
            sfmlWin.display();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Rendering time: " << duration.count() << " ms" << std::endl;
            std::cout << "Nr cells: " << cells.size() << std::endl;
            y++;
        }
    }
    return 0;
}
