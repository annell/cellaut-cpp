project(cellaut-cpp-example)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(${PROJECT_NAME} main.cpp Controls.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-window sfml-graphics cellaut-cpp)
