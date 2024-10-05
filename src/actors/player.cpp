#include "player.hpp"

void Player::processBulletHit(PhysicalWorld* world, ProjectileState* projectile){
    pl(state.hp_left)
    state.hp_left -= projectile->damage;
    pl(state.hp_left)
    //apply impulse
    // world->applyImpulse(actor.bindings.body, b2Vec2());
    //block?
}


Shape Player::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = props.radius;
        //overwrites
        shape.pos = visual_pos;
    return shape;
}

// for X only
void Player::softLimitVelocity(PhysicalWorld* world) {
    vec2 current_vel = actor.state.vel;
    float current_x_vel = current_vel.x;

    float desired_x_vel = 1.0f;  // desired X velocity
    
    if (fabs(current_x_vel) > desired_x_vel) {
        float excess_x_vel = current_x_vel - glm::sign(current_x_vel) * desired_x_vel;
        
        vec2 force = vec2(-excess_x_vel * 2.0f, 0.0f);
        
        world->applyForce(actor.bindings.body, glm2b(force));
    }
}

void Player::refillJumpsIfNeeded(){
    //like its next frame. TODO: box2d end event on the same tick?
    if(state.refill_jumps_next_frame){
        state.jumps_left = props.max_jumps;
        state.refill_jumps_next_frame = false;
    }
    if(state.touching_grass_counter > 0){
        state.refill_jumps_next_frame = true;
    }
}

void Player::update(PhysicalWorld* world /*for later?*/, vec2 view_pos){
    softLimitVelocity(world);
    refillJumpsIfNeeded();

    // if(actor.state.vel.y > -0.01){
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // } else {
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // }

    state.damage = props.damage;
    // state.
    state.aim_direction = normalize(view_pos - actor.state.pos);
}

void Player::addToWorld(PhysicalWorld* world){
    b2Circle player_circle = {};
        player_circle.radius = props.radius;
    b2ShapeDef shape = b2DefaultShapeDef();
        shape.restitution = 0.1;
        shape.friction = 0.05;
    b2BodyDef bdef = b2DefaultBodyDef();
        bdef.fixedRotation = true;

    world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &player_circle, &bdef, &shape);
}
// void Player::removeFromWorld(PhysicalWorld* world){
// }


void Player::draw(VisualView* view) {
    // pl(int(actor.properties.color.x))
    drawBody(view);
    // drawArmsAndGun(view);
    drawLegs(view);
    // drawBullets(view);
}

void Player::drawBody(VisualView* view) {
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = props.radius;
        //overwrites
        shape.pos = visual_pos;
    view->draw_dynamic_shape(constructShape(), SolidColor);
}

