#include "player.hpp"

#define DRAW_IF_DEBUG(_shape) if(debugView) debugView->draw_dynamic_shape(_shape, SolidColor)

void Player::processBulletHit(PhysicalWorld* world, ProjectileState* projectile){
    pl(state.hp_left)
    state.hp_left -= projectile->damage;
    pl(state.hp_left)

    if(state.hp_left < 0){
       // death anim? TODO: 
        state.lives_left--;
    }
    if(state.lives_left < 0){
       // final death? TODO?
    }
    //apply impulse
    // world->applyImpulse(actor.bindings.body, b2Vec2());
    //block?
}


Shape Player::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = props.player_radius;
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
    if(state.refill_body_jumps_next_frame){
        state.jumps_left = props.max_jumps;
        state.refill_body_jumps_next_frame = false;
    }
    // if(state.refill_leg_jumps_next_frame){
    //     state.jumps_left = props.max_jumps;
    //     state.refill_leg_jumps_next_frame = false;
    // }
    if(state.touching_grass_counter > 0){
        state.refill_body_jumps_next_frame = true;
        // state.refill_leg_jumps_next_frame = true;
    }
    if(state.touching_grass_counter < 0) state.touching_grass_counter = 0;
    // pl(state.refill_body_jumps_next_frame)
    // pl(state.refill_leg_jumps_next_frame)
    // pl(state.touching_grass_counter)
}

void Player::updateGun(float dTime) {
    state.time_since_last_reload += dTime;
    state.time_since_last_shot += dTime;

    if(state.time_since_last_shot > props.reload_duration){
        state.ammunition_left = props.bullets_per_magazine;
    }
}

void Player::update(PhysicalWorld* world, float dTime /*for later?*/, vec2 view_pos){
    softLimitVelocity(world);
    refillJumpsIfNeeded();
    updateGun(dTime);

    updateLegPositions(world);
    // if(actor.state.vel.y > -0.01){
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // } else {
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // }

    state.damage = props.bullet_damage;
    // state.
    state.aim_direction = normalize(view_pos - actor.state.pos);
}

void Player::addToWorld(PhysicalWorld* world){
    b2Circle playerBodyCircle = {};
        playerBodyCircle.radius = props.bullet_size;
    b2Filter playerFilter = {};
        playerFilter.categoryBits = to_underlying(ActorType::Player);
        playerFilter.maskBits = to_underlying(ActorType::Projectile|ActorType::Player|ActorType::Scenery|ActorType::PlayerLeg);
        // playerFilter.groupIndex = -abs((long long int)this % 666); // so never collides with own leg
    b2ShapeDef playerBodyShape = b2DefaultShapeDef();
        playerBodyShape.restitution = 0.1;
        playerBodyShape.friction = 0.05;
        playerBodyShape.filter = playerFilter;
    b2BodyDef bdef = b2DefaultBodyDef();
        bdef.fixedRotation = true;

    world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &playerBodyCircle, &bdef, &playerBodyShape);

    b2Circle playerLegCircle = {};
        playerLegCircle.radius = 0.1;
    b2Filter playerLegFilter = {};
        playerLegFilter.categoryBits = to_underlying(ActorType::PlayerLeg);
        playerLegFilter.maskBits = to_underlying(ActorType::Player|ActorType::Scenery);
        // playerLegFilter.groupIndex = -abs((long long int)this % 666); // so never collides with own body
    b2ShapeDef playerLegShape = b2DefaultShapeDef();
        playerLegShape.restitution = 0.05;
        playerLegShape.friction = 0.05;
        playerLegShape.filter = playerLegFilter;
        playerLegShape.density = 0;
    bdef.position.y -= props.player_radius;
    bdef.automaticMass = true;
    world->addActor<b2Circle, b2CreateCircleShape>(&leg.bindings, &actor.state, &actor.properties, &leg, &playerLegCircle, &bdef, &playerLegShape);

    // b2WeldJointDef jointDef = b2DefaultWeldJointDef();
    //     jointDef.bodyIdA = actor.bindings.body;
    //     jointDef.bodyIdB = leg.body;
    //     jointDef.collideConnected = true;
    //     jointDef.linearHertz = 1;
    //     jointDef.linearDampingRatio = 1;
    //     jointDef.angularHertz = 1;
    //     jointDef.angularDampingRatio = 1;
    //     jointDef.localAnchorA = {0, +0};
    //     pl(props.player_radius)
    //     jointDef.localAnchorB = {0, 0};
    //     jointDef.referenceAngle = 0;
    // legJoint = b2CreateWeldJoint(world->world_id, &jointDef);
    

    b2PrismaticJointDef jointDef = b2DefaultPrismaticJointDef();
        jointDef.bodyIdA = actor.bindings.body;
        jointDef.bodyIdB = leg.bindings.body;
        jointDef.collideConnected = true;
        jointDef.localAnchorA = {0, -props.player_radius};
        // pl(props.player_radius)
        jointDef.localAnchorB = {0, +0};
        jointDef.motorSpeed = 0;
        jointDef.enableMotor = false;
        jointDef.dampingRatio = .9;
        jointDef.maxMotorForce = 0;
        jointDef.upperTranslation = 1.f;
        jointDef.enableSpring = true;
        jointDef.hertz = 0;
        // jointDef.enableLimit
    legJoint = b2CreatePrismaticJoint(world->world_id, &jointDef);

}
// void Player::removeFromWorld(PhysicalWorld* world){
// }
//when not-0, used to display some computations
static VisualView* debugView = nullptr;

