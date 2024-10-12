#include "player.hpp"

#define DRAW_IF_DEBUG(_shape) if(debugView) debugView->draw_dynamic_shape(_shape, SolidColor)

void Player::processDamage(float damage) {
    // pl(damage)
    state.hp_left -= damage;

    if(state.hp_left < 0){
       // death anim? TODO: 
        state.lives_left--;
    }
    if(state.lives_left < 0){
       // final death? TODO?
    }
}
void Player::processBorderTouch(PhysicalWorld* world, vec2 normal) {
    // vec2 impulse = {0, +50};
    vec2 impulse = 10.f*normal;
    world->applyImpulse(actor.bindings.body, glm2b(impulse));
    world->applyImpulse(leg.bindings.body, glm2b(impulse));
    processDamage(60);
}

void Player::processBulletHit(PhysicalWorld* world, ProjectileState* projectile){
    float damage = projectile->damage;
    processDamage(damage);
    // world->applyImpulse(actor.bindings.body, b2Vec2());
    //block?
}


Shape Player::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = props.radius;
        // shape.rounding_radius = props.radius/2.0;
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

void Player::refillJumpsIfNeeded(float dTime){
    state.time_since_jump_refill += dTime;
    const float JUMP_REFILL_TIME = 0.1;

    //like its next frame. TODO: box2d end event on the same tick?
    if(state.refill_body_jumps_next_frame){
        if(state.time_since_jump_refill > JUMP_REFILL_TIME){
            state.jumps_left = props.max_jumps;
            state.refill_body_jumps_next_frame = false;
            state.time_since_jump_refill = 0;
        }
    }
    // if(state.refill_leg_jumps_next_frame){
    //     state.jumps_left = props.max_jumps;
    //     state.refill_leg_jumps_next_frame = false;
    // }
    if(state.jumps_left < props.max_jumps){
            if(state.touching_grass_counter > 0){
                state.refill_body_jumps_next_frame = true;
                // state.refill_leg_jumps_next_frame = true;
            }
            if(state.touching_grass_counter < 0) state.touching_grass_counter = 0;
    }
    // pl(state.refill_body_jumps_next_frame)
    // pl(state.refill_leg_jumps_next_frame)
    // pl(state.touching_grass_counter)
}

void Player::updateGun(float dTime) {
    state.time_since_last_reload += dTime;
    state.time_since_last_shot += dTime;

    if(state.time_since_last_reload > props.reload_duration){
        if(state.ammunition_left < props.max_ammo){
            state.ammunition_left = props.max_ammo;
            state.time_since_last_reload = 0;
        }
    }
}

void Player::updateHpBar(float dTime) {
    const float HP_DECAY_TIME = 0.15f;
    hp_bar.current_ratio = glm::mix(
        hp_bar.current_ratio,
        float(state.hp_left / props.hp),
        glm::clamp(dTime, 0.f, HP_DECAY_TIME)/HP_DECAY_TIME
    );
}

void Player::update(PhysicalWorld* world, float dTime /*for later?*/, vec2 view_pos){
    softLimitVelocity(world);
    refillJumpsIfNeeded(dTime);
    updateGun(dTime);
    updateLegPositions(world);
    updateThrusters(dTime);
    updateHpBar(dTime);
    // TODO: better jump
    // if(actor.state.vel.y > -0.01){
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // } else {
    //     b2Body_SetGravityScale(actor.bindings.body, 2.0);
    // }

    state.damage = props.bullet_damage;
    // state.
    state.aim_direction = normalize(view_pos - actor.state.pos);
}

void Player::updateThrusters(float dTime){
    int write_index = 0;
    for (int i = 0; i < thrusters.size(); i++) {
        Thruster& thruster = thrusters[i];

        // if still active
        if (thruster.time_left > 0) {
            thruster.time_left -= dTime;

            // keep this thruster
            thrusters[write_index] = thruster;
            write_index++;
        }
    }
}

