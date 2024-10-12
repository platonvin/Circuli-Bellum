#include "logic.hpp"
#include "macros.hpp"
#include <random>

#define println printf("%s:%d: Fun: %s\n", __FILE__, __LINE__, __FUNCTION__);
//TODO c-rand vs c++-gen
//quality is not required, so fast rand i believe is better
//possible to simd. 
float randFloat(float min, float max) {
    float rnd = min + float(rand()) / (float(RAND_MAX / (max - min)));
    assert(!std::isinf(max));
    assert(!std::isinf(min));
    assert(!std::isnan(min));
    assert(!std::isnan(max));
    return glm::clamp(rnd, min, max);
}
int randInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

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

void LogicalScene::addProjectile(Projectile projectile, float vel_var) {
    ListElem<Projectile>* new_e = projectiles.appendBack(projectile);
        new_e->obj()->actor.state.vel.x += randFloat(-vel_var, +vel_var);
    new_e->obj()->addToWorld(&world);
}

// Group of projectiles that do not collide with each other
void LogicalScene::addProjectilesPack(Projectile projectile, int count, float vel_var) {
    // General counter for unique group every time
    static int group = -1;
    for(int i=0; i < count; i++){
        ListElem<Projectile>* new_e = projectiles.appendBack(projectile);
            new_e->obj()->actor.state.vel.x += randFloat(-vel_var, +vel_var);
            new_e->obj()->actor.state.vel.y += randFloat(-vel_var, +vel_var);
        new_e->obj()->addToWorld(&world, group);
    }
    group--;
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
    Player player = {};
    player.actor.state.pos.y = 8.0;
    ListElem<Player>* new_e = players.appendBack(player);
    new_e->obj()->addToWorld(&world);
    new_e->obj()->resetState();
}
void LogicalScene::removeAllPlayers() {
    for (mut player : players) {
        b2DestroyBody(player.actor.bindings.body);
    }
    players.removeAll();
}
void LogicalScene::resetPlayersState() {
    //TODO
    for (mut player : players) {
        player.resetState();
        //re-add to the world? seems better to reset position
        //TODO reset pos, vel, etc.
    }
}

int LogicalScene::countActivePlayers() {
    int playerCount = 0;
    for(let p : players){
        if(p.state.lives_left > 0){
            playerCount++;
        }        
        // p.logState();
    }
    return playerCount;
}

void LogicalScene::endRound(){
    int id = rand() % std::size(cards);
    cards[id].printCard();
}

// there is no choise. DRAW A CARD
void LogicalScene::giveCardsToDeadPlayers(){
    for(mut p : players){
        int id = rand() % std::size(cards);
        if(p.state.lives_left == 0){
            p.drawCard(&cards[id]);
            cards[id].printCard();
        }
    }
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

        addScenery(Scenery(Rectangle, {.RECTANGLE_half_width=half_width, .RECTANGLE_half_height=half_height}, position, false));

        current_x += 2.0 * half_width + gap_between_rectangles;
        if (abs(current_x) > bounds) {
            current_x = -bounds;
            current_y += half_height + 2.5 + gap_between_rectangles;
        }
    }
}

