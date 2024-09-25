#pragma once

#include <cassert>
#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <box2d/box2d.h>

inline glm::vec2 b2glm(b2Vec2 v){return {v.x, v.y};}

using filterFn = std::function<bool(b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context)>;
using preSolveFn = std::function<bool(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context)>;
using std::tie;

inline bool operator==(b2ShapeId a, b2ShapeId b){
    return (a.index1 == b.index1) &&
        //    (a.revision == b.revision) &&
           (a.world0 == b.world0);
}

template <typename T> bool pair_eq_unorder(std::pair<T,T> a, std::pair<T,T> b){
    return ((a.first == b.first) && (a.second == b.second)) ||
           ((a.first == b.second) && (a.second == b.first));
}

enum ActorType {
    AT_Projectile = 1 << 0,
    AT_Player     = 1 << 1,
    AT_Scenery    = 1 << 2,
};

class Player {
private:
    const ActorType actorType = AT_Player;
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
    b2BodyId body;
    b2ShapeId shape;
    b2ShapeId sensor;
};

class Scenery {
private:
    const ActorType actorType = AT_Scenery;
public:
    void create() {
    }

    void destroy()  {
    }

    //health left
    //deadly?
    //
    //bound to box2d object
    b2BodyId body;
    b2ShapeId shape;
    b2ShapeId sensor;
};

class Projectile {
private:
    const ActorType actorType = AT_Projectile;
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
    b2BodyId body;
    b2ShapeId shape;
    b2ShapeId sensor;
};

// Thin wrapper around box2d
class PhysicalWorld {
public:
    void create(b2Vec2 gravity) {
        b2WorldDef worldDef = b2DefaultWorldDef();
        world_id = b2CreateWorld(&worldDef);
        b2World_SetGravity(world_id, gravity);
    }
    void destroy() {
        b2DestroyWorld(world_id);
    }

    // Adding different shapes automatically from params
public:
    std::tuple<b2BodyId, b2ShapeId> addCircle(float x, float y, float radius, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    std::tuple<b2BodyId, b2ShapeId> addPolygon(float x, float y, const std::vector<b2Vec2>& vertices, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    std::tuple<b2BodyId, b2ShapeId> addCapsule(float x, float y, b2Vec2 center1, b2Vec2 center2, float radius, float density = 1.0f, float friction = 0.3f, b2BodyType type = b2BodyType::b2_dynamicBody);
    // Wrapping box2D functions
    void applyForce(b2BodyId body, const b2Vec2& force, const b2Vec2& point);
    void applyImpulse(b2BodyId body, const b2Vec2& impulse, const b2Vec2& point);
    void setMass(b2BodyId body, float mass);
    void setVelocity(b2BodyId body, const b2Vec2& velocity);
    void castRay(const b2RayCastInput& input, b2CastOutput& output);
    void shapeCast(const b2ShapeCastInput& input, b2CastOutput& output);

    void setFilterCallback(b2CustomFilterFcn* fn, void* ctx);
    void setPresolveCallback(b2PreSolveFcn* fn, void* ctx);
    void step(float dTime);
    
    b2BodyEvents GetBodyEvents(void);
    b2ContactEvents GetContactEvents(void);
    b2SensorEvents GetSensorEvents(void);
private:
    b2WorldId world_id;
};