// Well, im not an artist, so i draw stuff with programatically
void Player::draw(VisualView* view) {
    // updateLegPositions()
    // pl(int(actor.properties.color.x))
    drawBody(view);
    drawEyes(view);
    drawArmsAndGun(view);
    drawLegs(view);

    Shape shape = {};
        shape.coloring_info = twpp::lime(600);
        shape.pos = b2glm(b2Body_GetPosition(leg.bindings.body));
        // pl(point.x);
        // pl(point.y);

        shape.rot_angle = 0;
        shape.rounding_radius = 0;
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = 0.15;
    DRAW_IF_DEBUG(shape);
}

static vec2 closest_point;
static float closest_frac;

float cast_fun(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void *context){
    // pl(fraction)
    // l();
    Shape shape = {};
        shape.coloring_info = twpp::cyan(600);
        shape.pos = b2glm(point);
        // pl(point.x);
        // pl(point.y);

        shape.rot_angle = 0;
        shape.rounding_radius = 0;
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = 0.15;
    // l();
    DRAW_IF_DEBUG(shape);
    // l();
    
    closest_point = b2glm(point);
    if(fraction < closest_frac){
        closest_frac = fraction;
        return fraction;
        // return fraction;
    }
    // l();
    
    return 1;
}

//finds place for a leg via b2 ray cast
void Player::updateLegPositions(PhysicalWorld* world) {
    
    float hipAngle = glm::pi<float>() / 6.0;
    vec2 hipPosLeft  = actor.state.pos + vec2(sin(-hipAngle), -(cos(hipAngle))) * props.player_radius;
    vec2 hipPosRight = actor.state.pos + vec2(sin(+hipAngle), -(cos(hipAngle))) * props.player_radius;

    // vec2 legOffset = (vec2(props.player_radius - 0.02, 0.1) / sqrtf(2.0));

    // Start point at the body
    // vec2 leftOrigin = actor.state.pos + vec2(-legOffset.x,  legOffset.y); 
    // vec2 rightOrigin = actor.state.pos + vec2(+legOffset.x, legOffset.y);

    b2QueryFilter filter_None = {};
        filter_None.categoryBits = to_underlying(ActorType::Scenery | ActorType::Projectile);
        filter_None.maskBits = to_underlying(ActorType::Scenery | ActorType::Projectile);
    //translation means vector prom [leg] origin to potential end
    float x_shift = 0;
    if (actor.state.vel.x > +.05){x_shift = +.5;}
    if (actor.state.vel.x < -.05){x_shift = -.5;}
    b2Vec2 translation = {};
        translation = {x_shift, -4.0};
    closest_frac = 10.0;
    
    // New candidates for leg positions
    b2World_CastRay(world->world_id, glm2b(hipPosLeft + vec2(0,0)), translation, filter_None, cast_fun, nullptr);
    // If any of candidates is far enough
    // then make a step
    if(closest_frac != 10){
        vec2 new_left_leg_pos = closest_point;
        if(glm::distance(new_left_leg_pos, real_left_leg_pos) > props.player_radius/0.5){
            real_left_leg_pos = new_left_leg_pos;
        }
    }

    if(debugView) {
        Shape shape = {};
            shape.coloring_info = twpp::red(600);
            shape.pos = closest_point;
            shape.rot_angle = 0;
            shape.rounding_radius = 0;
            shape.shapeType = Circle;
            shape.props.CIRCLE_radius = 0.15;
        debugView->draw_dynamic_shape(shape, SolidColor);
    }

        // translation = {x_shift, -4.0};
    b2World_CastRay(world->world_id, glm2b(hipPosRight + vec2(0,0)), translation, filter_None, cast_fun, nullptr);

    if(debugView) {
        Shape shape = {};
            shape.coloring_info = twpp::red(600);
            shape.pos = closest_point;
            shape.rot_angle = 0;
            shape.rounding_radius = 0;
            shape.shapeType = Circle;
            shape.props.CIRCLE_radius = 0.15;
        debugView->draw_dynamic_shape(shape, SolidColor);
    }
    
    if(closest_frac != 10){
        vec2 new_right_leg_pos = closest_point;
        if(glm::distance(new_right_leg_pos, real_right_leg_pos) > props.player_radius/0.5){
            real_right_leg_pos = new_right_leg_pos;
        }
    }

    smooth_left_leg_pos = mix(smooth_left_leg_pos, real_left_leg_pos, 0.8);
    smooth_right_leg_pos = mix(smooth_right_leg_pos, real_right_leg_pos, 0.8);
}
void Player::drawBody(VisualView* view) {
    // debugView = view;
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = props.player_radius;
        //overwrites
        shape.pos = visual_pos;
    view->draw_dynamic_shape(constructShape(), SolidColor);
}

