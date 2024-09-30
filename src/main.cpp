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
        current = glfwGetTime();
        if((current - previous) > 0.01){

            logic.tick(current - previous);
            
            previous = current;
        }
    }

    // Clean up
    logic.destroy();
    return 0;
}