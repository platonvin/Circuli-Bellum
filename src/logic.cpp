#include "logic.hpp"
#include "macros.hpp"

#define println printf("%s:%d: Fun: %s\n", __FILE__, __LINE__, __FUNCTION__);

void LogicalScene::addScenery(Scenery scenery){
    ListElem<Scenery>* new_scenery = sceneries.appendBack(scenery);
    addSceneryToWorld(new_scenery->obj(), {});
}
void LogicalScene::removeScenery(ListElem<Scenery>* scenery){
    sceneries.removeElem(scenery);
    b2DestroyBody(scenery->obj()->actor.bindings.body); //TODO
}
void LogicalScene::removeAllScenery(){
    for(auto& s : sceneries){
        b2DestroyBody(s.actor.bindings.body);
    }
    sceneries.removeAll();
}
void LogicalScene::addProjectile(Projectile projectile){
    ListElem<Projectile>* new_projectile = projectiles.appendBack(projectile);
    addProjectileToWorld(new_projectile->obj());
}
void LogicalScene::removeProjectile(ListElem<Projectile>* projectile){
    projectiles.removeElem(projectile);
    b2DestroyBody(projectile->obj()->actor.bindings.body); //TODO
}
void LogicalScene::removeAllProjectiles(){
    for(auto& s : projectiles){
        b2DestroyBody(s.actor.bindings.body);
    }
    projectiles.removeAll();
}

// when world is cleared and new one created
// players travel between worlds, everything else doesnt
void LogicalScene::addPlayerToWorld(Player* player) {
    assert(player);
    b2Circle player_circle = {};
        player_circle.radius = player->props.radius;
    // b2BodyDef bodyDef = b2DefaultBodyDef();
    //     bodyDef.type = player->actor.properties.body_type;
    //     bodyDef.position = glm2b(player->actor.state.pos);
    // b2ShapeDef shapeDef = b2DefaultShapeDef();
    //     shapeDef.enableHitEvents = true;

    world.addActor<b2Circle, b2CreateCircleShape>(&player->actor.bindings, &player->actor.state, &player->actor.properties, player, &player_circle);
}
void LogicalScene::addSceneryToWorld(Scenery* scenery, const std::vector<b2Vec2>& vertices) {
    //for now, just box ¯\_(ツ)_/¯
l()
    //TODO:
    b2Polygon poly = b2MakeBox(scenery->actor.shapeProps.SQUARE_half_width, scenery->actor.shapeProps.SQUARE_half_height);
l()
    world.addActor<b2Polygon, b2CreatePolygonShape>(&scenery->actor.bindings, &scenery->actor.state, &scenery->actor.properties, scenery, &poly);
l()
}
void LogicalScene::addProjectileToWorld(Projectile* projectile) {
    //TODO capsule
    b2Circle bullet_circle = {};
        bullet_circle.radius = projectile->props.radius;
    world.addActor<b2Circle, b2CreateCircleShape>(&projectile->actor.bindings, &projectile->actor.state, &projectile->actor.properties, projectile, &bullet_circle);
}

void LogicalScene::addNewPlayer(/*some visual props*/){
    Player player = Player(1, 4.2);
    player.actor.state.pos.y = 8.0;
    player.props.radius = 1.0;
        apl((int)player.actor.state.pos.x)
        apl((int)player.actor.state.pos.y)
        // apl((int)player.actor.properties.color.r)
        // apl((int)player.actor.properties.color.g)
        // apl((int)player.actor.properties.color.b)
    // pl((void*)(players.end()))
    // pl((void*)(players.begin()))
    // pl(players.end() - players.begin())
    ListElem<Player>* new_player = players.appendBack(player);
    // pl(new_player->obj())
    addPlayerToWorld(new_player->obj());
    // pl((void*)(players.end()))
    // pl((void*)(players.begin()))
    // pl(players.end() - players.begin())
    players.printList();
    // abort();
}
void LogicalScene::removeAllPlayers(){
    for(auto& p : players){
        b2DestroyBody(p.actor.bindings.body);
    }
    players.removeAll();
}