void Player::drawLegs(VisualView* view) {
    const float legLength = props.radius * 0.33f; // Length of each leg
    const float legRardius = props.radius * 0.15f; // width?
    const float legRotationOut = glm::pi<float>()/2.0;
    const float legRotationIn  = glm::pi<float>()/2.0;

    float legDirection = glm::sign(actor.state.vel.x);

    vec2 legOffset = vec2(props.radius, props.radius) / sqrtf(2.0);

    vec2 somewhatInbetweenPos = (visual_pos + actor.state.pos) / 2.f;

    vec2 leg1Pos1 = somewhatInbetweenPos + vec2(-legOffset.x, -legOffset.y); // Start point at the body
    vec2 leg1Pos2 = leg1Pos1 + vec2(0.0, -(legLength + legRardius)); // First segment end (outward)
    vec2 leg1Pos3 = leg1Pos2 + vec2(0.0, -(legLength + legRardius)); // Second segment end (inward)

    vec2 leg2Pos1 = somewhatInbetweenPos + vec2(+legOffset.x, -legOffset.y);
    vec2 leg2Pos2 = leg2Pos1 + vec2(0.0, -(legLength + legRardius));
    vec2 leg2Pos3 = leg2Pos2 + vec2(0.0, -(legLength + legRardius));

    Shape leg1Shape1, leg1Shape2;
        leg1Shape1.coloring_info = actor.properties.color;
        leg1Shape1.pos = leg1Pos1;
        leg1Shape1.shapeType = Capsule;
        leg1Shape1.props.CAPSULE_radius = legRardius; 
        leg1Shape1.props.CAPSULE_half_length = legLength / 2.0f;

        leg1Shape2 = leg1Shape1;
        leg1Shape2.pos = leg1Pos2;

    Shape leg2Shape1, leg2Shape2;
        leg2Shape1 = leg1Shape1;
        leg2Shape1.pos = leg2Pos1;

        leg2Shape2 = leg2Shape1;
        leg2Shape2.pos = leg2Pos2;


    leg1Shape1.rot_angle = -glm::pi<float>() / 2.0;
    leg1Shape2.rot_angle = -glm::pi<float>() / 2.0;
    leg2Shape1.rot_angle = -glm::pi<float>() / 2.0;
    leg2Shape2.rot_angle = -glm::pi<float>() / 2.0;

    float curve_angle = glm::pi<float>()/7.0;
    // if floating
    // TODO: random wobble
    if(state.touching_grass_counter > 0){
        if(legDirection == -1){
            //then all to the left 
            //defenetely first try guessed sign
            leg1Shape1.rot_angle += +curve_angle;
            leg1Shape2.rot_angle += -curve_angle;
            leg2Shape1.rot_angle += +curve_angle;
            leg2Shape2.rot_angle += -curve_angle;    
        }
        if(legDirection == +1){
            //then all to the right 
            leg1Shape1.rot_angle += -curve_angle;
            leg1Shape2.rot_angle += +curve_angle;
            leg2Shape1.rot_angle += -curve_angle;
            leg2Shape2.rot_angle += +curve_angle;    
        }
        if(legDirection == 0){
            //then all outside
            leg1Shape1.rot_angle += +curve_angle;
            leg1Shape2.rot_angle += -curve_angle;
            leg2Shape1.rot_angle += -curve_angle;
            leg2Shape2.rot_angle += +curve_angle;    
        }
    } else {
        //increase length!
        leg1Shape2.props.CAPSULE_half_length *= 1.6;
        leg2Shape2.props.CAPSULE_half_length *= 1.6;
    }
    view->draw_dynamic_shape(leg1Shape1, SolidColor);
    view->draw_dynamic_shape(leg1Shape2, SolidColor);
    view->draw_dynamic_shape(leg2Shape1, SolidColor);
    view->draw_dynamic_shape(leg2Shape2, SolidColor);
}

void Player::drawArmsAndGun(VisualView* view) {

}

void Player::drawAmmo(VisualView* view) {

}


void Player::drawCard(Card card) {
    props.hp *= card.hp_buff;
    props.mass = 1.0f;
    // props.radius *= ???? TODO compute from mass;
    props.bullet_radius *= card.bullet_radius_buff;
    props.jump_impulse *= card.jump_buff;
    props.max_jumps += card.extra_jumps;
    props.damage *= card.damage_buff;
    props.reload_time *= card.reload_time_buff; //between closest shots from separate magazines
    props.bullet_feed_time *= card.shoot_time_buff; //between shots from same magazine
    props.block_delay *= card.block_delay_buff;
    props.max_lives += card.lives_buff;
    props.ammunition_count += card.ammunition_buff;
    props.bullet_speed *= card.bullet_speed_buff;
    if(card.set_bullet_color){
        props.bullet_color = card.bullet_color_set;
    }
    props.bullet_bounces += card.bullet_bounce_buff;
    props.knockback *= card.knockback_buff;

    //constrains
    props.bullet_radius = glm::clamp(props.bullet_radius, 0.01f, 2.f);
}

// damper for better visuals. Breaks on high frame times. Causes AaAaAa in the beginning
void Player::updateVisualPos(float dTime) {
    vec2 target_pos = actor.state.pos;
    vec2 displacement = target_pos - visual_pos;

    vec2 springForce = springConstant * displacement;
    vec2 dampingForce = -dampingFactor * visual_vel;
    vec2 totalForce = springForce + dampingForce;

    visual_vel += totalForce * dTime;
    visual_pos += visual_vel * dTime;
}