#define GLFW_INCLUDE_NONE
#define GLM_FORCE_SWIZZLE
#include <cassert>
#include <glm/glm.hpp>
#include "logic.hpp"

int main() {
l()
    LogicalScene scene = {};
    scene.setup(1);

l()
    scene.addNewPlayer();
l()
    scene.addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=8.6, .RECTANGLE_half_height=.25}, vec2(0,-6), false)); // Ground
    scene.addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=3.3, .RECTANGLE_half_height=.25}, vec2(0,+1), false)); // Mid high platform
    scene.addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=2.0, .RECTANGLE_half_height=.25}, vec2(0,-4.0), false)); // Mid low
    scene.addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=1.5, .RECTANGLE_half_height=.5}, vec2(-4,-2.3), false)); // Left lower platform
    scene.addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=1.5, .RECTANGLE_half_height=.5}, vec2(+4,-2.3), false)); // Right lower platform
    scene.addScenery(Scenery(Circle, {.CIRCLE_radius=0.8}, vec2(-5,2.8), false));
    scene.addScenery(Scenery(Circle, {.CIRCLE_radius=0.8}, vec2(+5,2.8), false));
    scene.addScenery(Scenery(Circle, {.CIRCLE_radius=2.0}, vec2(-8,0), false));
    scene.addScenery(Scenery(Circle, {.CIRCLE_radius=2.0}, vec2(+8,0), false));
    scene.addScenery(Scenery(Capsule, {.CAPSULE_radius=0.5, .CAPSULE_half_length=2.0}, vec2(0,+4), false));
    

l()
    double previous = 0;
    double current = 0;
    while ((!glfwWindowShouldClose(scene.view.render.window.pointer)) && (!glfwGetKey(scene.view.render.window.pointer, GLFW_KEY_ESCAPE))) {
        glfwPollEvents();
        current = glfwGetTime();
        // if((current - previous) > 0.005){
            scene.tick(current - previous);
            previous = current;
        // }
    }

    scene.cleanup();
    return 0;
}