void Player::drawLegs(VisualView* view) {
    const float legRadius = props.player_radius * 0.05f;
    const float legLength = props.player_radius * 0.5f;
    const float maxLegReach = 2.5f * legLength; // values >2.0 cause deattachment 

    float hipAngle = glm::pi<float>() / 6.0;
    vec2 hipPosLeft = visual_pos + vec2(sin(-hipAngle), -(cos(hipAngle))) * props.player_radius;
    vec2 hipPosRight = visual_pos + vec2(sin(+hipAngle), -(cos(hipAngle))) * props.player_radius;

    // Clamp foot position to be within the maximum reach from the hip
    auto clampFootPosition = [&](vec2 hipPos, vec2 footPos) {
        vec2 dir = footPos - hipPos;
        float dist = glm::length(dir);
        if (dist > maxLegReach) {
            footPos = hipPos + (dir / dist) * maxLegReach;
        }
        return footPos;
    };

    smooth_left_leg_pos = clampFootPosition(hipPosLeft, smooth_left_leg_pos);
    smooth_right_leg_pos = clampFootPosition(hipPosRight, smooth_right_leg_pos);

    auto calcLegAngles = [&](vec2 hipPos, vec2 footPos) {
        vec2 dir = footPos - hipPos;
        float dist = glm::length(dir);
        dir = glm::normalize(dir);

        dist = glm::clamp(dist, 0.0f, 2.0f * legLength);

        // Knee angle using triangle rule
        float baseAngle = -glm::atan(dir.y, dir.x);
        float kneeAngle = +glm::acos(dist / (2.0f * legLength));

        return std::make_pair(baseAngle + kneeAngle, baseAngle - kneeAngle);
    };

    auto calculateKneePosition = [&](vec2 startPos, float angle) {
        return startPos + vec2(glm::cos(angle), glm::sin(angle)) * legLength;
    };

    auto leftLegAngles = calcLegAngles(hipPosLeft, smooth_left_leg_pos);
    vec2 kneePosLeft = calculateKneePosition(hipPosLeft, leftLegAngles.first);

    Shape leftUpperLeg, leftLowerLeg;
    leftUpperLeg.coloring_info = actor.properties.color;
    leftUpperLeg.pos = (hipPosLeft + kneePosLeft) / 2.0f;
    leftUpperLeg.shapeType = Capsule;
    leftUpperLeg.props.CAPSULE_radius = legRadius;
    leftUpperLeg.props.CAPSULE_half_length = legLength / 2.0f;
    leftUpperLeg.rot_angle = leftLegAngles.first;

    leftLowerLeg = leftUpperLeg;
    vec2 clamped_diff_left = smooth_left_leg_pos - kneePosLeft;
    float distLeft = glm::length(clamped_diff_left);
    if (distLeft > legLength) {
        clamped_diff_left = glm::normalize(clamped_diff_left) * legLength;
    }
    leftLowerLeg.pos = (kneePosLeft + smooth_left_leg_pos) / 2.0f;
    leftLowerLeg.rot_angle = leftLegAngles.second;

    auto rightLegAngles = calcLegAngles(hipPosRight, smooth_right_leg_pos);
    vec2 kneePosRight = calculateKneePosition(hipPosRight, rightLegAngles.first);

    Shape rightUpperLeg, rightLowerLeg;
    rightUpperLeg.coloring_info = actor.properties.color;
    rightUpperLeg.pos = (hipPosRight + kneePosRight) / 2.0f;
    rightUpperLeg.shapeType = Capsule;
    rightUpperLeg.props.CAPSULE_radius = legRadius;
    rightUpperLeg.props.CAPSULE_half_length = legLength / 2.0f;
    rightUpperLeg.rot_angle = rightLegAngles.first;

    rightLowerLeg = rightUpperLeg;
    vec2 clamped_diff_right = smooth_right_leg_pos - kneePosRight;
    float distRight = glm::length(clamped_diff_right);
    if (distRight > legLength) {
        clamped_diff_right = glm::normalize(clamped_diff_right) * legLength;
    }
    rightLowerLeg.pos = (kneePosRight + smooth_right_leg_pos) / 2.0f;
    rightLowerLeg.rot_angle = rightLegAngles.second;

    view->draw_dynamic_shape(leftUpperLeg, SolidColor);
    view->draw_dynamic_shape(leftLowerLeg, SolidColor);
    view->draw_dynamic_shape(rightUpperLeg, SolidColor);
    view->draw_dynamic_shape(rightLowerLeg, SolidColor);
}