void LogicalScene::clearWorld(void) {
    //TODO destroy ents?
    removeAllScenery();
    removeAllProjectiles();
    world.destroy(); 
    world.create();
}

void LogicalScene::setupActionCallbacks() {
    // input.rebindKey(Action::Jump, GLFW_KEY_SPACE);
    input.setActionCallback(Action::Jump, 
        [this](Action action) -> void {
            Player* p = players._start->obj();
            // if(p->state.jumps_left > 0){
            if(p->state.jumps_left > 0){
                world.applyImpulse(p->actor.bindings.body, 
                    b2Vec2(0,20.0));
                auto t = b2Body_GetTransform(p->actor.bindings.body);
                // t.p.y += 1.1;
                b2Body_SetTransform(p->actor.bindings.body, t.p, t.q);
                npl("JUMP");
                // b2set
                // p->state.jumps_left--;
                p->state.jumps_left--;

                //so not updated from previous
                p->state.refill_jumps_next_frame = false;
            }
        }
    );
    input.setActionCallback(Action::MoveLeft, 
        [this](Action action) -> void {
            world.applyForce(players._start->obj()->actor.bindings.body, 
                b2Vec2(-20.f,0.1));
            
        // al()
        }
    );
    input.setActionCallback(Action::MoveRight, 
        [this](Action action) -> void {
            world.applyForce(players._start->obj()->actor.bindings.body, 
                b2Vec2(+20.f,0.1));
        // al()
        }
    );
}
void LogicalScene::create(int player_count) {
    world.create();
    view.init(); 

    for(int i=0; i<player_count; i++){
        addNewPlayer();
    }

    input.setup(view.render.window.pointer);

    setupActionCallbacks();
}

void LogicalScene::destroy(void) {
    //TODO check b2
    removeAllPlayers();
    removeAllScenery();
    removeAllProjectiles();
    world.destroy();
    view.cleanup();
    input.cleanup();
}

