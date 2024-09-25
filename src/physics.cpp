#include "physics.hpp"
// Used for collision **logic** for pretty much anything
void PhysicalWorld::setFilterCallback(b2CustomFilterFcn* fn, void* ctx) {b2World_SetCustomFilterCallback(world_id, fn, ctx);}
void PhysicalWorld::setPresolveCallback(b2PreSolveFcn* fn, void* ctx) {b2World_SetPreSolveCallback(world_id, fn, ctx);}
b2BodyEvents PhysicalWorld::GetBodyEvents(void) {return b2World_GetBodyEvents(world_id);}
b2ContactEvents PhysicalWorld::GetContactEvents(void) {return b2World_GetContactEvents(world_id);}
b2SensorEvents PhysicalWorld::GetSensorEvents(void) {return b2World_GetSensorEvents(world_id);}
void PhysicalWorld::step(float dTime) {b2World_Step(world_id, dTime, 4);}

std::tuple<b2BodyId, b2ShapeId> PhysicalWorld::addCircle(float x, float y, float radius, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = type;
        bodyDef.position = {x, y};

    b2Circle circleShape = {};
        circleShape.radius = radius;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2ShapeId shape_id = b2CreateCircleShape(body_id, &shapeDef, &circleShape);

    return {body_id, shape_id};
}

std::tuple<b2BodyId, b2ShapeId> PhysicalWorld::addPolygon(float x, float y, const std::vector<b2Vec2>& vertices, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = type;
        bodyDef.position = {x, y};
    b2Hull hull = b2ComputeHull(vertices.data(), vertices.size());
    assert(hull.count > 0);
    b2Polygon polyShape = b2MakePolygon(&hull, 1) ;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2ShapeId shape_id = b2CreatePolygonShape(body_id, &shapeDef, &polyShape);

    return {body_id, shape_id};
}

std::tuple<b2BodyId, b2ShapeId> PhysicalWorld::addCapsule(float x, float y, b2Vec2 center1, b2Vec2 center2, float radius, float density, float friction, b2BodyType type) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = type;
        bodyDef.position = {x, y};

    b2Capsule capsuleShape = {};
        capsuleShape.center1 = center1;
        capsuleShape.center2 = center2;
        capsuleShape.radius = radius;

    b2BodyId body_id = b2CreateBody(world_id, &bodyDef);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2ShapeId shape_id = b2CreateCapsuleShape(body_id, &shapeDef, &capsuleShape);

    return {body_id, shape_id};
}

    // b2BodyDef bodyDef = b2DefaultBodyDef();
    //     bodyDef.type = type;
        
    //     bodyDef.position = {x, y};
    // // b2Hull hull = b2ComputeHull(vertices.data(), vertices.size());
    // // assert(hull.count > 0);
    // b2Polygon polyShape = b2MakeRoundedBox(float hx, float hy, float radius);

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