void Player::drawEyes(VisualView* view) {
    const float eyeRadius = 0.14 * props.player_radius;

    vec2 right_eye = vec2(.78, .62) * props.player_radius * 0.75f;

    Shape eye_right;
        eye_right.coloring_info = twpp::gray(800);
        eye_right.pos = visual_pos + right_eye;
        eye_right.shapeType = Circle;
        eye_right.props.CIRCLE_radius = eyeRadius; 
    Shape eye_left = eye_right;
        eye_left.pos.x -= props.player_radius*0.55;
    view->draw_dynamic_shape(eye_left, SolidColor);
    view->draw_dynamic_shape(eye_right, SolidColor);

    //extra height
    const float eyebrowHeight = 0.2 * props.player_radius;
    const float eyebrowLength = 0.22 * props.player_radius;
    
    Shape eyebrow_right;
        eyebrow_right.coloring_info = twpp::gray(900);
        eyebrow_right.pos = eye_right.pos + vec2(0, eyebrowHeight);
        eyebrow_right.shapeType = Rectangle;
        eyebrow_right.props.RECTANGLE_half_width = eyebrowLength; // Thin and long
        eyebrow_right.props.RECTANGLE_half_height = props.player_radius * 0.05f;
        eyebrow_right.rot_angle = -glm::radians(5.f);
        
    Shape eyebrow_left = eyebrow_right;
        eyebrow_left.pos = eye_left.pos + vec2(0, eyebrowHeight);
        eyebrow_left.rot_angle = +glm::radians(5.f);

    view->draw_dynamic_shape(eyebrow_left, SolidColor);
    view->draw_dynamic_shape(eyebrow_right, SolidColor);
}

