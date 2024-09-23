#include "../box2d/include/box2d/box2d.h"
#include "raylib.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <functional>

#define l() std::cout << __FILE__ <<":" << __LINE__ << " " << __FUNCTION__ << "() " "\n";
#define pl(x) std::cout << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";
#define apl(x) std::cout << ":" << __LINE__ << " " << __FUNCTION__ << "() " #x " " << x << "\n";

#define l()
#define pl(x)
#define apl(x)

// Entity that lives in the world
using CallbackFunction = std::function<void(b2BodyId)>;

class WEntity {
public:
    WEntity() {};
    ~WEntity() {};

    // Its so much easier not to use C++ in C++
    void create(b2WorldId worldId, const b2BodyDef& bodyDef, const b2ShapeDef& shapeDef, CallbackFunction callback = nullptr){
        callback_fun = callback;
        l()
        body_id = b2CreateBody(worldId, &bodyDef);
        pl(body_id.index1)
        pl(body_id.world0)
        pl(body_id.revision)
        l()
        b2Circle c = {.center = {0}, .radius = 4.2}; 
        shape_id = b2CreateCircleShape(body_id, &shapeDef, &c); // Use appropriate shape creation
        l()
    }

    void destroy() {
l()
        if (B2_IS_NON_NULL(body_id)) {
            b2DestroyBody(body_id);
            body_id = b2_nullBodyId;
        }
l()
    }

    void update() {
l()
        if (callback_fun) {
l()
            callback_fun(body_id);
        }
l()
    }

    // box2d physics body
    b2BodyId getBodyId() const {
        return body_id;
    }

private:
    // b2WorldId m_worldId;
    b2BodyId body_id;
    b2ShapeId shape_id;
    CallbackFunction callback_fun = nullptr;
};

class World {
public:
    World() {}
    ~World() {}
    void create(){
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldId = b2CreateWorld(&worldDef);
    }

    void destroy() {
        for (WEntity entity : entities) {
            entity.~WEntity();
        }
        b2DestroyWorld(worldId);
    }

    void addEntity(b2BodyDef bodyDef, b2ShapeDef shapeDef, CallbackFunction callback = nullptr) {
l()
        WEntity ent = {};
            ent.create(worldId, bodyDef, shapeDef, callback);
l()     
        entities.push_back(ent);
    }

    void tick(float timeStep) {
l()
        b2World_Step(worldId, timeStep, 4);
l()
        for (WEntity& entity : entities) {
l()
            entity.update();
l()
        }
l()
    }
    b2WorldId getId() {return worldId;}
private:
    b2WorldId worldId;
    std::vector<WEntity> entities = {};
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    // SetTraceLogLevel(LOG_WARNING);
    InitWindow(screenWidth, screenHeight, "Box2D - Physics Simulation");
    SetTargetFPS(60);
l()
    World world = {};
    world.create();

    b2World_SetGravity(world.getId(), {213.2, -9.81});
    // b2body
l()
    // Example callback function for entity updates
    auto callback = [](b2BodyId bodyId) {
        // pl(bodyId.index1)
        // pl(bodyId.world0)
        // pl(bodyId.revision)
        assert(b2Body_IsValid(bodyId));
        b2Vec2 position = b2Body_GetPosition(bodyId);
        std::cout << "Entity at position: (" << position.x << ", " << position.y << ")\n";
    };
l()

    // Create an entity and add it to the world
    auto bdef = b2DefaultBodyDef();
    bdef.type = b2BodyType::b2_dynamicBody;
    world.addEntity(bdef, b2DefaultShapeDef(), callback);
l()

    // Main loop
    while (!WindowShouldClose()) {
        ClearBackground(RAYWHITE);
l()
        BeginDrawing();
l()

        // Simulate physics
l()
        world.tick(0.016);
l()

        DrawFPS(10, 10);
l()
        EndDrawing();
l()
    }

    CloseWindow();

    // Clean up
    world.destroy();
    return 0;
}

/*
player body:
    create / destroy
    controlled from outside via forces
    can jump and climp on walls
    box2d dynamic
    is b2Circle

scenery:
    create / destroy
    box2d static
    is given shape

projectiles:
    create / destroy
    box2d dynamic
    provided callback for:
        projectile hits player
        projectile hits scenery
        projectile hits projectile
    optional function on update()
        (for implementation of custom things like magnet, bullets aiming players, etc)
    is b2Capsule

physical world:
    create / destroy
    contains players, scenery and projectiles
    tick(float dTime):
        calles b2World_Step

*/