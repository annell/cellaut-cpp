[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# cellaut-cpp
A general cellular automata engine implementation in C++ where you can define your own states for the automata.
It has a clean and safe interface to make the interaction with the automata as easy as possible.

## What is a Cellular Automata?
[Cellular automata](https://en.wikipedia.org/wiki/Cellular_automaton)

> A cellular automaton consists of a regular grid of cells, each in one of a finite number of states, such as on and off (in contrast to a coupled map lattice). The grid can be in any finite number of dimensions. For each cell, a set of cells called its neighborhood is defined relative to the specified cell. An initial state (time t = 0) is selected by assigning a state for each cell. A new generation is created (advancing t by 1), according to some fixed rule (generally, a mathematical function) that determines the new state of each cell in terms of the current state of the cell and the states of the cells in its neighborhood. Typically, the rule for updating the state of cells is the same for each cell and does not change over time, and is applied to the whole grid simultaneously, though exceptions are known, such as the stochastic cellular automaton and asynchronous cellular automaton.

## How to use
The cellular automata is very lightweight to use, the basic use case is:
1. Setup the states for the cellular automata with some smart logic.
```c++
class State1 {
    void Process(auto& automata, const Cell& cell) {
        if (automata.IsAt<State2>(cell.PlusY())) {
            automata.Set<State1>(cell);
        }
    }
};
class State2 {
    void Process(auto& automata, const Cell& cell) {
        if (automata.IsAt<State1>(cell.PlusX())) {
            automata.Set<State2>(cell);
        }
    }
};
```

2. Setup the cellular automata with your states.
```c++
CellularAutomata<State1, State2> automata(200, 200);
```

3. Add data to the automata.
```c++
automata.Set<State2>({100, 40});
```

4. Step the automata.
```c++
automata.Step();
```

# To install
## CMake method
1. Clone cellaut-cpp to your project `git clone --recurse-submodules`.
2. Add `add_subdirectory(path/cellaut-cpp)` to your CMakeLists.txt.
3. Link your project to `cellaut-cpp`.
4. Include `#include <cellaut-cpp/CellularAutomata.h>` in your project.

### Dependencies
- C++20

### Example on integration

# To run example
1. Clone repo to your project with submodules recursively `git clone --recurse-submodules`
2. Install dependencies.
3. Include the CMakeList in your cmake structure.
4. Build & run cellaut-cpp-example