void Player::drawArmsAndGun(VisualView* view) {
    // Settings - should be safe to change
    const float handRadius = props.player_radius * 0.2f;
    const float magazineWidth = props.player_radius * 0.45f;
    const float magazineHeight = props.player_radius * 0.65f;
    const float barrelWidth = props.player_radius * 0.3f;
    const float barrelLength = props.player_radius * 1.5f;
    const float ammoSize = props.player_radius * 0.1f;
    //TODO clamp?
    const int bulletsPerRow = 3;

    // Mirroring for gun
    vec2 aimDir = glm::normalize(state.aim_direction);
    const bool isMirrored = aimDir.x < 0;
    // Always render to the right and then maybe flip
    vec2 rawAimDir = aimDir;
    if (isMirrored) aimDir.x = -aimDir.x; 
    vec2 perpDir = vec2(-aimDir.y, aimDir.x);


    auto rotatePosition = [&](vec2 offset) -> vec2 {
        return visual_pos + aimDir * offset.x + perpDir * offset.y;
    };

    auto mirrorPosition = [&](vec2 center, vec2 pos) -> vec2 {
        vec2 relativePos = pos - center;
        if (isMirrored) {
            relativePos.x = -relativePos.x;
        }
        return relativePos + center;
    };

    auto mirrorRotation = [&](float rot) -> float {
        return isMirrored ? -rot : rot;
    };

        vec2 handPos = rotatePosition(vec2(props.player_radius * 0.8f, 0.0f));
    drawHand(view, handRadius, mirrorPosition(visual_pos, handPos));
    
        vec2 magazinePos = rotatePosition(vec2(props.player_radius * 0.8f + magazineHeight / 2.0f, 0.0f));
        const float rotAngle = -glm::atan(aimDir.y, aimDir.x);
    drawMagazine(view, mirrorPosition(visual_pos, magazinePos), magazineWidth, magazineHeight, mirrorRotation(rotAngle));

        vec2 barrelPos = rotatePosition(vec2(props.player_radius * 0.6f + magazineHeight + barrelLength / 2.0f, 0.1f));
    drawBarrel(view, mirrorPosition(visual_pos, barrelPos), barrelLength, barrelWidth, mirrorRotation(rotAngle));

        vec2 ammoStartPos = rotatePosition(vec2(props.player_radius * 0.8f + magazineHeight / 2.0f, magazineHeight / 2.0f));

    drawAmmo(view, mirrorPosition(visual_pos, ammoStartPos), ammoSize, bulletsPerRow, state.ammunition_left, rawAimDir, perpDir, rotAngle, isMirrored);
}

void Player::drawHand(VisualView* view, float handRadius, vec2 handPos) {
    Shape handShape;
        handShape.coloring_info = twpp::gray(600);
        handShape.pos = handPos;
        handShape.shapeType = Circle;
        handShape.props.CIRCLE_radius = handRadius;
    view->draw_dynamic_shape(handShape, SolidColor);
}

void Player::drawMagazine(VisualView* view, vec2 magazinePos, float magazineWidth, float magazineHeight, float rotAngle) {
    Shape magazineShape;
        magazineShape.coloring_info = twpp::gray(900);
        magazineShape.pos = magazinePos;
        magazineShape.shapeType = Rectangle;
        magazineShape.props.RECTANGLE_half_width = magazineWidth / 2.0f;
        magazineShape.props.RECTANGLE_half_height = magazineHeight / 2.0f;
        magazineShape.rounding_radius = 0.05f;
        magazineShape.rot_angle = rotAngle;
    view->draw_dynamic_shape(magazineShape, SolidColor);
}

void Player::drawBarrel(VisualView* view, vec2 barrelPos, float barrelLength, float barrelWidth, float rotAngle)  {
    Shape barrelShape;
        barrelShape.coloring_info = twpp::gray(900);
        barrelShape.pos = barrelPos;
        barrelShape.shapeType = Rectangle;
        barrelShape.props.RECTANGLE_half_width = barrelLength / 2.0f;
        barrelShape.props.RECTANGLE_half_height = barrelWidth / 2.0f;
        barrelShape.rot_angle = rotAngle;
    view->draw_dynamic_shape(barrelShape, SolidColor);
}

void Player::drawAmmo(VisualView* view, vec2 ammoStartPos, float ammoSize, int bulletsPerRow, 
                      int totalAmmo, vec2 aimDir, vec2 perpDir, float rot, bool isMirrored) {
    const int rows = (totalAmmo + bulletsPerRow - 1) / bulletsPerRow;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; (col < bulletsPerRow) && (totalAmmo > 0); col++) {
            int mirroredCol = isMirrored ? (bulletsPerRow - 1 - col) : col;

            vec2 bulletOffset = vec2((mirroredCol - 1) * ammoSize * 1.5f, row * ammoSize * 1.1f);
            vec2 bulletPos = ammoStartPos + aimDir * bulletOffset.x + perpDir * bulletOffset.y;

            Shape bulletShape;
                bulletShape.coloring_info = twpp::yellow(500);
                bulletShape.pos = bulletPos;
                bulletShape.rot_angle = rot;
                bulletShape.shapeType = Rectangle;
                bulletShape.props.RECTANGLE_half_width = ammoSize / 2.0f;
                bulletShape.props.RECTANGLE_half_height = ammoSize / 2.0f;
            view->draw_dynamic_shape(bulletShape, SolidColor);

            totalAmmo--;
        }
    }
}

