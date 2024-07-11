#pragma once
#include <variant>
#include <set>
#include <vector>
#include <type_traits>

using ShortInt = unsigned short int;

struct Cell {
    ShortInt x = 0;
    ShortInt y = 0;

    [[nodiscard]] Cell PlusY() const {
        return {x, static_cast<ShortInt>(y + 1)};
    }

    [[nodiscard]] Cell MinusY() const {
        return {x, static_cast<ShortInt>(y - 1)};
    }

    [[nodiscard]] Cell PlusX() const {
        return {static_cast<ShortInt>(x + 1), y};
    }

    [[nodiscard]] Cell MinusX() const {
        return {static_cast<ShortInt>(x - 1), y};
    }

    auto operator<=>(const Cell&) const = default;
};

bool IsValid(Cell cell, ShortInt Width, ShortInt Height) {
    return cell.x >= 0 && cell.x < Width && cell.y >= 0 && cell.y < Height;
}

template <typename TAutomata>
class Neighborhood {
public:
    enum Neighbor {
        TopLeft = 0,
        Top = 1,
        TopRight = 2,
        Left = 3,
        Right = 4,
        BottomLeft = 5,
        Bottom = 6,
        BottomRight = 7
    };

    Neighborhood(const Cell& baseCell) : baseCell(baseCell) {}

    [[nodiscard]] Cell GetNeighbor(Neighbor neighbor) const {
        switch (neighbor) {
            case TopLeft:
                return baseCell.MinusX().MinusY();
            case Top:
                return baseCell.MinusY();
            case TopRight:
                return baseCell.PlusX().MinusY();
            case Left:
                return baseCell.MinusX();
            case Right:
                return baseCell.PlusX();
            case BottomLeft:
                return baseCell.MinusX().PlusY();
            case Bottom:
                return baseCell.PlusY();
            case BottomRight:
                return baseCell.PlusX().PlusY();
        }
    }

private:
    Cell baseCell;
    TAutomata& automata;
};

template<typename... TStates>
class CellularAutomata {
private:
    using Buffer = std::vector<Cell>;
public:
    constexpr CellularAutomata(const ShortInt Width, const ShortInt Height) : Width(Width), Height(Height) {
        updatedStates.resize(Width * Height);
        states.resize(Width * Height);
        modifiedCells.reserve(Width * Height);
        previouslyModifiedCells.reserve(Width * Height);


        using firstState = std::tuple_element<0, std::tuple<TStates...>>::type;
        for (ShortInt y = 0; y < Height; y++) {
            for (ShortInt x = 0; x < Width; x++) {
                updatedStates.at(GetIndex({x, y})) = firstState{};
                states.at(GetIndex({x, y})) = firstState{};
            }
        }
    }

    [[nodiscard]] constexpr ShortInt GetWidth() const {
        return Width;
    }

    [[nodiscard]] constexpr ShortInt GetHeight() const {
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
        buffer.push_back(cell);
        for (int x = -neighborhoodSize; x <= neighborhoodSize; x++) {
            for (int y = -neighborhoodSize; y <= neighborhoodSize; y++) {
                if (x == 0 && y == 0) {
                    continue;
                }
                Cell newCell = {static_cast<ShortInt>(cell.x + x), static_cast<ShortInt>(cell.y + y)};
                if (IsValid(newCell, Width, Height)) {
                    buffer.push_back(newCell);
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

    [[nodiscard]] size_t Size() const {
        return states.size();
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

    Buffer& GetActiveBuffer () {
        return firstBufferActive ? modifiedCells : previouslyModifiedCells;
    }

    Buffer& GetPassiveBuffer () {
        return firstBufferActive ? previouslyModifiedCells : modifiedCells;
    }

    const ShortInt Height = 0;
    const ShortInt Width = 0;
    std::vector<std::variant<TStates ...>> updatedStates;
    std::vector<std::variant<TStates ...>> states;

    const size_t neighborhoodSize = 1;
    Buffer modifiedCells;
    Buffer previouslyModifiedCells;

    bool firstBufferActive = true;
};