void LogicalScene::processBeginEvents(b2ContactEvents contacts){ 
    auto beginProcessor = [](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        pl(typeA);
        pl(typeB);
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::Scenery): {
                Player* player = (typeA == ActorType::Player) ? (Player*)udataA : (Player*)udataB;
                player->state.touching_grass = true;
                // player->state.last_jmp_refill = glfwGetTime(); //TODO?
                break;
            }
            default:
                std::cerr << "wrong ActorType combination in beginEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.beginCount; ++i) {
        pl("begin")
        processCollisionEvent(contacts.beginEvents[i], beginProcessor);
    }
}
void LogicalScene::processEndEvents(b2ContactEvents contacts){ 
    auto endProcessor = [](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        pl(typeA);
        pl(typeB);
        switch (typeA | typeB) {
            case (ActorType::Player | ActorType::Scenery): {
                Player* player = (typeA == ActorType::Player) ? (Player*)udataA : (Player*)udataB;
                player->state.touching_grass = false;
                //if end event, we are in fact not touching grass and should not refill
                player->state.refill_jumps_next_frame = false;
                break;
            }
            default:
                std::cerr << "wrong ActorType combination in endEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.endCount; ++i) {
        pl("end")
        // l()
        processCollisionEvent(contacts.endEvents[i], endProcessor);
    }
}
void LogicalScene::processHitEvents(b2ContactEvents contacts){ 
    auto contactProcessor = [](ActorType typeA, void* udataA, ActorType typeB, void* udataB) {
        pl(typeA);
        pl(typeB);

        switch (typeA | typeB) {
            default:
                std::cerr << "wrong ActorType combination in hitEvent\n";
                break;
        }
    };

    for (int i = 0; i < contacts.hitCount; ++i) {
        // l()
        processCollisionEvent(contacts.hitEvents[i], contactProcessor);
    }
}

void LogicalScene::processMoveEvents(b2BodyEvents moves){
    for(int i=0; i<moves.moveCount; i++){
        auto& move = moves.moveEvents[i];
        // Actually, it is pointer to Player/Scenery/Projectile, but Actor is thir first member
        Actor* actor = (Actor*)move.userData; 
        assert(actor);
        actor->state.pos = b2glm(move.transform.p);
        actor->state.rot = b2glm(move.transform.q);

        // pl(actor->actorType);
        pl(actor->actorType);
        //updating player TODO move away
        if(actor->actorType == ActorType::Player){
            Player* player = (Player*)actor; 

            vec2 current_vel = world.getVelocity(actor->bindings.body);
            vec2 desired_vel = vec2(1);
            float current_len = glm::length(current_vel);
            float desired_len = glm::length(desired_vel);
            if(current_len > desired_len){
                vec2 force_direction = -glm::normalize(current_vel);
                float ldiff = (current_len - desired_len);
                world.applyForce(players._start->obj()->actor.bindings.body, 
                    glm2b(force_direction*ldiff*2.0f));
            }
            pl(player->state.touching_grass);
            // pl(player->state.jumps_left);
            pl("after");

            //like its next frame. TODO: box2d end event on the same tick?
            if(player->state.refill_jumps_next_frame){
                player->state.jumps_left = player->props.max_jumps;
                player->state.refill_jumps_next_frame = false;
            }
            if(player->state.touching_grass){
                player->state.refill_jumps_next_frame = true;

            }
            pl(player->state.touching_grass);
            // pl(player->state.jumps_left);
            pl(player->state.refill_jumps_next_frame);
        }
        /*
        what i want is:
        jump refilled whhen:
            in contact with surface and not jumped away from it
            so, basically
        */
    }
}
/*
input: jump
no jumps

on grass ? +1 : _

end event:
    
*/

void LogicalScene::tick(float dTime) {
    //first to be more responsive
    if(glfwGetKey(view.render.window.pointer, GLFW_KEY_ENTER) == GLFW_PRESS){
        pl("tick start");
        input.pollUpdates();
        
        world.step(dTime);
        b2BodyEvents bodyEvents = world.GetBodyEvents();
        b2ContactEvents contacts = world.GetContactEvents();
        // pl(players._start->obj()->state.has_jump);
        pl(players._start->obj()->state.touching_grass);
        processBeginEvents(contacts);
        pl(players._start->obj()->state.touching_grass);
        processEndEvents(contacts);
        processHitEvents(contacts);
        pl(players._start->obj()->state.touching_grass);
        processMoveEvents(bodyEvents);
        pl(players._start->obj()->state.refill_jumps_next_frame);


        // -y to invert vertically
        view.camera.cameraScale = {1920/float(1080)*10.0, -1*10.0};

        view.start_frame();
            view.start_main_pass();

    // l()
        // Draw everything. Basically reorders memory in specific buffers for GPU convenience
        for(let p : players){
            view.draw_dynamic_shape(p.constructShape(), SolidColor);
        }
        for(let s : sceneries){
            view.draw_dynamic_shape(s.constructShape(), SolidColor);
        }
        for(let p : projectiles){
            view.draw_dynamic_shape(p.constructShape(), SolidColor);
        }
    // l()

        view.end_main_pass();
        
        // view.bloom_pass();
    // println
        view.end_frame();
    // println
        pl("tick end\n");

        /*
        in contact
        if has jump:    
            jump
            now has NO jump
        still in contact
        */
    }
}

Shape Actor::constructActorShape(){
    Shape shape = {};
        shape.coloring_info = properties.color;
        shape.pos = state.pos;
        shape.props = shapeProps;
    return shape;
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
Shape Scenery::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Square;
        shape.shapeType = convertShapeType(actor.properties.shape_type);
        //TODO?
    return shape;
}
Shape Projectile::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Circle;
        //overwrites
        shape.props.CIRCLE_radius = props.radius;
    return shape;
}
