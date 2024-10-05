#include "logic.hpp"
#include "macros.hpp"
#include <random>

#define println printf("%s:%d: Fun: %s\n", __FILE__, __LINE__, __FUNCTION__);

void LogicalScene::addScenery(Scenery scenery){
    ListElem<Scenery>* new_e = sceneries.appendBack(scenery);
    new_e->obj()->addToWorld(&world);
}
void LogicalScene::removeScenery(ListElem<Scenery>* se){
    b2DestroyBody(se->obj()->actor.bindings.body); //TODO
    sceneries.removeElem(se);
}
void LogicalScene::removeAllScenery() {
    for (mut scenery : sceneries) {
        b2DestroyBody(scenery.actor.bindings.body);
    }
    sceneries.removeAll();
}

void LogicalScene::addProjectile(Projectile projectile) {
    ListElem<Projectile>* new_e = projectiles.appendBack(projectile);
    // pl(projectile.actor.state.pos.x)
    // pl(projectile.actor.state.pos.y)
    // pl(projectile.actor.state.vel.x)
    // pl(projectile.actor.state.vel.y)
    new_e->obj()->addToWorld(&world);
}

void LogicalScene::removeProjectile(ListElem<Projectile>* pe) {
    b2DestroyBody(pe->obj()->actor.bindings.body);
    projectiles.removeElem(pe);
}

void LogicalScene::removeAllProjectiles() {
    for (mut projectile : projectiles) {
        b2DestroyBody(projectile.actor.bindings.body);
    }
    projectiles.removeAll();
}

// when world is cleared and new one created
// players travel between worlds, everything else doesnt

void LogicalScene::addNewPlayer(/*some visual props*/) {
    Player player = Player();
    player.actor.state.pos.y = 8.0;
    ListElem<Player>* new_e = players.appendBack(player);
    new_e->obj()->addToWorld(&world);
}
void LogicalScene::removeAllPlayers() {
    for (mut player : players) {
        b2DestroyBody(player.actor.bindings.body);
    }
    players.removeAll();
}
void LogicalScene::resetPlayersState() {
    for (mut player : players) {
        player.state.jumps_left = player.props.max_jumps;
        player.state.touching_grass_counter = 0;
        player.state.refill_jumps_next_frame = false;
        player.state.hp_left = player.props.hp;
        player.state.aim_direction = vec2(0);
        player.state.damage = player.props.damage;
        player.state.time_since_last_reload = 0; //between closest shots from separate magazines
        player.state.time_since_last_shot = 0; //between shots from same magazine
        player.state.time_since_last_block = 0;
        player.state.lives_left = player.props.max_lives;
        player.state.ammunition_left = player.props.ammunition_count; //in magazine
        
        //re-add to the world? seems better to reset position
        //TODO reset pos, vel, etc.
    }
}

void LogicalScene::endRound(){
    //TODO random
    Card card = CarefulPlanning;
    controlled_player->drawCard(card);
}

//TODO
void LogicalScene::genRndScenery(){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> size_dist(1.0, 3.0); 

    float bounds = 13;
    float current_x = -bounds;
    float current_y = -bounds;

    int num_of_rectangles = 35;
    float gap_between_rectangles = 2.7; 

    for (int i = 0; i < num_of_rectangles; i++) {
        float half_width = size_dist(gen);
        float half_height = size_dist(gen);

        vec2 position = vec2(half_width+current_x, half_height+current_y);

        addScenery(Scenery(Square, {.SQUARE_half_width=half_width, .SQUARE_half_height=half_height}, position, false));

        current_x += 2.0 * half_width + gap_between_rectangles;
        if (abs(current_x) > bounds) {
            current_x = -bounds;
            current_y += half_height + 2.5 + gap_between_rectangles;
        }
    }
}

void LogicalScene::clearWorld(void) {
    //TODO destroy ents?
    removeAllScenery();
    removeAllProjectiles();
    world.cleanup(); 
    world.setup();
}

