#pragma once
#ifndef __ACTOR_HPP__
#define __ACTOR_HPP__
#include "box2d/types.h"
#include "data_structures/fixed_map.hpp"
#include "physics.hpp"
#include "visual.hpp"

// also used for box2d bits
enum class ActorType : int {
    Projectile     = 1 << 0,
    Player         = 1 << 1,
    PlayerLeg      = 1 << 2,
    StaticScenery  = 1 << 3,
    DynamicScenery = 1 << 4,
    Border         = 1 << 5, // does not refill jump btw

    ProjectilePlayer = Projectile|Player,
    PlayerScenery = Player|StaticScenery,
    SceneryProjectile = StaticScenery|Projectile,
    StaticSceneryBorder = StaticScenery|Border, // literally scene bounding box (border)
    DynamicSceneryBorder = DynamicScenery|Border, // literally scene bounding box (border)
    PlayerBorder = Player|Border,
    PlayerLegBorder = PlayerLeg|Border,
    PlayerPlayerLeg = Player|PlayerLeg, // literally scene bounding box (border)
    SceneryPlayerLeg = StaticScenery|PlayerLeg, // literally scene bounding box (border)
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
ShapeType convertShapeType(b2ShapeType b2type);
// loses info
b2ShapeType convertShapeType(ShapeType mineType);

//has no update/destroy, instead is changed from move events and by higher-level enteties
struct Actor {
public:
    //both type in actor and actor in anything else
    //should be first for custom rtti :)
    ActorType actorType = {};
    PhysicalState state = {};
    PhysicalProperties properties = {};
    PhysicalBindings bindings = {};

    ShapeProps shapeProps = {};

    // i didn't want to use constructors, but it is really not doing anythinh
    // The road to unreadable code is paved with readability intentions
    Actor(ActorType actorType, b2BodyType bodyType, ShapeType shapeType, u8vec3 coloring_info, vec2 pos, ShapeProps shapeProps) : 
        actorType{actorType}, 
        state{.pos=pos},
        properties{ .color=coloring_info, .shape_type=shapeType, .body_type=bodyType},
        shapeProps(shapeProps) {}

    Shape constructActorShape() const;
};

//changed by game actions. Reset on new round
struct PlayerState{
    int jumps_left = 0;
    // how much time after last jump refresh passed, in seconds
    float time_since_jump_refill = 0.0;
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
struct PlayerProps{
    double hp = 100;
    int max_lives = 1;
    int max_jumps = 1;
    float jump_impulse = 20;
    float mass = 1.0f;
    float radius = 0.5;
    float bullet_radius = 0.1;
    double bullet_damage = 15;
    double reload_duration = 2; //between closest shots from separate magazines
    double shot_delay = 1; //between shots from same magazine
    double bullet_speed = 17.0;
    int max_ammo = 3;
    int bullet_bounces = 0;
    float extra_damage_per_bounce = 0;
    float grow_factor = 0;
    u8vec3 bullet_color = twpp::lime(200);
    double block_delay = 10;
    int bullets_per_shot = 1;        // Additional bullets fired per shot (spread)
    float spread = 0.05;

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
};

struct ProjectileState {
    double damage = 0;
    double radius = 0;
    int bounces_left =1;
};
struct ProjectileProps{
    float damage = 0;
    float radius = 1.0;
    int max_bounces = 1;
    float extra_damage_per_bounce = 0; // Trickster thing. Increase damage on bounce
    float bounciness = 0.9; // physicsal property
    float grow_factor = 0; // multiplier per second. if 10, then damage *= 11 every second
};

struct SceneryState{
    double durability;
};
struct SceneryProps{
    double durability; //basically hp
    double contactDamage; //on touch
    double gravityMultiplier; //when inside. Water?
};
#endif // __ACTOR_HPP__