void LogicalScene::genBorderScenery(){
    auto addborder = [this](Scenery scenery){
        ListElem<Scenery>* new_e = borders.appendBack(scenery);
            new_e->obj()->actor.actorType = ActorType::Border;
        new_e->obj()->addToWorld(&world, ActorType::Border);
    };

    const float border_hw = 0.5;
    const float visible_w = 0.05;

    addborder(Scenery(Rectangle, 
        {.RECTANGLE_half_width = scene_hsize.x, .RECTANGLE_half_height = border_hw},
        vec2(0, - (scene_hsize.y + border_hw - visible_w)), false));
    addborder(Scenery(Rectangle, 
        {.RECTANGLE_half_width = scene_hsize.x, .RECTANGLE_half_height = border_hw},
        vec2(0, + (scene_hsize.y + border_hw - visible_w)), false));

    addborder(Scenery(Rectangle, 
        {.RECTANGLE_half_width = border_hw, .RECTANGLE_half_height = scene_hsize.y},
        vec2( - (scene_hsize.x + border_hw - visible_w), 0), false));
    addborder(Scenery(Rectangle, 
        {.RECTANGLE_half_width = border_hw, .RECTANGLE_half_height = scene_hsize.y},
        vec2( + (scene_hsize.x + border_hw - visible_w), 0), false));
    
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
            PlayerJumpEffect(slave);
            slave->processJump(&world);
            earth_shake+=5.1;
        }
    );

    input.setActionCallback(Action::Shoot, 
        [this](Action action) -> void {
            //if can shoot
            if(slave->state.time_since_last_shot > slave->props.shot_delay){
                if(slave->state.ammunition_left > 0){
                    slave->state.time_since_last_shot = 0;
                    slave->state.ammunition_left--;
                    slave->state.time_since_last_reload = 0;
                    //TODO: restructure
                    // Player* p = players._start->obj();
                    Projectile bullet = {};
                        // bullet.props = slave->createBullet();
                    bullet.setupFromPlayer(&slave->state, &slave->props, &slave->actor);
                    if(slave->props.bullets_per_shot > 1){
                        addProjectilesPack(bullet, slave->props.bullets_per_shot, slave->props.bullet_speed*slave->props.spread);
                    } else {
                        addProjectile(bullet, slave->props.bullet_speed*slave->props.spread);
                    }

                }
            }
            
        }
    );
    input.setActionCallback(Action::DrawRNDcard, 
        [this](Action action) -> void {
            // int id = rand() % std::size(cards);
            // slave->drawCard(&cards[id]);
            // cards[id].printCard();
            (rand()%3!=0) ? slave->drawCard(&Spray) : slave->drawCard(&Buckshot);
            (rand()%5==0) ? slave->drawCard(&Fastball) : (void());
            slave->drawCard(&Trickster);
        }
    );
    input.setActionCallback(Action::MoveLeft, 
        [this](Action action) -> void {
            vec2 old_vel = world.getVelocity(slave->actor.bindings.body);
            if(old_vel.x > -1.0){
                // world.setVelocity(slave->actor.bindings.body, {-1, old_vel.y});
                world.applyImpulse(slave->actor.bindings.body, {-1, 0});
            }
            world.applyForce(slave->actor.bindings.body, 
                b2Vec2(-20.f,0.1));
            
        // al()
        }
    );
    input.setActionCallback(Action::MoveRight, 
        [this](Action action) -> void {
            vec2 old_vel = world.getVelocity(slave->actor.bindings.body);
            if(old_vel.x < 1.0){
                world.applyImpulse(slave->actor.bindings.body, {+1, 0});
                // world.setVelocity(slave->actor.bindings.body, {1, old_vel.y});
            }
            world.applyForce(slave->actor.bindings.body, 
                b2Vec2(+20.f,0.1));
        // al()
        }
    );
}

    extern int main();
