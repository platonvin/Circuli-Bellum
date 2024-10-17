#pragma once
#include "visual.hpp"
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
// basically does nothing. 
class PhysicalWorld {
public:
    void setup(b2Vec2 _gravity = {0, -55.}) {
        gravity = _gravity;
        b2WorldDef worldDef = b2DefaultWorldDef();
        world_id = b2CreateWorld(&worldDef);
        b2World_SetGravity(world_id, gravity);
        // b2SetLengthUnitsPerMeter(100);

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
// private:
    b2WorldId world_id;
};

struct PhysicalState {
    glm::vec2 pos = glm::vec2(0);
    // glm::vec2 old_pos = glm::vec2(0);
    glm::vec2 rot = glm::vec2(1,0);
    glm::vec2 vel = glm::vec2(0);
};
struct PhysicalProperties {
    glm::u8vec4 color = twpp::purple(500);
    ShapeType shape_type = Circle;
    b2BodyType body_type = b2_staticBody;
    float friction = 0.3;
    //extra radius applied AFTER every sdf
    float rounding_radius = 0;
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
    // apl(props->body_type)
        bodyDef.type = props->body_type;
        bodyDef.position = glm2b(state->pos);
        bodyDef.linearVelocity = glm2b(state->vel);
        bodyDef.isAwake = true;
        bodyDef.isEnabled = true;
        bodyDef.enableSleep = false;
        bodyDef.sleepThreshold = 0;
        bodyDef.automaticMass = true;

    assert(bind);
    // pl(bind->body.index1)
    // pl(bind->shape.index1)
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
    // pl(bind->body.index1)
    // pl(bind->shape.index1)
    // assert(b2Shape_IsValid(bind->shape));
    // assert(user_data);
    b2Body_SetUserData(bind->body, user_data);
    b2Shape_SetUserData(bind->shape, user_data);
}

// basically for light.
// not float pendulum XD
struct DoublePendulum{
    using ftype = double;
    using vec = dvec2;
    // Angles 
    ftype theta1 = glm::radians(120.0);
    ftype theta2 = glm::radians(100.0);
    // Angular velocities
    ftype omega1 = 0;
    ftype omega2 = 0;  

    const ftype g = 9.82f; // im a bad person
    const ftype length1 = 1.0f;
    const ftype length2 = 1.0f;
    const ftype mass1 = 1.0f;
    const ftype mass2 = 1.0f;
    const ftype conservation = 0.992f;

    // lol did not know it existed
    const ftype epsilon = std::numeric_limits<ftype>::epsilon();

    void simulate(ftype dTime) {
        ftype deltaTheta = theta2 - theta1;
        if (std::fabs(deltaTheta) < epsilon) deltaTheta = (deltaTheta >= 0) ? 10.0 * epsilon : -10.0 * epsilon;

        ftype denominator1 = (mass1 + mass2) * length1 - mass2 * length1 * cos(deltaTheta) * cos(deltaTheta);
        ftype denominator2 = (length2 / length1) * denominator1;

        ftype num1 = -g * (2 * mass1 + mass2) * sin(theta1) - mass2 * g * sin(theta1 - 2 * theta2) - 2 * sin(deltaTheta) * mass2 * (omega2 * omega2 * length2 + omega1 * omega1 * length1 * cos(deltaTheta));
        ftype num2 = 2 * sin(deltaTheta) * (omega1 * omega1 * length1 * (mass1 + mass2) + g * (mass1 + mass2) * cos(theta1) + omega2 * omega2 * length2 * mass2 * cos(deltaTheta));

        ftype alpha1 = num1 / denominator1;
        ftype alpha2 = num2 / denominator2;

        omega1 += alpha1 * dTime;
        omega2 += alpha2 * dTime;

        omega1 *= conservation;
        omega2 *= conservation;

        theta1 += omega1 * dTime;
        theta2 += omega2 * dTime;

        // theta1 = wrap(theta1);
        // theta2 = wrap(theta2);
        if (std::isnan(theta1) || std::isnan(theta2) || std::isnan(omega1) || std::isnan(omega2)) {
            reset_state();
        }
    }

    void reset_state() {
        theta1 = glm::radians(120.0);
        theta2 = glm::radians(100.0);
        omega1 = 0;
        omega2 = 0;
    }

    void apply_impulse(ftype impulse1, ftype impulse2) {
        omega1 += glm::min(impulse1, 50.0);
        omega2 += glm::min(impulse2, 50.0);
    }

    glm::vec2 get_pendulum_position1() {
        return vec(length1 * sin(theta1), -length1 * cos(theta1));
    }

    glm::vec2 get_pendulum_position2() {
        vec pos1 = get_pendulum_position1();
        return pos1 + vec(length2 * sin(theta2), -length2 * cos(theta2));
    }
};

#endif // __PHYSICS_HPP__