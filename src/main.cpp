#include <cassert>
#include <glm/glm.hpp>
using namespace glm;

// #include "GLFW/glfw3.h"
// #include "macros.hpp"
#define GLFW_INCLUDE_NONE
#include "logic.hpp"

// #define let auto&

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    vec2 ssize = vec2(screenWidth, screenHeight);

    // SetTraceLogLevel(LOG_WARNING);
    // InitWindow(screenWidth, screenHeight, "Box2D - Physics Simulation");
    // SetTargetFPS(60);
l()
    LogicalScene logic = {};
    logic.create(0);

l()
    logic.addNewPlayer();
l()
    logic.addScenery(Scenery(Square, {.SQUARE_half_width=1, .SQUARE_half_height=3}, vec2(0), false));
l()
    // Main loop
    double previous = 0;
    double current = 0;
    while ((!glfwWindowShouldClose(logic.view.render.window.pointer)) && (!glfwGetKey(logic.view.render.window.pointer, GLFW_KEY_ESCAPE))) {
        glfwPollEvents();
        previous = current;
        current = glfwGetTime();

        logic.tick(current - previous);
    }

    // Clean up
    logic.destroy();
    return 0;
}