// void Player::drawCard(const Card* card) {
//     props.health_points *= card->hp_buff;
//     props.player_mass = 1.0f;
//     // props.player_radius *= ???? TODO compute from mass;
//     props.bullet_size *= card->bullet_radius_mul;
//     props.bullet_size += card->bullet_radius_add;
//     props.jump_force *= card->jump_buff;
//     props.max_jumps += card->extra_jumps;
//     props.bullet_damage *= card->damage_buff;
//     props.reload_duration *= card->reload_time_buff; //between closest shots from separate magazines
//     props.shot_delay *= card->shoot_time_buff; //between shots from same magazine
//     props.block_cooldown *= card->block_delay_buff;
//     props.max_lives += card->lives_buff;
//     props.bullets_per_magazine += card->ammunition_buff;
//     props.bullet_velocity *= card->bullet_speed_buff;
//     if(card->set_bullet_color){
//         props.bullet_color = card->bullet_color_set;
//     }
//     props.bullet_bounce_count += card->bullet_bounce_buff;
//     props.hit_knockback *= card->knockback_buff;

//     //constrains
//     props.bullet_size = glm::clamp(props.bullet_size, 0.01f, 2.f);
//     props.health_points = glm::max(props.health_points, 100.0);

//     cards.push_back(card);
// }

void Player::drawCard(const Card* card) {
    // Apply the card buffs
    props.health_points *= card->hp_mul;
    props.player_mass = 1.0f; // This can be updated if you want cards to modify player mass

    // TODO: Compute player radius from mass (assuming spherical scaling with mass)
    // props.player_radius = glm::pow((3.0f * props.player_mass) / (4.0f * glm::pi<float>()), 1.0f / 3.0f)*0.69;

    props.bullet_size *= card->bullet_radius_mul;
    props.bullet_size += card->bullet_radius_add;
    props.jump_impulse += card->jump_impulse_add;  // Addition to jump force
    props.max_jumps += card->jumps_add;         // Additional jumps
    props.bullet_damage *= card->damage_mul;    // Damage buff multiplier
    props.reload_duration *= card->reload_time_mul;
    props.reload_duration += card->reload_time_add;  // Adjust for flat reload time changes
    props.shot_delay *= card->shoot_time_mul;
    props.block_cooldown += card->block_cooldown_add;
    props.max_lives += card->lives_add;
    props.bullets_per_magazine += card->ammo_add;
    props.bullet_velocity *= card->bullet_speed_mul;

    // Check for custom bullet color    
    if (card->has_custom_bullet_color) {
        props.bullet_color = card->bullet_color;
    }

    props.bullet_bounce_count += card->bullet_bounces_add; // Add bullet bounce count
    props.hit_knockback += card->knockback_add; // Add knockback effect

    // Apply other special effects
    props.life_steal_percentage += card->life_steal_percentage_add;
    props.explosion_radius += card->explosion_radius_add;
    props.homing_strength += card->homing_force_add;
    props.poison_damage_percentage += card->poison_damage_percentage_add;
    props.stun_duration += card->stun_duration_add;

    // On Block effects
    // props.bombs_spawned_on_block += card->bombs_on_block_add;
    // props.teleport_distance_on_block += card->teleport_distance_on_block_add;
    // props.saw_damage_on_block += card->saw_damage_on_block_add;
    // props.dark_power_charge_time += card->dark_power_charge_time_add;

    // Ensure constraints on player and bullet properties
    props.bullet_size = glm::clamp(props.bullet_size, 0.01f, 2.f);
    props.health_points = glm::max(props.health_points, 100.0);

    // Save the card in the player's card collection
    cards.push_back(card);
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

void Player::resetState() {
    state.jumps_left = props.max_jumps;
    state.touching_grass_counter = 0;
    state.refill_body_jumps_next_frame = false;
    // state.refill_leg_jumps_next_frame = false;
    state.hp_left = props.health_points;
    state.aim_direction = vec2(0);
    state.damage = props.bullet_damage;
    state.time_since_last_reload = 0; //between closest shots from separate magazines
    state.time_since_last_shot = 0; //between shots from same magazine
    state.time_since_last_block = 0;
    state.lives_left = props.max_lives;
    state.ammunition_left = props.bullets_per_magazine; //in magazine
    actor.state.vel = {};
    actor.state.pos = {};
    b2Body_SetTransform(actor.bindings.body, {}, {1, 0});
    b2Body_SetTransform(leg.bindings.body, {}, {1, 0});
    b2Body_SetLinearVelocity(actor.bindings.body, {});
    b2Body_SetLinearVelocity(leg.bindings.body, {});
}