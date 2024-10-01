#pragma once
#ifndef __PHYSICS_HPP__
#define __PHYSICS_HPP__

#define TWPP_NO_EXCEPTIONS
#include "twpp.hpp"
#include <cassert>
// #include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <box2d/box2d.h>

#include "macros.hpp"

inline glm::vec2 b2glm(b2Vec2 v){return {v.x, v.y};}
inline glm::vec2 b2glm(b2Rot  v){return {v.c, v.s};}
inline b2Vec2 glm2b(glm::vec2 v){return {v.x, v.y};}

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

//fwd declarations
struct PhysicalState; 
struct PhysicalProperties; 
struct PhysicalBindings;

using glm::vec2;

// Thin wrapper around box2d
class PhysicalWorld {
public:
    void setup(b2Vec2 _gravity = {0, -17.}) {
        gravity = _gravity;
        b2WorldDef worldDef = b2DefaultWorldDef();
        world_id = b2CreateWorld(&worldDef);
        b2World_SetGravity(world_id, gravity);
    }
    void cleanup() {
        b2DestroyWorld(world_id);
    }

    // Adding different shapes automatically from params
    // Wrapping box2D functions
    template <typename b2shapeTypeTemplate, b2ShapeId Fun(b2BodyId, const b2ShapeDef*, const b2shapeTypeTemplate*)>
    void addActor (PhysicalBindings* bind, PhysicalState* state, PhysicalProperties* props, void* user_data, b2shapeTypeTemplate* shape, b2BodyDef* bdef = nullptr, b2ShapeDef* sdef = nullptr);

    void applyForce(b2BodyId body, const b2Vec2& force);
    void applyImpulse(b2BodyId body, const b2Vec2& impulse);
    void setMass(b2BodyId body, float mass);
    void setVelocity(b2BodyId body, const b2Vec2& velocity);
    vec2 getVelocity(b2BodyId body);
    void castRay(const b2RayCastInput& input, b2CastOutput& output);
    void shapeCast(const b2ShapeCastInput& input, b2CastOutput& output);

    void setFilterCallback(b2CustomFilterFcn* fn, void* ctx);
    void setPresolveCallback(b2PreSolveFcn* fn, void* ctx);
    void step(float dTime);

    b2Vec2 gravity;
    
    b2BodyEvents GetBodyEvents(void);
    b2ContactEvents GetContactEvents(void);
    b2SensorEvents GetSensorEvents(void);
private:
    b2WorldId world_id;
};

struct PhysicalState {
    glm::vec2 pos = glm::vec2(0);
    glm::vec2 rot = glm::vec2(0);
    glm::vec2 vel = glm::vec2(0);
};
struct PhysicalProperties {
    glm::u8vec3 color = twpp::purple(500);
    float friction = 0.3;
    b2BodyType body_type = b2_staticBody;
    // not aligned with view ShapeType. They are just different
    b2ShapeType shape_type = b2_circleShape;
};
struct PhysicalBindings {
    b2BodyId body = {};
    b2ShapeId shape = {};
    b2ShapeId sensor = {};
};

template <typename b2shapeTypeTemplate, b2ShapeId Fun(b2BodyId, const b2ShapeDef*, const b2shapeTypeTemplate*)>
void PhysicalWorld::addActor (PhysicalBindings* bind, PhysicalState* state, PhysicalProperties* props, void* user_data, b2shapeTypeTemplate* shape, b2BodyDef* bdef, b2ShapeDef* sdef){
    b2BodyDef bodyDef;
    if(bdef == nullptr){
        bodyDef = b2DefaultBodyDef();
    } else {
        bodyDef = *bdef;
    }
    apl(props->body_type)
    bodyDef.type = props->body_type;
    bodyDef.position = glm2b(state->pos);
    bodyDef.linearVelocity = glm2b(state->vel);
    bodyDef.isAwake = true;
    bodyDef.isEnabled = true;
    bodyDef.enableSleep = false;
    bodyDef.sleepThreshold = 0;

    assert(bind);
    pl(bind->body.index1)
    pl(bind->shape.index1)
    assert(b2World_IsValid(world_id));
    bind->body = b2CreateBody(world_id, &bodyDef);
    // if(sdef == nullptr){
    // }
    b2ShapeDef shapeDef;
    if(sdef == nullptr){
        shapeDef = b2DefaultShapeDef();
    } else {
        shapeDef = *sdef;
    }
    shapeDef.enableContactEvents = true;
    shapeDef.density = 1;

    bind->shape = Fun(bind->body, &shapeDef, shape);
    assert(b2Shape_IsValid(bind->shape));
    if(!b2Body_IsValid(bind->body)){

    }
    assert(b2Body_IsValid(bind->body));
    pl(bind->body.index1)
    pl(bind->shape.index1)
    // assert(b2Shape_IsValid(bind->shape));
    assert(user_data);
    b2Body_SetUserData(bind->body, user_data);
    b2Shape_SetUserData(bind->shape, user_data);
}

#endif // __PHYSICS_HPP__