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

// also used for box2d bits
enum class ActorType : int {
    Projectile = 1 << 0,
    Player     = 1 << 1,
    PlayerLeg  = 1 << 2,
    Scenery    = 1 << 3,
    Border     = 1 << 4,
    // Steppable

    ProjectilePlayer = Projectile|Player,
    PlayerScenery = Player|Scenery,
    SceneryProjectile = Scenery|Projectile,
    SceneryBorder = Scenery|Border, // literally scene bounding box (border)
    PlayerPlayerLeg = Player|PlayerLeg, // literally scene bounding box (border)
    SceneryPlayerLeg = Scenery|PlayerLeg, // literally scene bounding box (border)
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
        case b2_polygonShape: return Rectangle;
        default: assert(false);
    }
}
// loses info
inline b2ShapeType convertShapeType(ShapeType mineType){
    switch (mineType) {
        case Circle: return b2_circleShape;
        case Capsule: return b2_capsuleShape;
        case Rectangle: return b2_polygonShape;
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
        properties{ .color=coloring_info, .shape_type=shapeType, .body_type=bodyType},
        shapeProps(shapeProps) {}
    
    Shape constructActorShape();
};

//changed by game actions. Reset on new round
struct PlayerState{
    int jumps_left = 0;
    // how much time after last jump refresh passed, in seconds
    // double last_jmp_refill = 0.0;
    int touching_grass_counter = 0;
    // there is 1 frame delay between jumping and touching grass set to false, so we need to delay refill too
    bool refill_body_jumps_next_frame = false;
    // bool refill_leg_jumps_next_frame = false;
    
    double hp_left = 100;
    vec2 aim_direction = vec2(0);

    double damage = 0;
    double time_since_last_reload = 0; //between closest shots from separate magazines
    double time_since_last_shot = 0; //between shots from same magazine
    double time_since_last_block = 0;
    int lives_left = 1; // 0 if dead X_x
    int ammunition_left = 0; //in magazine
};
//changed by cards. Reset on new game
struct PlayerProps {
    // Core Attributes
    double health_points = 100;            // Total health points
    int max_lives = 1;                     // Maximum number of lives
    int max_jumps = 1;                     // Maximum number of jumps per round
    float jump_impulse = 20;                 // Impulse applied when jumping
    float player_mass = 1.0f;              // Player's physical mass
    float player_radius = 0.5;             // Size of the player (hitbox radius)
    
    // Weapon/Bullet Attributes
    float bullet_size = 0.1;               // Radius of each bullet
    double bullet_damage = 15;             // Damage per bullet hit
    double reload_duration = 2;            // Total time to reload the weapon
    double shot_delay = 1;                 // Delay between consecutive shots from the same magazine
    double bullet_velocity = 17.0;         // Speed at which bullets travel
    int bullets_per_magazine = 3;          // Number of bullets per magazine
    int bullet_bounce_count = 0;           // Number of times a bullet can bounce off surfaces
    int extra_bullets_per_shot = 0;        // Additional bullets fired per shot (spread)

    float life_steal_percentage = 0.0f;    // Percentage of damage converted to health restoration

    // Block and Cooldown Attributes
    float block_cooldown = 10;             // Cooldown time between blocks
    float hit_knockback = 1.3;          // Knockback effect applied after shooting

    // Multipliers and Effect Modifiers
    float on_damage_hp_boost = 1.0f;       // HP boost when dealing damage (e.g., Brawler)
    float damage_received_multiplier = 1.0f;  // Multiplier for damage taken
    float bounce_damage_multiplier = 1.0f;  // Additional damage from bullet bounces
    float explosion_radius = 0;            // Explosive power for bullets (radius of explosion)
    float homing_strength = 0;             // Homing force for bullets to track targets
    float poison_damage_percentage = 0;    // Percentage of damage applied as poison (over time)
    float stun_duration = 0;               // Duration of stun effect from bullets
    float damage_over_time = 0.0f;         // Damage applied over time (e.g., poison, parasite)

    // On Block Effects
    int bombs_spawned_on_block = 0;        // Number of bombs spawned when blocking
    float teleport_distance_on_block = 0;  // Distance teleported when blocking
    float saw_damage_on_block = 0;         // Damage from spawning a saw when blocking
    float dark_power_charge_time = -1;     // Time required to summon dark powers (Abyssal Countdown)
    
    // Movement and Utility
    float chase_speed_multiplier = 1.0f;   // Speed multiplier when moving towards the opponent
    
    // Visual Effects
    glm::u8vec3 bullet_color = twpp::lime(200); // Color of the bullets (RGB format)
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