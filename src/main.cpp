#include <cassert>
#include <glm/glm.hpp>
using namespace glm;

#include "logic.hpp"

#define l() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
#define pl(x) std::cout << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
#define apl(x) std::cout << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";

#define l()
// #define pl(x)
// #define apl(x)

#define let auto

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    vec2 ssize = vec2(screenWidth, screenHeight);

    // SetTraceLogLevel(LOG_WARNING);
    // InitWindow(screenWidth, screenHeight, "Box2D - Physics Simulation");
    // SetTargetFPS(60);
l()
    LogicalScene logic = {};
    logic.create();

l()
    logic.addPlayer({0.03,100.123}, 10, 100);
    logic.addScenery({0.01,0.02}, 10, 100);
    // Main loop

    while ((!glfwWindowShouldClose(logic.view.render.window.pointer)) && (!glfwGetKey(logic.view.render.window.pointer, GLFW_KEY_ESCAPE))) {
        glfwPollEvents();
        // ClearBackground(RAYWHITE);
l()
        // BeginDrawing();
l()

        // Simulate physics
l()
        logic.tick(0.016);
        // DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
l()     
        for(let s: logic.sceneries){
            glm::vec2 v = b2glm(b2Body_GetPosition(s->body)) + ssize/2.f;
            // DrawRectangle(v.x, v.y, 10, 10, BEIGE);
        }
        for(let p: logic.players){
            glm::vec2 v = b2glm(b2Body_GetPosition(p->body)) + ssize/2.f;
            // DrawCircle(v.x, v.y, 1.0, MAGENTA);
        }
        
        // DrawFPS(10, 10);
l()
        // EndDrawing();
l()
    }

    // CloseWindow();

    // Clean up
    logic.destroy();
    return 0;
}