void LogicalScene::setupActionCallbacks() {
    // input.rebindKey(Action::Jump, GLFW_KEY_SPACE);
    input.setActionCallback(Action::Jump, 
        [this](Action action) -> void {
            // Player* p = players._start->obj();
            // pl(controlled_player->state.jumps_left)
            if(controlled_player->state.jumps_left > 0){
                // npl("JUMP");
                PlayerJumpEffect(controlled_player);
                vec2 old_vel = world.getVelocity(controlled_player->actor.bindings.body);
                world.setVelocity(controlled_player->actor.bindings.body, {old_vel.x, 15.0});
                // world.applyImpulse(controlled_player->actor.bindings.body, 
                //     b2Vec2(0,20.0));
                auto t = b2Body_GetTransform(controlled_player->actor.bindings.body);
                t.p.y += 0.01;
                b2Body_SetTransform(controlled_player->actor.bindings.body, t.p, t.q);
                controlled_player->state.jumps_left--;

                //so not updated from previous
                controlled_player->state.refill_jumps_next_frame = false;
            }
        }
    );
    input.setActionCallback(Action::Shoot, 
        [this](Action action) -> void {
            // static int counter=0;
            // counter++;
            pl("shoot")
            pl(controlled_player->props.damage)
            //TODO: restructure
            // Player* p = players._start->obj();
            auto bullet = Projectile(controlled_player->props.damage, controlled_player->props.bullet_radius);
            
            bullet.setup(&controlled_player->state, &controlled_player->props, &controlled_player->actor);
            // l()
            addProjectile(bullet);
            pl("end shoot")
        }
    );
    input.setActionCallback(Action::DrawRNDcard, 
        [this](Action action) -> void {
            int id = rand() % std::size(cards);
            pl("drawing a card?")
                controlled_player->drawCard(cards[id]);
                controlled_player->drawCard(BigFastBullets);
            pl("... and what did we get?")
        }
    );
    input.setActionCallback(Action::MoveLeft, 
        [this](Action action) -> void {
            vec2 old_vel = world.getVelocity(controlled_player->actor.bindings.body);
            if(old_vel.x > -1.0){
                // world.setVelocity(controlled_player->actor.bindings.body, {-1, old_vel.y});
                world.applyImpulse(controlled_player->actor.bindings.body, {-1, 0});
            }
            world.applyForce(controlled_player->actor.bindings.body, 
                b2Vec2(-20.f,0.1));
            
        // al()
        }
    );
    input.setActionCallback(Action::MoveRight, 
        [this](Action action) -> void {
            vec2 old_vel = world.getVelocity(controlled_player->actor.bindings.body);
            if(old_vel.x < 1.0){
                world.applyImpulse(controlled_player->actor.bindings.body, {+1, 0});
                // world.setVelocity(controlled_player->actor.bindings.body, {1, old_vel.y});
            }
            world.applyForce(controlled_player->actor.bindings.body, 
                b2Vec2(+20.f,0.1));
        // al()
        }
    );
}
void LogicalScene::setup(int player_count) {
    world.setup();
    view.setup(); 


    for(int i=0; i<player_count; i++){
        addNewPlayer();
    }

    //TODO
    controlled_player = players._start->obj();

    input.setup(view.render.window.pointer);

    setupActionCallbacks();
}

void LogicalScene::cleanup(void) {
    //TODO check b2
    removeAllPlayers();
    removeAllScenery();
    removeAllProjectiles();
    world.cleanup();
    view.cleanup();
    input.cleanup();
}

