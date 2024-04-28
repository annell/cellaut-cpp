#pragma once
#include <variant>
#include <set>
#include <vector>
#include <type_traits>

struct Cell {
    using varSize = unsigned short int;
    varSize x = 0;
    varSize y = 0;

    [[nodiscard]] Cell PlusY() const {
        return {x, static_cast<varSize>(y + 1)};
    }

    [[nodiscard]] Cell MinusY() const {
        return {x, static_cast<varSize>(y - 1)};
    }

    [[nodiscard]] Cell PlusX() const {
        return {static_cast<varSize>(x + 1), y};
    }

    [[nodiscard]] Cell MinusX() const {
        return {static_cast<varSize>(x - 1), y};
    }

    auto operator<=>(const Cell&) const = default;
};

bool IsValid(const Cell& cell, size_t Width, size_t Height) {
    return cell.x >= 0 && cell.x < Width && cell.y >= 0 && cell.y < Height;
}

template<typename... TStates>
class CellularAutomata {
public:
    constexpr CellularAutomata(const Cell::varSize Width, const Cell::varSize Height) : Width(Width), Height(Height) {
        updatedStates.resize(Width * Height);
        states.resize(Width * Height);

        using firstState = std::tuple_element<0, std::tuple<TStates...>>::type;
        for (Cell::varSize y = 0; y < Height; y++) {
            for (Cell::varSize x = 0; x < Width; x++) {
                updatedStates.at(GetIndex({x, y})) = firstState{};
                states.at(GetIndex({x, y})) = firstState{};
            }
        }
    }

    [[nodiscard]] constexpr Cell::varSize GetWidth() const {
        return Width;
    }

    [[nodiscard]] constexpr Cell::varSize GetHeight() const {
        return Height;
    }

    void Step() {
        for (const auto& cell : GetPassiveBuffer()) {
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

        auto& buffer = GetActiveBuffer();
        buffer.insert(cell);
        for (int x = -neighborhoodSize; x <= neighborhoodSize; x++) {
            for (int y = -neighborhoodSize; y <= neighborhoodSize; y++) {
                if (x == 0 && y == 0) {
                    continue;
                }
                Cell newCell = {static_cast<Cell::varSize>(cell.x + x), static_cast<Cell::varSize>(cell.y + y)};
                if (IsValid(newCell, Width, Height)) {
                    buffer.insert(newCell);
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
        for (const auto& cell : GetActiveBuffer()) {
            states.at(GetIndex(cell)) = updatedStates.at(GetIndex(cell));
        }
        firstBufferActive = !firstBufferActive;
        GetActiveBuffer().clear();
    }

    [[nodiscard]] size_t GetIndex(const Cell& cell) const {
        return cell.x + cell.y * Width;
    }

    std::set<Cell>& GetActiveBuffer () {
        return firstBufferActive ? modifiedCells : previouslyModifiedCells;
    }

    std::set<Cell>& GetPassiveBuffer () {
        return firstBufferActive ? previouslyModifiedCells : modifiedCells;
    }

    const Cell::varSize Height = 0;
    const Cell::varSize Width = 0;
    std::vector<std::variant<TStates ...>> updatedStates;
    std::vector<std::variant<TStates ...>> states;

    const size_t neighborhoodSize = 1;
    std::set<Cell> modifiedCells;
    std::set<Cell> previouslyModifiedCells;

    bool firstBufferActive = true;
};