void Player::addToWorld(PhysicalWorld* world){
    b2Circle playerBodyCircle = {};
        playerBodyCircle.radius = props.radius;
    b2Filter playerFilter = {};
        playerFilter.categoryBits = to_underlying(ActorType::Player);
        playerFilter.maskBits = to_underlying(ActorType::Projectile|ActorType::Player|ActorType::StaticScenery|ActorType::DynamicScenery|ActorType::Border|ActorType::PlayerLeg);
        // playerFilter.groupIndex = -abs((long long int)this % 666); // so never collides with own leg
    b2ShapeDef playerBodyShape = b2DefaultShapeDef();
        playerBodyShape.restitution = 0.01;
        playerBodyShape.friction = 0.05;
        playerBodyShape.filter = playerFilter;
    b2BodyDef bdef = b2DefaultBodyDef();
        bdef.fixedRotation = true;

    world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &playerBodyCircle, &bdef, &playerBodyShape);

    b2Circle playerLegCircle = {};
        playerLegCircle.radius = 0.1;
    b2Filter playerLegFilter = {};
        playerLegFilter.categoryBits = to_underlying(ActorType::PlayerLeg);
        playerLegFilter.maskBits = to_underlying(ActorType::Player|ActorType::StaticScenery);
        // playerLegFilter.groupIndex = -abs((long long int)this % 666); // so never collides with own body
    b2ShapeDef playerLegShape = b2DefaultShapeDef();
        playerLegShape.restitution = 0.01;
        playerLegShape.friction = 0.05;
        playerLegShape.filter = playerLegFilter;
        playerLegShape.density = 0;
    bdef.position.y -= props.radius;
    bdef.automaticMass = true;
    world->addActor<b2Circle, b2CreateCircleShape>(&leg.bindings, &actor.state, &actor.properties, &leg, &playerLegCircle, &bdef, &playerLegShape);

    b2PrismaticJointDef jointDef = b2DefaultPrismaticJointDef();
        jointDef.bodyIdA = actor.bindings.body;
        jointDef.bodyIdB = leg.bindings.body;
        jointDef.collideConnected = true;
        jointDef.localAnchorA = {0, -props.radius};
        // pl(props.radius)
        jointDef.localAnchorB = {0, +0};
        jointDef.motorSpeed = 0;
        jointDef.enableMotor = false;
        jointDef.dampingRatio = .99;
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

    drawHpBar(view);

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
    vec2 hipPosLeft  = actor.state.pos + vec2(sin(-hipAngle), -(cos(hipAngle))) * props.radius;
    vec2 hipPosRight = actor.state.pos + vec2(sin(+hipAngle), -(cos(hipAngle))) * props.radius;

    // vec2 legOffset = (vec2(props.radius - 0.02, 0.1) / sqrtf(2.0));

    // Start point at the body
    // vec2 leftOrigin = actor.state.pos + vec2(-legOffset.x,  legOffset.y); 
    // vec2 rightOrigin = actor.state.pos + vec2(+legOffset.x, legOffset.y);

    b2QueryFilter filter_None = {};
        filter_None.categoryBits = to_underlying(ActorType::StaticScenery | ActorType::Projectile);
        filter_None.maskBits = to_underlying(ActorType::StaticScenery | ActorType::Projectile);
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
        if(glm::distance(new_left_leg_pos, real_left_leg_pos) > props.radius/0.5){
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
        if(glm::distance(new_right_leg_pos, real_right_leg_pos) > props.radius/0.5){
            real_right_leg_pos = new_right_leg_pos;
        }
    }

    smooth_left_leg_pos = mix(smooth_left_leg_pos, real_left_leg_pos, 0.8);
    smooth_right_leg_pos = mix(smooth_right_leg_pos, real_right_leg_pos, 0.8);
}
void Player::drawBody(VisualView* view) {
    debugView = view;
    view->draw_dynamic_shape(constructShape(), SolidColor);
}

