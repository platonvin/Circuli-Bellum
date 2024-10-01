#include "projectile.hpp"
#include "player.hpp"

bool Projectile::processPlayerHit(PlayerState* player){
    // if(player.blocks){
    //     //bounce
    //     return true;
    // }
    // else {
    //     b2DestroyBody(actor.bindings.body); //TODO
    //     //false=destroy
    //     return false;
    // }
    // b2DestroyBody(actor.bindings.body);
    return false;
}

//return true if bullet survives, false if should be killed
bool Projectile::processSceneryHit(SceneryState* scenery){
    if(state.bounces_left > 0){
        //bounce
        state.bounces_left--;
        return true;
    }
    else {
        //false=destroy
        return false;
    }
}

void Projectile::addToWorld(PhysicalWorld* world){
    //TODO capsule
    b2Circle bullet_circle = {};
        bullet_circle.radius = props.radius;
    b2ShapeDef bouncy_shape = b2DefaultShapeDef();
        bouncy_shape.restitution = props.bounciness;
        bouncy_shape.friction = 0;
    b2BodyDef bullet_bdef = b2DefaultBodyDef();
        bullet_bdef.fixedRotation = true;
        // bouncy_shape.
    // world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &bullet_circle,
    //     nullptr, &bouncy_shape);
    world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &bullet_circle,
        &bullet_bdef, &bouncy_shape);
}

Shape Projectile::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        //overwrites
        shape.props.CIRCLE_radius = props.radius;
    return shape;
}
