#pragma once
#include <variant>
#include <set>
#include <vector>
#include <type_traits>

struct Cell {
    size_t x = 0;
    size_t y = 0;

    [[nodiscard]] Cell PlusY() const {
        return {x, y + 1};
    }

    [[nodiscard]] Cell MinusY() const {
        return {x, y - 1};
    }

    [[nodiscard]] Cell PlusX() const {
        return {x + 1, y};
    }

    [[nodiscard]] Cell MinusX() const {
        return {x - 1, y};
    }

    auto operator<=>(const Cell&) const = default;
};

bool IsValid(const Cell& cell, size_t Width, size_t Height) {
    return cell.x >= 0 && cell.x < Width && cell.y >= 0 && cell.y < Height;
}

template<typename... TStates>
class CellularAutomata {
public:
    constexpr CellularAutomata(const size_t Width, const size_t Height) : Width(Width), Height(Height) {
        updatedStates.resize(Width * Height);
        states.resize(Width * Height);

        using firstState = std::tuple_element<0, std::tuple<TStates...>>::type;
        for (size_t y = 0; y < Height; y++) {
            for (size_t x = 0; x < Width; x++) {
                updatedStates.at(GetIndex({x, y})) = firstState{};
                states.at(GetIndex({x, y})) = firstState{};
            }
        }
    }

    [[nodiscard]] constexpr size_t GetWidth() const {
        return Width;
    }

    [[nodiscard]] constexpr size_t GetHeight() const {
        return Height;
    }

    void Step() {
        for (const auto& cell : previouslyModifiedCells) {
            std::visit([&](auto&& state){
                state.Process(*this, cell);
            }, states.at(GetIndex(cell)));
        }
        Commit();
    }

    template<typename TState>
    [[nodiscard]] bool IsAt(const Cell& cell) const {
        return std::get_if<TState>(&states.at(GetIndex(cell))) != nullptr;
    }

    template<typename TState>
    void Set(const Cell& cell) {
        updatedStates.at(GetIndex(cell)) = TState{};

        modifiedCells.insert(cell);
        for (int x = -updatedWindowsSize; x <= updatedWindowsSize; x++) {
            for (int y = -updatedWindowsSize; y <= updatedWindowsSize; y++) {
                if (x == 0 && y == 0) {
                    continue;
                }
                Cell newCell = {cell.x + x, cell.y + y};
                if (IsValid(newCell, Width, Height)) {
                    modifiedCells.insert(newCell);
                }
            }
        }
    }

    template <typename TTargetState>
    bool SwapIfTargetIs(const Cell& from, const Cell& target) {
        if (IsAt<TTargetState>(target)) {
            auto setState = [&](const Cell& cell1, const Cell& cell2) {
                std::visit([&](auto&& stateTarget){
                    Set<typename std::remove_reference<decltype(stateTarget)>::type>(cell2);
                }, states.at(GetIndex(cell1)));
            };
            setState(from, target);
            setState(target, from);
            return true;
        }
        return false;
    }

private:
    void Commit() {
        for (const auto& cell : modifiedCells) {
            states.at(GetIndex(cell)) = updatedStates.at(GetIndex(cell));
        }
        previouslyModifiedCells = modifiedCells;
        modifiedCells.clear();
    }

    [[nodiscard]] size_t GetIndex(const Cell& cell) const {
        return cell.x + cell.y * Width;
    }

    const size_t Height = 0;
    const size_t Width = 0;
    std::vector<std::variant<TStates ...>> updatedStates;
    std::vector<std::variant<TStates ...>> states;

    const size_t updatedWindowsSize = 1;
    std::set<Cell> modifiedCells;
    std::set<Cell> previouslyModifiedCells;
};

