#include "physics.hpp"
// Used for collision **logic** for pretty much anything

void PhysicalWorld::setFilterCallback(b2CustomFilterFcn* fn, void* ctx) {b2World_SetCustomFilterCallback(world_id, fn, ctx);}
void PhysicalWorld::setPresolveCallback(b2PreSolveFcn* fn, void* ctx) {b2World_SetPreSolveCallback(world_id, fn, ctx);}
b2BodyEvents PhysicalWorld::GetBodyEvents(void) {return b2World_GetBodyEvents(world_id);}
b2ContactEvents PhysicalWorld::GetContactEvents(void) {return b2World_GetContactEvents(world_id);}
b2SensorEvents PhysicalWorld::GetSensorEvents(void) {return b2World_GetSensorEvents(world_id);}
void PhysicalWorld::step(float dTime) {b2World_Step(world_id, dTime, 4);}

void PhysicalWorld::applyForce(b2BodyId body, const b2Vec2& force) {
    b2Body_ApplyForceToCenter(body, force, true);
}
void PhysicalWorld::applyImpulse(b2BodyId body, const b2Vec2& impulse) {
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
glm::vec2 PhysicalWorld::getVelocity(b2BodyId body) {
    return b2glm(b2Body_GetLinearVelocity(body));
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