void LogicalScene::setup(int player_count) {
    srand((long long)&main);
    
    world.setup();
    view.setup(); 

    genBorderScenery();
    
    for(int i=0; i<player_count; i++){
        addNewPlayer();
    }

    //TODO
    slave = players._start->obj();

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
    auto beginProcessor = [this](ActorType typeA, void* udataA, ActorType typeB, void* udataB, vec2 normal) {
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::StaticScenery): {
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                player->state.touching_grass_counter++;
                break;
            }
            case (ActorType::PlayerLeg | ActorType::StaticScenery): {
                Player* player = (Player*)(
                    ((typeA == ActorType::PlayerLeg) ? (char*)(udataA) : (char*)(udataB)) - offsetof(Player, leg) + offsetof(Player, actor));
                player->state.touching_grass_counter++;
                break;
            }
            case (ActorType::Player | ActorType::Border): {
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                player->processBorderTouch(&world, normal);
                break;
            }
            case (ActorType::PlayerLeg | ActorType::Border): {
                Player* player = (Player*)(
                    ((typeA == ActorType::PlayerLeg) ? (char*)(udataA) : (char*)(udataB)) - offsetof(Player, leg) + offsetof(Player, actor));
                    player->processBorderTouch(&world, normal);
                break;
            }
            case (ActorType::Projectile | ActorType::StaticScenery): {
                Projectile* projectile = (typeA == ActorType::Projectile) ? (Projectile*)(udataA) : (Projectile*)(udataB);
                Scenery* scenery = (typeA == ActorType::StaticScenery) ? (Scenery*)(udataA) : (Scenery*)(udataB);
                //possibly damage scenery, bounce/kill bullet
                BulletSceneryHitEffect(projectile, scenery);
                bool survived = projectile->processSceneryHit(&scenery->state);
                if(!survived){
                    body_garbage.insert(projectile->actor.bindings.body);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                    earth_shake += log(projectile->state.damage+1.0);
                }
                break;
            }
            case (ActorType::Projectile | ActorType::Projectile): {
                //TODO:
                Projectile* projectileA = (Projectile*)(udataA);
                Projectile* projectileB = (Projectile*)(udataB);

                if((projectileA->time_elapsed > 0) && (projectileB->time_elapsed > 0)){
                    body_garbage.insert(projectileA->actor.bindings.body);
                    body_garbage.insert(projectileB->actor.bindings.body);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectileA);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectileB);
                }
                break;
            }
            case (ActorType::Player | ActorType::Projectile): {
                Projectile* projectile = (typeA == ActorType::Projectile) ? (Projectile*)(udataA) : (Projectile*)(udataB);
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);

                bool survived = projectile->processPlayerHit(&player->state);
                if(!survived){
                    body_garbage.insert(projectile->actor.bindings.body);
                    // body_garbage_annotations.push_back("c:\\prog\\Circuli Bellum\\src\\logic.cpp: 312");
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                    BulletPlayerHitEffect(projectile, player);
                }
                    player->processBulletHit(&world, &projectile->state);
                    vec2 impulse = 
                        normalize(projectile->actor.state.vel) * player->props.hit_knockback;
                    world.applyImpulse(player->actor.bindings.body, glm2b(impulse));
                if(player == slave){
                    blast_shake += projectile->state.damage;
                }
                    
                break;
            }
            // case (ActorType::PlayerLeg | ActorType::Projectile): {
            // }
            // case (ActorType::PlayerLeg | ActorType::Scenery): {
            //     pl(79)
            // }
            default:
                // pl("wrong ActorType combination in beginEvent");
                break;
        }
    };

    for (int i = 0; i < contacts.beginCount; i++) {
        // pl("begin")
        processCollisionEvent(contacts.beginEvents[i], beginProcessor);
    }
}
void LogicalScene::processEndEvents(b2ContactEvents contacts){ 
    auto endProcessor = [this](ActorType typeA, void* udataA, ActorType typeB, void* udataB, vec2 normal) {
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::StaticScenery): {
                Player* player = (typeA == ActorType::Player) ? (Player*)(udataA) : (Player*)(udataB);
                // pl(player->state.touching_grass_counter)
                // pl(player)
                player->state.touching_grass_counter--;
                //if end event, we are in fact not touching grass and should not refill
                player->state.refill_body_jumps_next_frame = false;
                // pl(player->state.touching_grass_counter)
                break;
            }
            case (ActorType::PlayerLeg | ActorType::StaticScenery): {
                Player* player = (Player*)(
                    ((typeA == ActorType::PlayerLeg) ? (char*)(udataA) : (char*)(udataB)) - offsetof(Player, leg) + offsetof(Player, actor));
                // pl(player->state.touching_grass_counter)
                // pl(player)
                player->state.touching_grass_counter--;
                // player->state.refill_leg_jumps_next_frame = false;
                player->state.refill_body_jumps_next_frame = false;
                // pl("finsh")
                // pl(player->state.touching_grass_counter)
                break;
            }
            default:
                // std::cerr << "wrong ActorType combination in endEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.endCount; i++) {
        // l()
        processCollisionEvent(contacts.endEvents[i], endProcessor);
    }
}
void LogicalScene::processHitEvents(b2ContactEvents contacts){ 
    auto contactProcessor = [](ActorType typeA, void* udataA, ActorType typeB, void* udataB, vec2 normal) {
        switch (typeA | typeB) {
            default:
                // std::cerr << "wrong ActorType combination in hitEvent\n";
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
        // al()
        auto& move = moves.moveEvents[i];
        // Actually, it is pointer to Player/Scenery/Projectile, but Actor is thir first member
        Actor* actor = (Actor*)(move.userData); 
        if(!actor) continue;
        // assert(actor);
        // al()
        actor->state.pos = b2glm(move.transform.p);
        // al()
        actor->state.rot = b2glm(move.transform.q);
        // al()

        // al()
        // pl((long long) actor)
        // pl(actor->bindings.body.index1)
        // pl(actor->bindings.body.world0)
        // pl(actor->bindings.body.revision)
        actor->state.vel = world.getVelocity(actor->bindings.body);
        // al()
        if(actor->actorType == ActorType::Player){
            Player* player = (Player*)actor; 
            // pl(player->state.touching_grass_counter);

            vec2 mouse_wpos = (vec2((input.mousePosf)/dvec2(view.render.swapChainExtent.width,view.render.swapChainExtent.height))*2.f - 1.f) * view.camera.cameraScale;
            player->update(&world, dTime, (mouse_wpos));
            player->updateVisualPos(dTime);

        }
        // al()
        if(actor->actorType == ActorType::PlayerLeg){
            Player* player = (Player*)(actor) - offsetof(Player, leg) + offsetof(Player, actor);
            // pl(player->state.touching_grass_counter);
            // player->update(&world, (mouse_wpos));
            // player->updateVisualPos(dTime);

        }
        // al()
        if(actor->actorType == ActorType::Projectile){
            Projectile* bullet = (Projectile*)actor; 
            bullet->update(&world, dTime);
        }
    }
}

void LogicalScene::tick(double dTime) {
    input.pollUpdates();
    
    //here for debug draw
    view.start_frame();
        view.start_main_pass();
        Shape background = {};
            background.shapeType = Rectangle;
            background.coloring_info = twpp::slate(950);
            background.props.RECTANGLE_half_width = scene_hsize.x;
            background.props.RECTANGLE_half_height = scene_hsize.y;
        // view.draw_dynamic_shape(background, WAVYstyle);
        view.draw_background(background, WAVYstyle);
        
    if(!glfwGetKey(view.render.window.pointer, GLFW_KEY_RIGHT_SHIFT)){
        world.step(dTime);
        b2BodyEvents bodyEvents = world.GetBodyEvents();
        b2ContactEvents contacts = world.GetContactEvents();

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
        projectiles.collectGarbage();
    // al()
        players.collectGarbage();
    // al()
        sceneries.collectGarbage();
    // al()
        for(auto& bg : body_garbage){
            // let msg = body_garbage_annotations[body_garbage_annotations.size()-1];
            // if(!b2Body_IsValid(bg)){
            //     std::cout << msg;
            // }
            assert(b2Body_IsValid(bg));
            // body_garbage_annotations.pop_back();
            b2DestroyBody(bg);
        } body_garbage.clear();

        //soft limit
        const float TARGET_PARTICLE_COUNT = 10000; //maybe thats too little
        // const float    MAX_PARTICLE_COUNT = 100; //maybe thats too little
        //higher bias -> particles removed earlier  
        float current_bias = 0.006;
        if(particles.size() > TARGET_PARTICLE_COUNT){
            float ratio = particles.size() / TARGET_PARTICLE_COUNT; //>1
            //square for aggressive clamping
            //TODO: clamp particle count?
            current_bias = current_bias * ratio*ratio;
        }
        // avg_radius = 0;

        // pl(current_bias)
        // pl(particles.size())
        
        // Basically processes and removes dead ones
        int write_index = 0;
        for (int i = 0; i < particles.size(); i++) {
            //TODO:
            // bool should_keep = particles[i].shape.props.CIRCLE_radius > 0.001;
            bool should_keep = particles[i].shape.props.CIRCLE_radius > current_bias; // TODO until visible
            // avg_radius += particles[i].shape.props.CIRCLE_radius;
            if (should_keep) {
                particles[write_index] = particles[i];
                particles[write_index].shape.pos += particles[write_index].vel * float(dTime);
                // particles[write_index].lifetime -= dTime; // if enabled, then dTime/(0 /-value) might happen
                particles[write_index].shape.props.CIRCLE_radius *= exp(-dTime/particles[write_index].lifetime);
                write_index++;
            }
        }
        // avg_radius /= float(write_index);
        particles.resize (write_index);

        int activePlayers = countActivePlayers();
        if(activePlayers < 2) {
            endRound();
            giveCardsToDeadPlayers();
            startNewRound();
        }
    } // end of pause game for debug
    
    // smoothing chromatic abberation
    const float springForce = -20.f * chromatic_abb;
    const float dampingForce = -5.0 * blast_shake;
    const float totalForce = springForce + dampingForce;
    blast_shake += totalForce * dTime;
    chromatic_abb += blast_shake * dTime;

    // -y to invert vertically
    view.camera.cameraScale = {scene_hsize.x, -scene_hsize.y};
    view.ubo_cpu.chrom_abb = 1.f * vec2(chromatic_abb) / vec2(view.ubo_cpu.res);
    view.ubo_cpu.light_pos = vec2(0,12) + light_pendulum.get_pendulum_position2();
    light_pendulum.apply_impulse(earth_shake, 0);
    earth_shake=0;
    light_pendulum.simulate(dTime);
    
    // view.draw_shadows();
    // Draw everything. Basically reorders memory in specific buffers for GPU convenience
    //TODO static
    for(mut b : borders){
        b.draw(&view, FBMstyle);
    }
    for(mut p : particles){
        p.draw(&view);
    }
    for(mut s : sceneries){
        s.draw(&view, WAVYstyle);
    }
    for(mut p : projectiles){
        p.draw(&view);
    }
    for(mut p : players){
        p.draw(&view);
    }

    view.end_main_pass();
    
    view.bloom_pass();
    view.end_frame();

}

void LogicalScene::startNewRound(){
    removeAllProjectiles();
    // removeAllScenery();
    resetPlayersState();
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

        float particleSize = glm::max(baseSize + randFloat(-sizeVariation, sizeVariation), 0.01f);
        float particleLifetime = glm::max(baseLifetime + randFloat(-lifetimeVariation, lifetimeVariation), 0.01f);

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
        float(0),        // Position variation
        vec2(0),         // Base velocity
        float(R*1.0),    // Velocity variation
        R*1.1,           // Base size
        R*0.5,           // Size variation
        1.6,             // Base lifetime
        0.5,             // Lifetime variation
        7                // Number of particles
    );
    
    addEffect(
        bullet->actor.properties.color,  // Base color
        20,                              // Color variation
        bullet->actor.state.pos,         // Base position
        float(R*2.0), // Position variation
        vec2(0),      // Base velocity
        float(1.7*R), // Velocity variation
        R/2.0,        // Base size
        R/3.0,        // Size variation
        1.4,          // Base lifetime
        0.5,          // Lifetime variation
        20            // Number of particles
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
        (player->props.radius / 2.0),     // Position variation
        vec2(0),                          // Base velocity
        (player->props.radius),           // Velocity variation
        player->props.radius / 7.0,       // Base size
        player->props.radius / 8.0,       // Size variation
        player->props.radius / 5.0,       // Base lifetime
        0.5,                              // Lifetime variation
        20                                // Number of particles
    );
}

void LogicalScene::PlayerDieEffect(Player* player){

}

void LogicalScene::PlayerSceneryHitEffect(Player* player, Scenery* scnery){

};