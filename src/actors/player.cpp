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
        //overwrites
        shape.props.CIRCLE_radius = props.radius;
    // pl(shape.pos.x);
    // pl(shape.pos.y);
    // pl(shape.props.CIRCLE_radius);
    return shape;
}

void Player::softLimitVelocity(PhysicalWorld* world){
    vec2 current_vel = world->getVelocity(actor.bindings.body);
    vec2 desired_vel = vec2(1);
    float current_len = glm::length(current_vel);
    float desired_len = glm::length(desired_vel);
    if(current_len > desired_len){
        vec2 force_direction = -glm::normalize(current_vel);
        float ldiff = (current_len - desired_len);
        world->applyForce(actor.bindings.body, glm2b(force_direction*ldiff*2.0f));
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

void Player::draw(VisualView* view){
    view->draw_dynamic_shape(constructShape(), FBMstyle);
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
}