void LogicalScene::processBeginEvents(b2ContactEvents contacts){ 
    auto beginProcessor = [this](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::Scenery): {
                // al()
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                player->state.touching_grass_counter++;
                // player->state.last_jmp_refill = glfwGetTime(); //TODO?
                // al()
                break;
            }
            case (ActorType::Projectile | ActorType::Scenery): {
                // al()
                Projectile* projectile = (typeA == ActorType::Projectile) ? (Projectile*)(udataA) : (Projectile*)(udataB);
                Scenery* scenery = (typeA == ActorType::Scenery) ? (Scenery*)(udataA) : (Scenery*)(udataB);
                //possibly damage scenery, bounce/kill bullet
                // smoke
                BulletSceneryHitEffect(projectile, scenery);
                bool survived = projectile->processSceneryHit(&scenery->state);
                if(!survived){
                    // body_garbage.push_back(projectile->actor.bindings.body);
                    // projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                    // removeProjectile((ListElem<Projectile>*)projectile);
                    body_garbage.push_back(projectile->actor.bindings.body);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                }
                // al()
                break;
            }
            case (ActorType::Player | ActorType::Projectile): {
                // al()
                Projectile* projectile = (typeA == ActorType::Projectile) ? (Projectile*)(udataA) : (Projectile*)(udataB);
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                pl(projectile->state.damage);
                pl(projectile->props.damage);

                bool survived = projectile->processPlayerHit(&player->state);
                if(!survived){
                    body_garbage.push_back(projectile->actor.bindings.body);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                    BulletPlayerHitEffect(projectile, player);
                }
                if(player != controlled_player){
                    player->processBulletHit(&world, &projectile->state);
                    vec2 impulse = 
                        normalize(projectile->actor.state.vel) *
                        log2(float(projectile->state.damage));
                    world.applyImpulse(player->actor.bindings.body, glm2b(impulse));

                }
                    
                // al()
                break;
            }
            default:
                std::cerr << "wrong ActorType combination in beginEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.beginCount; i++) {
        // pl("begin")
        processCollisionEvent(contacts.beginEvents[i], beginProcessor);
    }
}
void LogicalScene::processEndEvents(b2ContactEvents contacts){ 
    auto endProcessor = [this](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::Scenery): {
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                player->state.touching_grass_counter--;
                //if end event, we are in fact not touching grass and should not refill
                player->state.refill_jumps_next_frame = false;
                break;
            }
            
            default:
                std::cerr << "wrong ActorType combination in endEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.endCount; i++) {
        // l()
        processCollisionEvent(contacts.endEvents[i], endProcessor);
    }
}
void LogicalScene::processHitEvents(b2ContactEvents contacts){ 
    auto contactProcessor = [](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        switch (typeA | typeB) {
            default:
                std::cerr << "wrong ActorType combination in hitEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.hitCount; i++) {
        // l()
        processCollisionEvent(contacts.hitEvents[i], contactProcessor);
    }
}

void LogicalScene::processMoveEvents(b2BodyEvents moves, double dTime){
    for(int i=0; i<moves.moveCount; i++){
        auto& move = moves.moveEvents[i];
        // Actually, it is pointer to Player/Scenery/Projectile, but Actor is thir first member
        Actor* actor = (Actor*)(move.userData); 
        assert(actor);
        actor->state.pos = b2glm(move.transform.p);
        actor->state.rot = b2glm(move.transform.q);

        actor->state.vel = world.getVelocity(actor->bindings.body);

        if(actor->actorType == ActorType::Player){
            Player* player = (Player*)actor; 

            vec2 mouse_wpos = (vec2((input.mousePosf)/dvec2(1920,1080))*2.f - 1.f) * view.camera.cameraScale;
            player->update(&world, (mouse_wpos));
            player->updateVisualPos(dTime);

        }
        if(actor->actorType == ActorType::Projectile){
            Projectile* bullet = (Projectile*)actor; 
            bullet->update(&world, dTime);
        }
    }
}

void LogicalScene::tick(double dTime) {
    input.pollUpdates();
    
if(!glfwGetKey(view.render.window.pointer, GLFW_KEY_RIGHT_SHIFT)){
    world.step(dTime);
    b2BodyEvents bodyEvents = world.GetBodyEvents();
    b2ContactEvents contacts = world.GetContactEvents();

// al()
    processBeginEvents(contacts);
// al()
    processEndEvents(contacts);
// al()
    processHitEvents(contacts);
// al()
    processMoveEvents(bodyEvents, dTime);
// al()

// al()
    // pl(body_garbage.size());
    for(auto& bg : body_garbage){
        b2DestroyBody(bg);
    } body_garbage.clear();
    projectiles.collectGarbage();
// al()
    players.collectGarbage();
// al()
    sceneries.collectGarbage();

    // Basically processes and removes dead ones
    int write_index = 0;
    for (int i = 0; i < particles.size(); i++) {
        //TODO:
        // bool should_keep = particles[i].shape.props.CIRCLE_radius > 0.001;
        bool should_keep = particles[i].shape.props.CIRCLE_radius > 0.0025; // TODO until visible
        if (should_keep) {
            particles[write_index] = particles[i];
            particles[write_index].shape.pos += particles[write_index].vel * float(dTime);
            // particles[write_index].lifetime -= dTime; // if enabled, then dTime/(0 /-value) might happen
            particles[write_index].shape.props.CIRCLE_radius *= (1.0-dTime/particles[write_index].lifetime);
            write_index++;
        }
    }
    particles.resize (write_index);
} // end of pause game for debug

    // -y to invert vertically
    view.camera.cameraScale = {1920/float(1080)*10.0, -1*10.0};

    view.start_frame();
        view.start_main_pass();

    Shape background = {};
        background.shapeType = Square;
        background.coloring_info = twpp::pink(200);
        background.props.SQUARE_half_height = 10.0;
        background.props.SQUARE_half_width = 1920/float(1080)*10.0;
    view.draw_dynamic_shape(background, WAVYstyle);
    
    // Draw everything. Basically reorders memory in specific buffers for GPU convenience
    //TODO static
    for(mut p : particles){
        p.draw(&view);
    }
    for(mut s : sceneries){
        s.draw(&view);
    }
    for(mut p : projectiles){
        p.draw(&view);
    }
    for(mut p : players){
        p.draw(&view);
        drawHpBar(&p);
    }

    view.end_main_pass();
    
    view.bloom_pass();
    view.end_frame();

}

Shape Actor::constructActorShape(){
    Shape shape = {};
        shape.coloring_info = properties.color;
        shape.pos = state.pos;
        shape.rot_angle = b2Rot_GetAngle({state.rot.x, state.rot.y});
        shape.rounding_radius = properties.rounding_radius;shape.props = shapeProps;
    return shape;
}


void LogicalScene::startNewRound(){
    removeAllProjectiles();
    removeAllScenery();
}

void LogicalScene::addParticle(u8vec3 color, vec2 pos, vec2 vel, float size, float lifetime){
    //TODO vel?
    Particle p = {};
        p.shape.coloring_info = color;
        p.shape.shapeType = Circle;
        p.shape.props = {.CIRCLE_radius = size};
        p.shape.pos = pos;
        p.vel = vel;
    p.lifetime = lifetime;

    particles.push_back(p);
}

//TODO c-rand vs c++-gen
//quality is not required, so fast rand i believe is better
//possible to simd. 
float randFloat(float min, float max) {
    float rnd = min + float(rand()) / (float(RAND_MAX / (max - min)));
    return rnd;
}
int randInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void LogicalScene::addEffect(
    u8vec3 baseColor, u8 colorVariation, 
    vec2 basePos, float posVariation, 
    vec2 baseVel, float velVariation, 
    float baseSize, float sizeVariation, 
    float baseLifetime, float lifetimeVariation, 
    int numParticles
) {
    for (int i = 0; i < numParticles; i++) {
        vec2 particlePos = basePos + vec2(randFloat(-posVariation, posVariation), randFloat(-posVariation, posVariation));
        vec2 particleVel = baseVel + vec2(randFloat(-velVariation, velVariation), randFloat(-velVariation, velVariation));

        u8vec3 modifiedColor = {
            glm::clamp(int(baseColor.x) + randInt(-colorVariation, colorVariation), 0, 255),
            glm::clamp(int(baseColor.y) + randInt(-colorVariation, colorVariation), 0, 255),
            glm::clamp(int(baseColor.z) + randInt(-colorVariation, colorVariation), 0, 255)
        };

        float particleSize = baseSize + randFloat(-sizeVariation, sizeVariation);
        float particleLifetime = baseLifetime + randFloat(-lifetimeVariation, lifetimeVariation);

        if(
            (particleLifetime > 0) &&
            (particleSize > 0) &&
            true
        ){
            addParticle(modifiedColor, particlePos, particleVel, particleSize, particleLifetime);
        }
    }
}

void Particle::draw(VisualView* view){
    view->draw_dynamic_shape(shape, SolidColor);
}
void Particle::update(double dTime){
    shape.props.CIRCLE_radius *= (1.0 - (double)dTime / (double)lifetime);
}

//TODO explosion
void LogicalScene::BulletSceneryHitEffect(Projectile* bullet, Scenery* scenery){
    //like smoke or smth
    float R = bullet->state.radius;
    addEffect(
        twpp::zinc(800), // Base color
        3,               // Color variation
        bullet->actor.state.pos, // Base position
        float(0),         // Position variation
        vec2(0),         // Base velocity
        float(R*2.0),     // Velocity variation
        R*1.1,           // Base size
        R*0.5,           // Size variation
        bullet->state.radius*2.0, // Base lifetime
        0.5,             // Lifetime variation
        7                // Number of particles
    );
    
    addEffect(
        bullet->actor.properties.color,  // Base color
        20,                              // Color variation
        bullet->actor.state.pos,         // Base position
        float(bullet->state.radius*2.0),  // Position variation
        vec2(0),                         // Base velocity
        float(1.7*bullet->state.radius),  // Velocity variation
        bullet->state.radius/2.0,        // Base size
        0.5,                             // Size variation
        bullet->state.radius*2.0,        // Base lifetime
        0.5,                             // Lifetime variation
        20                               // Number of particles
    );
}

void LogicalScene::BulletPlayerHitEffect(Projectile* bullet, Player* player){
    addEffect(
        player->actor.properties.color,   // Base color
        20,                               // Color variation
        (bullet->actor.state.pos + player->actor.state.pos) / 2.0f, // Base position (midpoint between bullet and player for more stability)
        (bullet->state.radius*2.0),   // Position variation
        vec2(0),                          // Base velocity
        (1.7*bullet->state.radius),   // Velocity variation
        player->props.radius / 5.0,       // Base size
        0.5,                              // Size variation
        bullet->state.radius*2.0,         // Base lifetime
        0.5,                              // Lifetime variation
        10                                // Number of particles
    );

    addEffect(
        bullet->actor.properties.color,   // Base color
        20,                               // Color variation
        bullet->actor.state.pos,          // Base position
        (bullet->state.radius*2.0),   // Position variation
        vec2(0),                          // Base velocity
        (1.7*bullet->state.radius),   // Velocity variation
        bullet->state.radius / 5.0,       // Base size
        0.5,                              // Size variation
        bullet->state.radius*2.0,         // Base lifetime
        0.5,                              // Lifetime variation
        10                                // Number of particles
    );}

void LogicalScene::BulletBullethitEffect(Projectile* bullet1, Projectile* bullet2){

}

void LogicalScene::PlayerJumpEffect(Player* player){
    vec2 player_bottom = player->actor.state.pos;
        player_bottom.y -= player->props.radius;

    addEffect(
        player->actor.properties.color,   // Base color
        20,                               // Color variation
        player_bottom,                    // Base position
        (player->props.radius / 2.0), // Position variation
        vec2(0),                          // Base velocity
        (player->props.radius),       // Velocity variation
        player->props.radius / 7.0,       // Base size
        0.5,                              // Size variation
        player->props.radius / 5.0,       // Base lifetime
        0.5,                              // Lifetime variation
        20                                // Number of particles
    );
}

void LogicalScene::PlayerDieEffect(Player* player){

}

void LogicalScene::PlayerSceneryHitEffect(Player* player, Scenery* scnery){

};

void LogicalScene::drawHpBar(Player* player) {
    float barWidth = player->props.radius*2.7;
    float barHeight = player->props.radius*0.08;
    float barYOffset = player->props.radius - 0.01;
    vec2 barPosition = player->visual_pos + vec2(0, player->props.radius + barYOffset);

    double maxHP = player->props.hp;
    double currentHP = player->state.hp_left;

    float filledBarWidth = float((currentHP / maxHP) * barWidth);

    Shape barBack = {};
        barBack.shapeType = Square;
        barBack.coloring_info = twpp::slate(800);
        barBack.props.SQUARE_half_width = (barWidth / 2.0f) * 1.1;
        barBack.props.SQUARE_half_height = (barHeight / 2.0f) * 1.1;
        barBack.pos = barPosition;

    Shape barFill = {};
        barFill.shapeType = Square;
        barFill.coloring_info = twpp::lime(600);
        barFill.props.SQUARE_half_width = filledBarWidth / 2.0f;
        barFill.props.SQUARE_half_height = barHeight / 2.0f;
        barFill.pos = barPosition - vec2((barWidth - filledBarWidth) / 2.0f, 0);

    view.draw_dynamic_shape(barBack, SolidColor);
    view.draw_dynamic_shape(barFill, SolidColor);
}