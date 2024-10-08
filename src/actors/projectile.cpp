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
        bulletFilter.maskBits = to_underlying(ActorType::Projectile|ActorType::Player|ActorType::Scenery);
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
        shape.props.CIRCLE_radius = props.radius;
    return shape;
}

void Projectile::updateTrailData() {
    oldPositions[0] = actor.state.pos;
    // Shift old positions down
    for (int i = oldPositions.size() - 1; i > 0; i--) {
        oldPositions[i] = oldPositions[i - 1];
    }
}

void Projectile::update(PhysicalWorld* world, float dTime){
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

// actually, not really that precise
// leaves gaps
// TODO?
void Projectile::drawTrail(VisualView* view) {
    float numSegments = oldPositions.size();
    glm::vec2 newPos = actor.state.pos;

    for (int i = 0; i < oldPositions.size(); i++) {
        glm::vec2 prevPos = oldPositions[i];

        // skip trail segments for stationary points (no movement)
        if (newPos == prevPos) continue;

        glm::vec2 direction = newPos - prevPos;
        glm::vec2 dirNorm = glm::normalize(direction);
        // if (glm::length(direction) < 0.01f) continue;

        float angle = atan2(dirNorm.x, dirNorm.y);
        // glm::vec2 rotation = glm::vec2(cos(angle), sin(angle));
        // glm::vec2 rotation = glm::vec2(sin(angle), cos(angle));
        // glm::vec2 rotation = glm::vec2(1, 0);

        // sizes are based on the segment index (first segment is wider, last is narrower)
        float bottomHalfSize = props.radius * sqrt(( 1.0f - (float(i+1) / numSegments)));
        float topHalfSize    = props.radius * sqrt(((1.0f - (float(i )) / numSegments)));
        float height = glm::length(direction);

        Shape shape = Shape();
            shape.coloring_info = actor.properties.color;
            shape.pos = (prevPos + newPos) / 2.f;
            shape.rot_angle = angle; //set negative to get unique debugging experience
            shape.shapeType = Trapezoid;
            shape.props.TRAPEZOID_half_bottom_size = bottomHalfSize;
            shape.props.TRAPEZOID_half_top_size = topHalfSize;
            shape.props.TRAPEZOID_half_height = height / 2.0f;   

        view->draw_dynamic_shape(shape, SolidColor);

        // Move to the next segment
        newPos = prevPos;
    }
}

void Projectile::setup(PlayerState* ownerState, PlayerProps* ownerProps, Actor* ownerActor) {
    actor.state.pos = 
        ownerActor->state.pos +
        ownerState->aim_direction * (props.radius + ownerProps->player_radius + 0.1f);
    actor.state.vel = 
        ownerState->aim_direction * 
        float(ownerProps->bullet_velocity);
        // actor.state.vel+=ownerActor->state.vel;
    state.radius = props.radius;
    state.damage = ownerState->damage;
    props.damage = ownerState->damage;
    actor.properties.color = ownerProps->bullet_color;
    state.bounces_left = ownerProps->bullet_bounce_count;

    //to not have a trail to (0,0)
    for(auto& op : oldPositions){
        op = actor.state.pos;
    }
}