#pragma once
#ifndef __LOGIC_HPP__
#define __LOGIC_HPP__

#include <vector>

#include "physics.hpp"
#include "visual.hpp"
#include "input.hpp"
#include "dlist.hpp"
// using namespace glm;

/*
this is where all the parts are glued together
*/

enum class ActorType : int {
    Projectile = 1 << 0,
    Player     = 1 << 1,
    Scenery    = 1 << 2,

    ProjectilePlayer = Projectile|Player,
    PlayerScenery = Player|Scenery,
    SceneryProjectile = Scenery|Projectile,
};
inline constexpr ActorType operator&(ActorType x, ActorType y){
    return static_cast<ActorType> (to_underlying(x) & to_underlying(y));
}   
inline constexpr ActorType operator|(ActorType x, ActorType y){
    return static_cast<ActorType> (to_underlying(x) | to_underlying(y));
}
inline std::ostream& operator<<(std::ostream& o, ActorType a){
    std::cout << to_underlying(a);
    return o;
}

inline ShapeType convertShapeType(b2ShapeType b2type){
    switch (b2type) {
        case b2_circleShape: return Circle;
        case b2_capsuleShape: return Capsule;
        case b2_polygonShape: return Square;
        default: assert(false);
    }
}
inline b2ShapeType convertShapeType(ShapeType mineType){
    switch (mineType) {
        case Circle: return b2_circleShape;
        case Capsule: return b2_capsuleShape;
        case Square: return b2_polygonShape;
        default: assert(false);
    }
}

class Actor {
public:
    //both type in actor and actor in anything else
    //should be first for custom rtti :)
    ActorType actorType;
    PhysicalState state;
    PhysicalProperties properties;
    PhysicalBindings bindings;

    ShapeProps shapeProps;

    Actor(ActorType actorType, b2BodyType bodyType, b2ShapeType shapeType, u8vec3 coloring_info, vec2 pos, ShapeProps shapeProps) : 
        actorType{actorType}, 
        state{.pos=pos},
        properties{ .color=coloring_info, .body_type=bodyType, .shape_type=shapeType},
        shapeProps(shapeProps) {}

    Shape constructActorShape();
};

class Player {
public:
    Actor actor;
    Player(float mass, float radius) : 
        actor(ActorType::Player, b2_dynamicBody, b2_circleShape, 
            twpp::pink(700), vec2(0), {.CIRCLE_radius = float(radius)}),
        props{.mass=mass, .radius=radius} {}
    
    struct player_state{
        double hp_left=100;
        // int jumps_left=0;
        // how much time after last jump refresh passed, in seconds
        // double last_jmp_refill = 0.0;
        bool touching_grass=false;
        bool has_jump = false;
    } state;
    struct player_props{
        double hp = 100;
        float mass = 1.0f;
        float radius = 1.0;
        int max_jumps = 1;
    } props;

    Shape constructShape();
};

class Scenery {
public:
    //first for custom rtti :)
    Actor actor;
    Scenery(ShapeType type, ShapeProps shapeProps, vec2 pos, bool dynamic) : 
        actor(ActorType::Scenery, (dynamic? b2_dynamicBody : b2_staticBody), convertShapeType(type), \
            twpp::pink(700), vec2(0), shapeProps),
        props{} {} //TODO
    
    struct scenery_state{
        double durability;
    } state;
    struct scenery_props{
        double durability; //basically hp
        double contactDamage; //on touch
        double gravityMultiplier; //when inside
    } props;
    
    Shape constructShape();
};

class Projectile {
public:
    //first for custom rtti :)
    Actor actor;
    Projectile(double damage, float radius) : 
        actor(ActorType::Projectile, b2_dynamicBody, b2_circleShape, \
            twpp::pink(700), vec2(0), {.CIRCLE_radius = radius}),
        props{.damage=damage, .radius=radius} {}

    struct projectile_state{
        double damage;
    } state;
    struct projectile_props{
        double damage;
        float radius;
    } props;
    
    Shape constructShape();
};

enum class SceneState {
    Round,
    Card,
    Menu,
};

// somewhat like a full-game-object 
// TODO: separate menu from game (or should i?)
class LogicalScene {
public:
    PhysicalWorld world = {};
    VisualView view = {};
    InputHandler input = {};
    // for projectiles, it is perfomance requirement. Others are in list for structural coherence
    List<Player> players = {};
    List<Scenery> sceneries = {};
    List<Projectile> projectiles = {};

    // std::vector<hit> hits
    void create(int player_count = 1);
    void destroy(void);
    void setupActionCallbacks();

    void tick(float dTime);

    void loadScene(); //TODO: from file?

    // players travel between worlds
    // everything else doesnt

    // is called once game match begins
    void addNewPlayer(/*some visual props*/);
    // is called in the end of the match
    void removeAllPlayers();
    
    void       addScenery(Scenery scenery);
    void    removeScenery(ListElem<Scenery>* scenery);
    void removeAllScenery();
    void       addProjectile(Projectile projectile);
    void    removeProjectile(ListElem<Projectile>* projectile);
    void removeAllProjectiles();

    void addPlayerToWorld(Player* player);
    void addSceneryToWorld(Scenery* scenery, const std::vector<b2Vec2>& vertices);
    void addProjectileToWorld(Projectile* projectile);

    void clearWorld(void);

    SceneState current_state;

private:

    using collisionProcessFun = std::function<void(ActorType, void*, ActorType, void*)>;
    // template <typename b2ContactEvent> 
    void processCollisionEvent(auto& touch, collisionProcessFun caseProcessor) {
        // std::cout << "A: " << touch.shapeIdA.index1 << " B: " << touch.shapeIdB.index1 << '\n';
        void* udataA = b2Shape_GetUserData(touch.shapeIdA);
        void* udataB = b2Shape_GetUserData(touch.shapeIdB);
        assert(udataA && udataB);
        ActorType typeA = static_cast<Actor*>(udataA)->actorType;
        ActorType typeB = static_cast<Actor*>(udataB)->actorType;
        caseProcessor(typeA, udataA, typeB, udataB); 
    }

    void processBeginEvents(b2ContactEvents contacts);
    void processEndEvents(b2ContactEvents contacts);
    void processHitEvents(b2ContactEvents contacts);
    void processMoveEvents(b2BodyEvents moves);

    // double getTime() {return glfwGetTime();}
};

#endif // __LOGIC_HPP__