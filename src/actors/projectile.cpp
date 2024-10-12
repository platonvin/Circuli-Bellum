#include "projectile.hpp"

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
        state.damage += props.damage * props.extra_damage_per_bounce; 
        return true;
    }
    else {
        //false=destroy
        return false;
    }
}

//same group always collides if group > 0 and never collides if group < 0
void Projectile::addToWorld(PhysicalWorld* world, int group){
    //TODO capsule
    b2Circle bullet_circle = {};
        bullet_circle.radius = props.radius;
    b2Filter bulletFilter = {};
        bulletFilter.categoryBits = to_underlying(ActorType::Projectile);
        bulletFilter.maskBits = to_underlying(ActorType::Projectile|ActorType::Player|ActorType::StaticScenery|ActorType::DynamicScenery|ActorType::Border);
        bulletFilter.groupIndex = group;
    b2ShapeDef bouncy_shape = b2DefaultShapeDef();
        bouncy_shape.restitution = props.bounciness;
        bouncy_shape.friction = 0;
        bouncy_shape.filter = bulletFilter;
    b2BodyDef bullet_bdef = b2DefaultBodyDef();
        bullet_bdef.fixedRotation = true;

        // bouncy_shape.
    // world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &bullet_circle,
    //     nullptr, &bouncy_shape);
    world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &bullet_circle,
        &bullet_bdef, &bouncy_shape);
    // b2Body_SetMassData()
}

Shape Projectile::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        //overwrites
        shape.props.CIRCLE_radius = state.radius;
    return shape;
}

void Projectile::updateTrailData() {
    // update 0'st segment
    trailSegments[0].pos = actor.state.pos;
    vec2 diff = trailSegments[0].pos - trailSegments[1].pos;
    trailSegments[0].len = length(diff);
    trailSegments[0].dir = normalize(diff);
    //TODO? diff?
    trailSegments[0].angle = atan2(trailSegments[0].dir.x, trailSegments[0].dir.y);
    
    // Shift old positions down
    for (int i = trailSegments.size() - 1; i > 0; i--) {
        trailSegments[i] = trailSegments[i - 1];
    }
}

void Projectile::update(PhysicalWorld* world, float dTime){
    // scaling for Grow
    if(props.grow_factor != 0){
        state.damage *= exp(dTime*props.grow_factor);
        state.radius *= exp(dTime*props.grow_factor);
        auto prev = b2Shape_GetCircle(actor.bindings.shape);
        prev.radius = state.radius;
        b2Shape_SetCircle(actor.bindings.shape, &prev);
    }
    time_elapsed += dTime;
    const float TIME_BETWEEN_TRAIL_UPDATES = 0.01;
    if(time_elapsed > TIME_BETWEEN_TRAIL_UPDATES){
        time_elapsed = 0;
        updateTrailData();    
    }
}

void Projectile::draw(VisualView* view){
    view->draw_dynamic_shape(constructShape(), SolidColor);
    drawTrail(view);
}

// actually, not really that precise - leaves gaps
void Projectile::drawTrail(VisualView* view) {
    int numSegments = trailSegments.size();

    // segments are somewhat cached
    // (atan+normalize+length were the bottlneck)
    for (int i = 1; i < numSegments; i++) {
        TrailSegment prevSeg = trailSegments[i-1];
        TrailSegment  curSeg = trailSegments[i];

        // skip trail segments for stationary points (no movement)
        // if (prevSeg.pos == curSeg.pos) continue;

        float bottomHalfSize = state.radius * sqrt(( 1.0f - (float(i+1) / float(numSegments))));
        float topHalfSize    = state.radius * sqrt(((1.0f - (float(i)) / float(numSegments))));

        Shape shape = Shape();
            shape.coloring_info = actor.properties.color;
            shape.pos = (prevSeg.pos + curSeg.pos) / 2.f;
            shape.rot_angle = prevSeg.angle; //set negative to get unique debugging experience
            shape.shapeType = Trapezoid;
            shape.props.TRAPEZOID_half_bottom_size = bottomHalfSize;
            shape.props.TRAPEZOID_half_top_size = topHalfSize;
            shape.props.TRAPEZOID_half_height = prevSeg.len / 2.0f;   
        view->draw_dynamic_shape(shape, SolidColor);
    }
}

void Projectile::setupFromPlayer(PlayerState* ownerState, PlayerProps* ownerProps, Actor* ownerActor) {
    props.damage = ownerProps->bullet_damage;
    props.extra_damage_per_bounce = ownerProps->extra_damage_per_bounce;
    props.bounciness = .9;
    props.radius = ownerProps->bullet_radius;
    props.grow_factor = ownerProps->grow_factor;

    actor.state.pos = 
        ownerActor->state.pos +
        ownerState->aim_direction * (props.radius + ownerProps->radius + 0.1f);
    actor.state.vel = 
        ownerState->aim_direction * 
        float(ownerProps->bullet_speed);
        // actor.state.vel+=ownerActor->state.vel;
    state.radius = props.radius;
    state.damage = ownerState->damage;
    props.damage = ownerState->damage;
    actor.properties.color = ownerProps->bullet_color;
    state.bounces_left = ownerProps->bullet_bounces;

    //to not have a trail to (0,0)
    for(auto& seg : trailSegments){
        seg.pos = actor.state.pos;
    }
}