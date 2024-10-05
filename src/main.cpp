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
    LogicalScene scene = {};
    scene.setup(1);

l()
    scene.addNewPlayer();
l()
    // logic.addScenery(Scenery(Trapezoid, {.TRAPEZOID_half_bottom_size=5, .TRAPEZOID_half_top_size=2, .TRAPEZOID_half_height=2.5}, vec2(0,-3), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=8.6, .SQUARE_half_height=.25}, vec2(0,-6), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=3.3, .SQUARE_half_height=.25}, vec2(0,+1), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=1.5, .SQUARE_half_height=.5}, vec2(-4,-1.3), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=1.5, .SQUARE_half_height=.5}, vec2(+4,-1.3), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=1.5, .SQUARE_half_height=.5}, vec2(-6,-3.7), false));
    scene.addScenery(Scenery(Square, {.SQUARE_half_width=1.5, .SQUARE_half_height=.5}, vec2(+6,-3.7), false));
    // logic.addScenery(Scenery(Trapezoid, {.TRAPEZOID_half_bottom_size=2, .TRAPEZOID_half_top_size=5, .TRAPEZOID_half_height=2.5,}, vec2(0,-3), false));
    // logic.addScenery(Scenery(Square, {.TRAPEZOID_half_bottom_size=5, .TRAPEZOID_half_top_size=2, .TRAPEZOID_half_height=2.5,}, vec2(0,-3), false));
    // logic.genRndScenery();

    // logic.addScenery(Scenery(Square, {.SQUARE_half_width=9, .SQUARE_half_height=1}, vec2(2,-1), false));
l()
    // Main loop
    double previous = 0;
    double current = 0;
    while ((!glfwWindowShouldClose(scene.view.render.window.pointer)) && (!glfwGetKey(scene.view.render.window.pointer, GLFW_KEY_ESCAPE))) {
        glfwPollEvents();
        current = glfwGetTime();
        if((current - previous) > 0.002){

            scene.tick(current - previous);
            
            previous = current;
        }
    }

    // Clean up
    scene.cleanup();
    return 0;
}