#include "logic.hpp"
#include "twpp.hpp"
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
    b2Circle player_circle = {};
        player_circle.radius = player->props.radius;
    assert(player);
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
    player.actor.state.pos.y = 12.0;
    player.props.radius = 1.0;
        apl((int)player.actor.properties.color.r)
        apl((int)player.actor.properties.color.g)
        apl((int)player.actor.properties.color.b)
    ListElem<Player>* new_player = players.appendBack(player);
    addPlayerToWorld(new_player->obj());
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
            world.applyImpulse(players._start->obj()->actor.bindings.body, 
                b2Vec2(0,20.0));
        al()
        }
    );
    input.setActionCallback(Action::MoveLeft, 
        [this](Action action) -> void {
            world.applyForce(players._start->obj()->actor.bindings.body, 
                b2Vec2(-20.f,0.1));
            
        al()
        }
    );
    input.setActionCallback(Action::MoveRight, 
        [this](Action action) -> void {
            world.applyForce(players._start->obj()->actor.bindings.body, 
                b2Vec2(+20.f,0.1));
        al()
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
}

void LogicalScene::tick(float dTime) {
    //first to be more responsive
    input.pollUpdates();
    
    world.step(dTime);

l()
    b2BodyEvents bodyEvents = world.GetBodyEvents();
    b2ContactEvents contacts = world.GetContactEvents();
l()

    for(int i=0; i<contacts.beginCount; i++){
l()
        auto touch = contacts.beginEvents[i];
l() 
        std::cout <<"A: "<< touch.shapeIdA.index1 <<" B: " << touch.shapeIdB.index1 <<'\n';
        assert(b2Shape_GetUserData(touch.shapeIdA));
        assert(b2Shape_GetUserData(touch.shapeIdB));
        ActorType typeA = ((Actor*)b2Shape_GetUserData(touch.shapeIdA))->actorType;
l()
        ActorType typeB = ((Actor*)b2Shape_GetUserData(touch.shapeIdB))->actorType;
l()
        std::cout <<"BEGIN: "<< typeA <<" : " << typeB <<'\n';
l()
        switch (typeA | typeB) {
            case (ActorType::Player|ActorType::Player):{break;}
            case (ActorType::Scenery|ActorType::Scenery):{break;}
            case (ActorType::Projectile|ActorType::Projectile):{
                //destroy both | weak one
                break;
            }

            case (ActorType::Player|ActorType::Scenery):{
                //extra knockbak to player? Stun
                // b2Shape_GetBody(touch.shapeIdA);
                // b2Body_GetContactData()
                break;
            }
            case (ActorType::Projectile|ActorType::Player):{
                //dmg+destroy / shield bounce
                break;
            }
            case (ActorType::Projectile|ActorType::Scenery):{
                //destroy / bounce
                break;
            }
            default:
                assert(false && "WRONG AT|AT");
        }        
    }

l()
    for(int i=0; i<bodyEvents.moveCount; i++){
        auto& move = bodyEvents.moveEvents[i];
        // Actually, it is pointer to Player/Scenery/Projectile, but Actor is thir first member
        Actor* actor = (Actor*)move.userData; 
        assert(actor);
        actor->state.pos = b2glm(move.transform.p);
        actor->state.rot = b2glm(move.transform.q);

        //limiting speed. TODO move away
        if(actor->actorType == ActorType::Player){
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
        }
    }

l()
    //TODO
    // -y to invert vertically
    view.camera.cameraScale = {1920/float(1080)*10.0, -1*10.0};

    view.start_frame();
        view.start_main_pass();

l()
    // Draw everything. Basically reorders memory in specific buffers for GPU convenience
    for(auto p : players){
        view.draw_dynamic_shape(p.constructShape(), SolidColor);
    }
    for(auto s : sceneries){
        view.draw_dynamic_shape(s.constructShape(), SolidColor);
    }
    for(auto p : projectiles){
        view.draw_dynamic_shape(p.constructShape(), SolidColor);
    }
l()

    view.end_main_pass();
    
    // view.bloom_pass();
// println
    view.end_frame();
// println
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
