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
    for (let scenery : sceneries) {
        b2DestroyBody(scenery.actor.bindings.body);
    }
    projectiles.removeAll();
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
    for (let projectile : projectiles) {
        b2DestroyBody(projectile.actor.bindings.body);
    }
    projectiles.removeAll();
}

// when world is cleared and new one created
// players travel between worlds, everything else doesnt

void LogicalScene::addNewPlayer(/*some visual props*/) {
    Player player = Player(1, 2.2);
    player.actor.state.pos.y = 8.0;
    player.props.radius = 1.0;
    ListElem<Player>* new_e = players.appendBack(player);
    new_e->obj()->addToWorld(&world);
}
void LogicalScene::removeAllPlayers() {
    for (let player : players) {
        b2DestroyBody(player.actor.bindings.body);
    }
    players.removeAll();
}
void LogicalScene::resetPlayersState() {
    for (let player : players) {
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
                world.applyImpulse(controlled_player->actor.bindings.body, 
                    b2Vec2(0,20.0));
                auto t = b2Body_GetTransform(controlled_player->actor.bindings.body);
                t.p.y += 0.001;
                b2Body_SetTransform(controlled_player->actor.bindings.body, t.p, t.q);
                controlled_player->state.jumps_left--;

                //so not updated from previous
                controlled_player->state.refill_jumps_next_frame = false;
            }
        }
    );
    input.setActionCallback(Action::Shoot, 
        [this](Action action) -> void {
            static int counter=0;
            counter++;
            pl("shoot")
            pl(counter)
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
            world.applyForce(controlled_player->actor.bindings.body, 
                b2Vec2(-20.f,0.1));
            
        // al()
        }
    );
    input.setActionCallback(Action::MoveRight, 
        [this](Action action) -> void {
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

                bool survived = projectile->processPlayerHit(&player->state);
                if(!survived){
                    body_garbage.push_back(projectile->actor.bindings.body);
                    projectiles.softRemoveElem((ListElem<Projectile>*)projectile);
                }
                if(player != controlled_player){
                    player->processBulletHit(&world, &projectile->state);
                    vec2 impulse = 
                        normalize(projectile->actor.state.vel) *
                        float(projectile->state.damage);
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

void LogicalScene::processMoveEvents(b2BodyEvents moves, float dTime){
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
        }
        if(actor->actorType == ActorType::Projectile){
            Projectile* bullet = (Projectile*)actor; 
            bullet->update(&world, dTime);
        }
    }
}

void LogicalScene::tick(float dTime) {
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
        bool should_keep = particles[i].shape.props.CIRCLE_radius > 0.001;
        if (should_keep) {
            particles[write_index] = particles[i];
            particles[write_index].lifetime -= dTime;
            particles[write_index].shape.props.CIRCLE_radius *= (1-dTime/particles[write_index].lifetime);
            write_index++;
        }
    }
    particles.resize (write_index);
} // end of pause game for debug

    // -y to invert vertically
    view.camera.cameraScale = {1920/float(1080)*10.0, -1*10.0};

    view.start_frame();
        view.start_main_pass();

    // Draw everything. Basically reorders memory in specific buffers for GPU convenience
    //TODO static
    
    for(let p : players){
        p.draw(&view);
    }
    for(let s : sceneries){
        s.draw(&view);
    }
    for(let p : projectiles){
        p.draw(&view);
    }
    for(let p : particles){
        p.draw(&view);
    }

    view.end_main_pass();
    
    // view.bloom_pass();
    view.end_frame();

}

Shape Actor::constructActorShape(){
    Shape shape = {};
        shape.coloring_info = properties.color;
        shape.pos = state.pos;
        shape.rot = state.rot;
        shape.props = shapeProps;
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
    p.lifetime = lifetime;
}

void Particle::draw(VisualView* view){
    view->draw_dynamic_shape(shape, SolidColor);
}
void Particle::update(float dTime){
    shape.props.CIRCLE_radius *= (1.0 - dTime / lifetime);
}