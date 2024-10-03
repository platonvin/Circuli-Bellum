#pragma once
#include "box2d/types.h"
#include "data_structures/fixed_map.hpp"
#include "physics.hpp"
#include "visual.hpp"
#ifndef __ACTOR_HPP__
#define __ACTOR_HPP__

//absolutely legit and 100% safe code
//this is to store information in box2d pointer
//TODO test just llint
inline int ptr2id(void* ptr){
    return (int)(long long int)(ptr);
}
inline void* id2ptr(int id){
    return (void*)(long long int)(id);
}

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

// not full
inline ShapeType convertShapeType(b2ShapeType b2type){
    switch (b2type) {
        case b2_circleShape: return Circle;
        case b2_capsuleShape: return Capsule;
        case b2_polygonShape: return Square;
        default: assert(false);
    }
}
// loses info
inline b2ShapeType convertShapeType(ShapeType mineType){
    switch (mineType) {
        case Circle: return b2_circleShape;
        case Capsule: return b2_capsuleShape;
        case Square: return b2_polygonShape;
        case Trapezoid: return b2_polygonShape;
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

    //has no update/destroy, instead is changed from move events and by higher-level enteties
    
    Actor(ActorType actorType, b2BodyType bodyType, ShapeType shapeType, u8vec3 coloring_info, vec2 pos, ShapeProps shapeProps) : 
        actorType{actorType}, 
        state{.pos=pos},
        properties{ .color=coloring_info, .body_type=bodyType, .shape_type=shapeType},
        shapeProps(shapeProps) {}
    
    Shape constructActorShape();
};

struct PlayerState{
    int jumps_left = 0;
    // how much time after last jump refresh passed, in seconds
    // double last_jmp_refill = 0.0;
    int touching_grass_counter = false;
    // there is 1 frame delay between jumping and touching grass set to false, so we need to delay refill too
    bool refill_jumps_next_frame = false;
    
    double hp_left = 100;
    vec2 aim_direction = vec2(0);

    double damage = 0;
    double time_since_last_reload = 0; //between closest shots from separate magazines
    double time_since_last_shot = 0; //between shots from same magazine
    double time_since_last_block = 0;
    int lives_left = 1;
    int ammunition_left = 0; //in magazine
};
struct PlayerProps{
    double hp = 100;
    int max_lives = 1;
    int max_jumps = 1;
    float jump_impulse = 20;
    float mass = 1.0f;
    float radius = 1.0;
    float bullet_radius = 0.1;
    double damage = 15;
    double reload_time = 2; //between closest shots from separate magazines
    double bullet_feed_time = 1; //between shots from same magazine
    double bullet_speed = 17.0;
    int ammunition_count = 3;
    int bullet_bounces = 0;
    u8vec3 bullet_color = twpp::lime(200);
    double block_delay = 10;
    double knockback = 1.3; //not a special number
};

struct ProjectileState {
    double damage;
    float radius;
    int bounces_left=1;
};
struct ProjectileProps{
    double damage;
    float radius = 1.0;
    int max_bounces = 1;
    float bounciness = 0.9;
};

struct SceneryState{
    double durability;
};
struct SceneryProps{
    double durability; //basically hp
    double contactDamage; //on touch
    double gravityMultiplier; //when inside
};
#endif // __ACTOR_HPP__