#include "logic.hpp"
#define println printf("%s:%d: Fun: %s\n", __FILE__, __LINE__, __FUNCTION__);

void LogicalScene::addPlayer(const b2Vec2& position, float radius, float mass) {
    Player* player = new Player();
    // Players are round :)
    player->create();
    tie(player->body, player->shape) = world.addCircle(position.x, position.y, radius, 1.0f, 0.3f, b2BodyType::b2_dynamicBody);
    players.push_back(player);
    // Back pointer
    b2Body_SetUserData(player->body, player);
    b2Shape_SetUserData(player->shape, player);
    // b2shape_
}
void LogicalScene::addScenery(const b2Vec2& position, float radius, float mass) {
    Scenery* scenery = new Scenery();

    scenery->create();
    tie(scenery->body, scenery->shape) = world.addPolygon(position.x, position.y, {{0,0},{0,1},{1,1},{1,0}}, 1.0f, 0.3f, b2BodyType::b2_staticBody);
    sceneries.push_back(scenery);
    // Back pointer
    b2Body_SetUserData(scenery->body, scenery);
    b2Shape_SetUserData(scenery->shape, scenery);
}
void LogicalScene::addProjectile(const b2Vec2& position, float radius, float mass) {
    Projectile* bullet = new Projectile();

    bullet->create();
    tie(bullet->body, bullet->shape) = world.addCapsule(position.x, position.y, {0,0},{0,1}, 0.5);
    projectiles.push_back(bullet);
    // Back pointer
    b2Body_SetUserData(bullet->body, bullet);
    b2Shape_SetUserData(bullet->shape, bullet);
}

void LogicalScene::clearWorld(void) {
    //TODO destroy ents?
    world.destroy(); 
    world.create(gravity);
}

bool CustomFilterFcn( b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context ) {return true;}
bool CustomPreSolveFcn( b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* context ){return true;}
void LogicalScene::create(void) {
    world.create(gravity);

    view.init();
    // TODO: do i need it?
    // Logical scene is passed, but could also be global var.
    // world.setFilterCallback(&CustomFilterFcn, this);
    // world.setPresolveCallback(&CustomPreSolveFcn, this);
}

void LogicalScene::destroy(void) {
    //TODO check b2
    world.destroy();
}

void LogicalScene::tick(float dTime) {
    world.step(dTime);

    b2BodyEvents bodyEvents = world.GetBodyEvents();
    b2ContactEvents contacts = world.GetContactEvents();

    for(int i=0; i<contacts.beginCount; i++){
        auto touch = contacts.beginEvents[i];
        ActorType typeA = *((ActorType*)b2Shape_GetUserData(touch.shapeIdA));
        ActorType typeB = *((ActorType*)b2Shape_GetUserData(touch.shapeIdB));
        std::cout <<"BEGIN: "<< typeA <<" : " << typeB <<'\n';
        switch (typeA | typeB) {
            case (AT_Player|AT_Player):{break;}
            case (AT_Scenery|AT_Scenery):{break;}
            case (AT_Projectile|AT_Projectile):{
                //destroy both | weak one
                break;
            }

            case (AT_Player|AT_Scenery):{
                //extra knockbak to player? Stun
                // b2Shape_GetBody(touch.shapeIdA);
                // b2Body_GetContactData()
                break;
            }
            case (AT_Projectile|AT_Player):{
                //dmg+destroy / shield bounce
                break;
            }
            case (AT_Projectile|AT_Scenery):{
                //destroy / bounce
                break;
            }
            default:
                assert(false && "WRONG AT|AT");
        }        
    }

    view.start_frame();
// println
        view.start_main_pass();

    for(auto p : players){
        Shape shape = {};
            shape.coloring_info = {};
            shape.shapeType = Circle;
            shape.pos = vec2(0.5);
            shape.CIRCLE_radius = (0.5);
        view.draw_dynamic_shape(shape, SolidColor);
    }
        for(auto p : players){
        Shape shape = {};
            shape.coloring_info = {};
            shape.shapeType = Circle;
            shape.pos = vec2(0.5);
            shape.CIRCLE_radius = (0.5);
        view.draw_dynamic_shape(shape, SolidColor);
    }
// println
// println
            view.end_main_pass();
            // Draw everything
        // view.bloom_pass();
// println
    view.end_frame();
// println
}