#include "../box2d/include/box2d/box2d.h"
#include <cassert>
#include <vector>
#include <functional>

using filterFn = std::function<bool(b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context)>;
using preSolveFn = std::function<bool(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context)>;

class Player {
public:
    void create() {
    }

    void destroy()  {
    }

    //stats
    //vector<> cards
    //bullet refs
    //

    // Bound to box2d object
    b2BodyId physical_body;
};

class Scenery {
public:
    void create() {
    }

    void destroy()  {
    }

    //health left
    //deadly?
    //
    //bound to box2d object
    b2BodyId physical_body;
};

class Projectile {
public:
public:
    void create() {
    }

    void destroy()  {
    }

    //stats
    //origin player
    //state: bounces, size...
    //bound to box2d object
    //box2d 
    b2BodyId physical_body;
};


// Thin wrapper around box2d
class PhysicalWorld {
public:
    void create(b2Vec2 gravity) {
        b2WorldDef worldDef = b2DefaultWorldDef();
        world_id = b2CreateWorld(&worldDef);
    }
    void destroy() {
        b2DestroyWorld(world_id);
    }

    // Adding different shapes automatically from params
public:
    b2BodyId addCircle(float x, float y, float radius, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    b2BodyId addPolygon(float x, float y, const std::vector<b2Vec2>& vertices, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    b2BodyId addCapsule(float x, float y, b2Vec2 center1, b2Vec2 center2, float radius, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    // Wrapping box2D functions
    void applyForce(b2BodyId body, const b2Vec2& force, const b2Vec2& point);
    void applyImpulse(b2BodyId body, const b2Vec2& impulse, const b2Vec2& point);
    void setMass(b2BodyId body, float mass);
    void setVelocity(b2BodyId body, const b2Vec2& velocity);
    void castRay(const b2RayCastInput& input, b2CastOutput& output);
    void shapeCast(const b2ShapeCastInput& input, b2CastOutput& output);

    void setFilterCallback(b2CustomFilterFcn* fn, void* ctx);
    void setPresolveCallback(b2PreSolveFcn* fn, void* ctx);
    
    b2BodyEvents GetBodyEvents();
private:
    b2WorldId world_id;
};
// Used for collision **logic** for pretty much anything
void PhysicalWorld::setFilterCallback(b2CustomFilterFcn* fn, void* ctx) {b2World_SetCustomFilterCallback(world_id, fn, ctx);}
void PhysicalWorld::setPresolveCallback(b2PreSolveFcn* fn, void* ctx) {b2World_SetPreSolveCallback(world_id, fn, ctx);}
b2BodyEvents PhysicalWorld::GetBodyEvents() {return b2World_GetBodyEvents(world_id);}

b2BodyId PhysicalWorld::addCircle(float x, float y, float radius, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = {};
        bodyDef.type = type;
        bodyDef.position = {x, y};

    b2Circle circleShape = {};
        circleShape.radius = radius;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = {};
    b2ShapeId shape_id = b2CreateCircleShape(body_id, &shapeDef, &circleShape);

    return body_id;
}

b2BodyId PhysicalWorld::addPolygon(float x, float y, const std::vector<b2Vec2>& vertices, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = {};
        bodyDef.type = type;
        bodyDef.position = {x, y};
    b2Hull hull = b2ComputeHull(vertices.data(), vertices.size());
    b2Polygon polyShape = b2MakePolygon(&hull, 1) ;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = {};
    b2ShapeId shape_id = b2CreatePolygonShape(body_id, &shapeDef, &polyShape);

    return body_id;
}

b2BodyId PhysicalWorld::addCapsule(float x, float y, b2Vec2 center1, b2Vec2 center2, float radius, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = {};
        bodyDef.type = type;
        bodyDef.position = {x, y};

    b2Capsule capsuleShape = {};
        capsuleShape.center1 = center1;
        capsuleShape.center2 = center2;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = {};
    b2ShapeId shape_id = b2CreateCapsuleShape(body_id, &shapeDef, &capsuleShape);

    return body_id;
}

void PhysicalWorld::applyForce(b2BodyId body, const b2Vec2& force, const b2Vec2& point) {
    b2Body_ApplyForce(body, force, point, true);
}
void PhysicalWorld::applyImpulse(b2BodyId body, const b2Vec2& impulse, const b2Vec2& point) {
    b2Body_ApplyLinearImpulseToCenter(body, impulse, true);
}
void PhysicalWorld::setMass(b2BodyId body, float mass) {
    b2MassData massData = {};
        massData.mass = mass;
    b2Body_SetMassData(body, massData);
}
void PhysicalWorld::setVelocity(b2BodyId body, const b2Vec2& velocity) {
    b2Body_SetLinearVelocity(body, velocity);
}
void PhysicalWorld::castRay(const b2RayCastInput& input, b2CastOutput& output) {
    // b2CastResultFcn( b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context )
    // b2World_CastRay(world_id, input.origin, input.translation, {.categoryBits = 0}, [&](b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction) -> float {
    //     output.point = point;
    //     output.normal = normal;
    //     output.fraction = fraction;
    //     output.hit = true;
    //     return fraction;
    // }, nullptr);
    b2World_CastRayClosest(world_id, input.origin, input.translation, b2DefaultQueryFilter());
}

// Class with "Actual" logic
class LogicalScene {
    PhysicalWorld world;
    std::vector<Player*> players;
    std::vector<Scenery*> scenery;
    std::vector<Projectile*> projectiles;

    // std::vector<hit> hits
    // 
    void create();
    void destroy();

    b2Vec2 gravity = {0, -9.81};

    void addPlayer(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    // void addSceneryBlock(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    void addSceneryCircle(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    // void addSceneryTriangle(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    void addProjectileCircle(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    // void addProjectileCapsule(const b2Vec2& position, float radius, float mass, std::function<void(Player&)> callback, void* context);
    void clearWorld(void);
};

void LogicalScene::addPlayer(const b2Vec2& position, float radius, float mass, 
               std::function<void(Player&)> callback, void* context) {
    Player* player = new Player();
    // Players are round :)
    player->create();
    player->physical_body = world.addCircle(position.x, position.y, radius);
    players.push_back(player);
    // Back pointer
    b2Body_SetUserData(player->physical_body, player);
}

void LogicalScene::clearWorld(void) {
    //TODO destroy ents?
    world.destroy();
    world.create(gravity);
}

bool CustomFilterFcn( b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context ) {return true;}
bool CustomPreSolveFcn( b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context ){return true;}
void LogicalScene::create(void) {
    world.create(gravity);

    // TODO: do i need it?
    // Logical scene is passed, but could also be global var.
    // world.setFilterCallback(&CustomFilterFcn, this);
    // world.setPresolveCallback(&CustomPreSolveFcn, this);
}

void LogicalScene::destroy(void) {
    //TODO check b2
    world.destroy();
}