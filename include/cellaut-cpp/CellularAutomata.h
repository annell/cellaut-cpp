#pragma once
#include <variant>
#include <set>
#include <vector>
#include <type_traits>

using ShortInt = unsigned short int;

struct Cell {
    ShortInt x = 0;
    ShortInt y = 0;

    /**
     * @brief Returns a new cell with the x value incremented by 1
     */
    [[nodiscard]] Cell PlusY() const {
        return {x, static_cast<ShortInt>(y + 1)};
    }

    /**
     * @brief Returns a new cell with the y value decremented by 1
     */
    [[nodiscard]] Cell MinusY() const {
        return {x, static_cast<ShortInt>(y - 1)};
    }

    /**
     * @brief Returns a new cell with the x value incremented by 1
     */
    [[nodiscard]] Cell PlusX() const {
        return {static_cast<ShortInt>(x + 1), y};
    }

    /**
     * @brief Returns a new cell with the x value decremented by 1
     */
    [[nodiscard]] Cell MinusX() const {
        return {static_cast<ShortInt>(x - 1), y};
    }

    auto operator<=>(const Cell&) const = default;
};

/**
 * @brief Concept that defines a state that can be processed,
 * this is a requirement for the states added to the CellularAutomata.
 * @tparam TState The state to be processed
 * @tparam TNeighborhood The neighborhood of the state
 */
template <typename TState, typename TNeighborhood>
concept State = requires(TState state, TNeighborhood neighborhood) {
    { state.Process(neighborhood) };
};

template<typename... TStates>
class CellularAutomata {
private:
    using TAutomata = CellularAutomata<TStates...>;

    /**
     * @brief Represents the neighborhood of a cell, this is used to
     * interact with the automata from within the states.
     * Only exposes a simplified API towards the Automata.
     */
    class Neighborhood {
    public:
        Neighborhood(const Cell& cell, TAutomata& automata) : centerCell(cell), automata(automata) {}
        Neighborhood(const Neighborhood&) = delete;
        Neighborhood& operator=(const Neighborhood&) = delete;
        Neighborhood(Neighborhood&&) = delete;
        Neighborhood& operator=(Neighborhood&&) = delete;

        /**
         * @brief Returns the center cell of the neighborhood
         * @return The center cell
         */
        [[nodiscard]]
        const Cell& GetCenter() const {
            return centerCell;
        }

        /**
         * @brief Sets the state of the center cell
         * @tparam TState The state to set
         */
        template<State<Neighborhood> TState>
        void Set() {
            automata.template Set<TState>(GetCenter());
        }

        /**
         * @brief Returns the state of the cell
         * @tparam TState The state to check
         * @param cell The cell to check
         * @return True if the cell is of the state, false otherwise
         */
        template<State<Neighborhood> TState>
        [[nodiscard]] bool IsAt(const Cell& cell) const {
            return automata.template IsAt<TState>(cell);
        }

        /**
         * @brief Checks if the cell is valid
         * @param cell The cell to check
         * @return True if the cell is valid, false otherwise
         */
        [[nodiscard]] bool IsValid(const Cell& cell) const {
            return automata.IsValid(cell);
        }

        /**
         * @brief Swaps the state of the center cell with the target cell
         * if the target cell is of the target state
         * @tparam TTargetState The target state
         * @param target The target cell
         * @return True if the swap was successful, false otherwise
         */
        template <State<Neighborhood> TTargetState>
        [[nodiscard]] bool SwapIfTargetIs(const Cell& target) {
            return automata.template SwapIfTargetIs<TTargetState>(GetCenter(), target);
        }

        /**
         * @brief Returns the width of the automata
         * @return The width of the automata
         */
        [[nodiscard]]
       ShortInt GetWidth() const {
           return automata.GetWidth();
       }

        /**
         * @brief Returns the height of the automata
         * @return The height of the automata
         */
       [[nodiscard]]
       ShortInt GetHeight() const {
           return automata.GetHeight();
       }

    private:
        TAutomata& automata;
        const Cell& centerCell;
    };

public:
    constexpr CellularAutomata(const ShortInt Width, const ShortInt Height) : Width(Width), Height(Height) {
        updatedStates.resize(Width * Height);
        states.resize(Width * Height);
        modifiedCells.reserve(Width * Height);
        previouslyModifiedCells.reserve(Width * Height);
        changedCells.resize(Width * Height);
        for (int i = 0; i < changedCells.size(); i++) {
            changedCells[i]  = false;
        }

        using firstState = std::tuple_element<0, std::tuple<TStates...>>::type;
        for (ShortInt y = 0; y < Height; y++) {
            for (ShortInt x = 0; x < Width; x++) {
                updatedStates.at(GetIndex({x, y})) = firstState{};
                states.at(GetIndex({x, y})) = firstState{};
            }
        }
    }


    /**
     * @brief Returns the width of the automata
     * @return The width of the automata
     */
    [[nodiscard]] constexpr ShortInt GetWidth() const {
        return Width;
    }

    /**
     * @brief Returns the height of the automata
     * @return The height of the automata
     */
    [[nodiscard]] constexpr ShortInt GetHeight() const {
        return Height;
    }

    /**
     * @brief Steps the automata one step
     */
    void Step() {
        for (const auto& cell : GetPassiveBuffer()) {
            std::visit([&](auto&& state){
                Neighborhood neighborhood(cell, *this);
                state.Process(neighborhood);
            }, states.at(GetIndex(cell)));
        }
        Commit();
    }

    /**
     * @brief Checks if the cell is of the state
     * @tparam TState The state to check
     * @param cell The cell to check
     * @return True if the cell is of the state, false otherwise
     */
    template<State<Neighborhood> TState>
    [[nodiscard]] bool IsAt(const Cell& cell) const {
        return IsValid(cell) && std::get_if<TState>(&states.at(GetIndex(cell))) != nullptr;
    }

    /**
     * @brief Sets the state of the cell
     * @tparam TState The state to set
     * @param cell The cell to set the state of
     */
    template<State<Neighborhood> TState>
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
                if (!IsValid(newCell)) {
                    continue;
                }
                if (!changedCells.at(GetIndex(newCell))) {
                    buffer.push_back(newCell);
                    changedCells.at(GetIndex(newCell)) = true;
                }
            }
        }
    }

    /**
     * @brief Swaps the state of the from cell with the target cell
     * if the target cell is of the target state
     * @tparam TTargetState The target state
     * @param from The from cell
     * @param target The target cell
     * @return True if the swap was successful, false otherwise
     */
    template <State<Neighborhood> TTargetState>
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

    /**
     * Checks if the cell is valid.
     * @param cell The cell to check
     * @return True if the cell is valid, false otherwise
     */
    [[nodiscard]] bool IsValid(const Cell& cell) const {
        return cell.x >= 0 && cell.x < Width && cell.y >= 0 && cell.y < Height;
    }

private:
    void Commit() {
        for (const auto& cell : GetActiveBuffer()) {
            states.at(GetIndex(cell)) = updatedStates.at(GetIndex(cell));
            changedCells.at(GetIndex(cell)) = false;
        }
        firstBufferActive = !firstBufferActive;
        GetActiveBuffer().clear();
    }

    [[nodiscard]] size_t GetIndex(const Cell& cell) const {
        return cell.x + cell.y * Width;
    }

    using Buffer = std::vector<Cell>;
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
    std::vector<bool> changedCells;

    bool firstBufferActive = true;
};

