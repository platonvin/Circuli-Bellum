#include <cassert>
#include <glm/glm.hpp>
using namespace glm;

// #include "GLFW/glfw3.h"
// #include "macros.hpp"
#define GLFW_INCLUDE_NONE
#include "logic.hpp"

// #define let auto&

int main() {
l()
    LogicalScene logic = {};
    logic.setup(1);

l()
    logic.addNewPlayer();
l()
    logic.addScenery(Scenery(Square, {.SQUARE_half_width=1, .SQUARE_half_height=3}, vec2(-3,0), false));

    logic.addScenery(Scenery(Square, {.SQUARE_half_width=9, .SQUARE_half_height=1}, vec2(2,-1), false));
l()
    // Main loop
    double previous = 0;
    double current = 0;
    while ((!glfwWindowShouldClose(logic.view.render.window.pointer)) && (!glfwGetKey(logic.view.render.window.pointer, GLFW_KEY_ESCAPE))) {
        glfwPollEvents();
        current = glfwGetTime();
        if((current - previous) > 0.005){

            logic.tick(current - previous);
            
            previous = current;
        }
    }

    // Clean up
    logic.cleanup();
    return 0;
}