void Player::drawLegs(VisualView* view) {
    const float legRadius = props.radius * 0.05f;
    const float legLength = props.radius * 0.5f;
    const float maxLegReach = 2.5f * legLength; // values >2.0 cause deattachment 

    float hipAngle = glm::pi<float>() / 6.0;
    vec2 hipPosLeft = visual_pos + vec2(sin(-hipAngle), -(cos(hipAngle))) * props.radius;
    vec2 hipPosRight = visual_pos + vec2(sin(+hipAngle), -(cos(hipAngle))) * props.radius;

    // Clamp foot position to be within the maximum reach from the hip
    auto clampFootPosition = [&](vec2 hipPos, vec2 footPos) {
        vec2 dir = footPos - hipPos;
        float dist = glm::length(dir);
        if (dist > maxLegReach) {
            footPos = hipPos + (dir / dist) * maxLegReach;
        }
        return footPos;
    };

    vec2 smooth_left_leg_pos = clampFootPosition(hipPosLeft, this->smooth_left_leg_pos);
    vec2 smooth_right_leg_pos = clampFootPosition(hipPosRight, this->smooth_right_leg_pos);

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

void Player::drawThrusters(VisualView* view) {
    for (const Thruster& thruster : thrusters) {
        const  vec2 player_contact = visual_pos + props.radius * thruster.dir;
        const  vec2 center = player_contact + thruster.hlen * thruster.dir;
        const float rot_angle = -glm::atan(thruster.dir.y, thruster.dir.x);

        // main gray capsule
        Shape shape = {};
            shape.shapeType = Capsule;
            shape.pos = center;
            shape.rot_angle = rot_angle;
            shape.props.CAPSULE_half_length = thruster.hlen;
            shape.props.CAPSULE_radius = thruster.hlen * 0.8;
            shape.coloring_info = twpp::gray(600);
        view->draw_dynamic_shape(shape, SolidColor);

        // inside glowing thing
        shape = {};
            shape.shapeType = Capsule;
            shape.pos = center;
            shape.rot_angle = rot_angle;
            shape.props.CAPSULE_half_length = thruster.hlen;
            shape.props.CAPSULE_radius = thruster.hlen * 0.8;
            shape.coloring_info = twpp::gray(600);
        view->draw_dynamic_shape(shape, SolidColor);

        // vec2 trailPos = visual_pos;
            // vec2 trailVel = -thruster.dir * 50.0f;

            // scene->addEffect(
            //     u8vec3(255, 200, 50),  // Base color: yellowish for fire trail
            //     20,                    // Color variation
            //     trailPos, 3.0f,        // Particle spawn position variation
            //     trailVel, 10.0f,       // Velocity with slight variation
            //     thruster.size * 0.1f,  // Particle size based on thruster size
            //     thruster.size * 0.05f, // Size variation
            //     0.5f,                  // Particle lifetime
            //     0.2f,                  // Lifetime variation
            //     10                     // Number of particles
            // );
    }
}

void Player::drawEyes(VisualView* view) {
    const float eyeRadius = 0.14 * props.radius;

    vec2 right_eye = vec2(.78, .62) * props.radius * 0.75f;

    Shape eye_right;
        eye_right.coloring_info = twpp::gray(800);
        eye_right.pos = visual_pos + right_eye;
        eye_right.shapeType = Circle;
        eye_right.props.CIRCLE_radius = eyeRadius; 
    Shape eye_left = eye_right;
        eye_left.pos.x -= props.radius*0.55;
    view->draw_dynamic_shape(eye_left, SolidColor);
    view->draw_dynamic_shape(eye_right, SolidColor);

    //extra height
    const float eyebrowHeight = 0.2 * props.radius;
    const float eyebrowLength = 0.22 * props.radius;
    
    Shape eyebrow_right;
        eyebrow_right.coloring_info = twpp::gray(900);
        eyebrow_right.pos = eye_right.pos + vec2(0, eyebrowHeight);
        eyebrow_right.shapeType = Rectangle;
        eyebrow_right.props.RECTANGLE_half_width = eyebrowLength; // Thin and long
        eyebrow_right.props.RECTANGLE_half_height = props.radius * 0.05f;
        eyebrow_right.rot_angle = -glm::radians(5.f);
        
    Shape eyebrow_left = eyebrow_right;
        eyebrow_left.pos = eye_left.pos + vec2(0, eyebrowHeight);
        eyebrow_left.rot_angle = +glm::radians(5.f);

    view->draw_dynamic_shape(eyebrow_left, SolidColor);
    view->draw_dynamic_shape(eyebrow_right, SolidColor);
}

void Player::drawArmsAndGun(VisualView* view) {
    // Settings - should be safe to change
    const float handRadius = props.radius * 0.2f;
    const float magazineWidth = props.radius * 0.45f;
    const float magazineHeight = props.radius * 0.65f;
    const float barrelWidth = props.radius * 0.3f;
    const float barrelLength = props.radius * 1.5f;
    const float ammoSize = props.radius * 0.1f;
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

        vec2 handPos = rotatePosition(vec2(props.radius * 0.8f, 0.0f));
    drawHand(view, handRadius, mirrorPosition(visual_pos, handPos));
    
        vec2 magazinePos = rotatePosition(vec2(props.radius * 0.8f + magazineHeight / 2.0f, 0.0f));
        const float rotAngle = -glm::atan(aimDir.y, aimDir.x);
    drawMagazine(view, mirrorPosition(visual_pos, magazinePos), magazineWidth, magazineHeight, mirrorRotation(rotAngle));

        vec2 barrelPos = rotatePosition(vec2(props.radius * 0.6f + magazineHeight + barrelLength / 2.0f, 0.1f));
    drawBarrel(view, mirrorPosition(visual_pos, barrelPos), barrelLength, barrelWidth, mirrorRotation(rotAngle));

        vec2 ammoStartPos = rotatePosition(vec2(props.radius * 0.8f + magazineHeight / 2.0f, magazineHeight / 2.0f));

    drawAmmo(view, mirrorPosition(visual_pos, ammoStartPos), ammoSize, bulletsPerRow, state.ammunition_left, rawAimDir, perpDir, rotAngle, isMirrored);

    // at the end of the gun
    float shotDelaySize = props.radius * 0.3f;
    float shotDelayFill = state.time_since_last_shot / props.shot_delay;
    vec2 shotDelayPos = mirrorPosition(visual_pos, magazinePos + perpDir * (barrelLength / 2.0f) + aimDir * barrelLength);
    drawShotDelay(view, shotDelayPos, shotDelayFill, shotDelaySize);

    // above the barrel
    float reloadDelaySize = props.radius * 0.3f;
    float reloadDelayFill = state.time_since_last_reload / props.reload_duration;
    vec2 reloadDelayPos = mirrorPosition(visual_pos, magazinePos + perpDir * (barrelLength / 2.0f)); 
    drawReloadDelay(view, reloadDelayPos, reloadDelayFill, reloadDelaySize);
}

void Player::drawShotDelay(VisualView* view, vec2 pos, float fill, float size){
    Shape shape;
        shape.coloring_info = twpp::gray(600);
        shape.pos = pos;
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = size;
        shape.rounding_radius = fill*size;
    view->draw_dynamic_shape(shape, SolidColor);
}
void Player::drawReloadDelay(VisualView* view, vec2 pos, float fill, float size){
    Shape shape;
        shape.coloring_info = twpp::gray(600);
        shape.pos = pos;
        shape.shapeType = Circle;
        shape.props.CIRCLE_radius = size;
        if(state.ammunition_left == props.max_ammo) fill = 1;
        shape.rounding_radius = fill*size;
    view->draw_dynamic_shape(shape, SolidColor);
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

void Player::drawCard(const Card* card) {
    props.hp *= card->hp_mul;
    props.mass = 1.0f;
    // props.radius *= ???? TODO compute from mass;
    props.bullet_radius *= card->bullet_radius_mul;
    props.bullet_radius += card->bullet_radius_add;
    props.spread += card->spread_add;
    props.jump_impulse *= card->jump_impulse_add;
    props.max_jumps += card->jumps_add;
    props.bullet_damage *= card->damage_mul;
    props.bullets_per_shot += card->extra_bullets_per_shot_add;
    props.reload_duration *= card->reload_time_mul; //between closest shots from separate magazines
    props.shot_delay *= card->shoot_time_mul; //between shots from same magazine
    // props.block_delay *= card->block_delay_buff;
    props.max_lives += card->lives_add;
    props.max_ammo += card->ammo_add;
    props.bullet_speed *= card->bullet_speed_mul;
    if(card->has_custom_bullet_color){
        props.bullet_color = card->bullet_color;
    }
    props.bullet_bounces += card->bullet_bounces_add;
    props.extra_damage_per_bounce += card->bullet_bounce_damage_add;
    props.grow_factor += card->grow_factor_add;
    props.hit_knockback += card->knockback_add;

    //constrains
    props.bullet_radius = glm::clamp(props.bullet_radius, 0.01f, 2.f);
    props.hp = glm::max(props.hp, 50.0);
    props.spread = glm::clamp(props.spread, 0.01f, .495f);
    props.bullet_damage = glm::max(props.bullet_damage, 1.0);
    props.shot_delay = glm::clamp(props.shot_delay, 0.005, 60.0);
    // props.bullet_bounces = glm::clamp(props.bullet_bounces, 0.f, .99f);

    cards.push_back(card);
}

ProjectileProps Player::createBullet(){
    ProjectileProps _props = {};
        _props.damage = props.bullet_damage;
        _props.extra_damage_per_bounce = props.extra_damage_per_bounce;
        _props.bounciness = .9;
        _props.radius = props.bullet_radius;
    return _props;
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
    state.hp_left = props.hp;
    state.aim_direction = vec2(0);
    state.damage = props.bullet_damage;
    state.time_since_last_reload = +INFINITY;
    state.time_since_last_shot = +INFINITY;
    state.time_since_last_block = +INFINITY;
    state.lives_left = props.max_lives;
    state.ammunition_left = props.max_ammo;
    actor.state.vel = {};
    actor.state.pos = {};
    b2Body_SetTransform(actor.bindings.body, {}, {1, 0});
    b2Body_SetTransform(leg.bindings.body, {}, {1, 0});
    b2Body_SetLinearVelocity(actor.bindings.body, {});
    b2Body_SetLinearVelocity(leg.bindings.body, {});
}

void Player::processJump(PhysicalWorld* world) {
    if(state.jumps_left > 0){
        // npl("JUMP");
        vec2 old_vel = world->getVelocity(actor.bindings.body);
        world->setVelocity(actor.bindings.body, {old_vel.x, 15.0});
        world->setVelocity(leg.bindings.body, {old_vel.x, 15.0});
        // TODO?
        // world.applyImpulse(actor.bindings.body, 
        //     b2Vec2(0,20.0));
        //it is important to set transform due to latency
        b2Transform tb = b2Body_GetTransform(actor.bindings.body);
        b2Transform tl = b2Body_GetTransform(actor.bindings.body);
        tb.p.y += 0.01;
        tl.p.y += 0.01;
        b2Body_SetTransform(actor.bindings.body, tb.p, tb.q);
        b2Body_SetTransform(leg.bindings.body, tl.p, tl.q);
        state.jumps_left--;

        //so not updated from previous
        state.refill_body_jumps_next_frame = false;
        // state.refill_leg_jumps_next_frame = false;
    }
}


void Player::drawHpBar(VisualView* view) {
    float barWidth = props.radius*2.7;
    float barHeight = props.radius*0.08;
    float barYOffset = props.radius - 0.01;
    vec2 barPosition = visual_pos + vec2(0, props.radius + barYOffset);

    double maxHP = props.hp;
    double currentHP = state.hp_left;

    float filledBarWidth = float(hp_bar.current_ratio * barWidth);

    Shape barBack = {};
        barBack.shapeType = Rectangle;
        barBack.coloring_info = twpp::slate(800);
        barBack.props.RECTANGLE_half_width = (barWidth / 2.0f) * 1.1;
        barBack.props.RECTANGLE_half_height = (barHeight / 2.0f) * 1.1;
        barBack.pos = barPosition;

    Shape barFill = {};
        barFill.shapeType = Rectangle;
        barFill.coloring_info = twpp::lime(600);
        barFill.props.RECTANGLE_half_width = filledBarWidth / 2.0f;
        barFill.props.RECTANGLE_half_height = barHeight / 2.0f;
        barFill.pos = barPosition - vec2((barWidth - filledBarWidth) / 2.0f, 0);

    view->draw_dynamic_shape(barBack, SolidColor);
    view->draw_dynamic_shape(barFill, SolidColor);
}