#pragma once
#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "actor.hpp"
#include "card.hpp"
// #include <iostream>

class Projectile;
class Scenery;
class ParticleSystem;
class Field;

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
    double block_time_left = 0; // if 1 then block will keep going for 1 second

    int lives_left = 1; // 0 if dead X_x
    int ammunition_left = 0; //in magazine
};
//changed by cards. Reset on new game
struct PlayerProps{
    double max_hp = 100;
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
    u8vec4 bullet_color = twpp::lime(200);
    int bullets_per_shot = 1;        // Additional bullets fired per shot (spread)
    float spread = 0.05;

    float life_steal_percentage = 0.0f;    // Percentage of damage converted to health restoration
    float thruster_force = 0.0f; // Force of thrust rocket. does NOT scale with mass

    float block_cooldown = 4;             // Cooldown time between blocks
    float block_duration = 0.5;
    float hit_knockback = 1.3;          // Knockback effect applied after shooting

    // Effect Modifiers (bullet things)
    float on_damage_hp_boost = 1.0f;       // HP boost when dealing damage (e.g., Brawler)
    float bounce_damage_multiplier = 1.0f;  // Additional damage from bullet bounces
    float explosion_radius = 0;            // Explosive power for bullets (radius of explosion)
    float homing_strength = 0;             // Homing force for bullets to track targets
    float poison_damage_percentage = 0;    // Percentage of damage applied as poison (over time)
    float stun_duration = 0;               // Duration of stun effect from bullets
    float damage_over_time = 0.0f;         // Damage applied over time (e.g., poison, parasite)


    // On Block Effects
    int bombs_spawned_on_block = 0;
    
    float saw_damage = 0;
    float saw_radius_rel = 2.0; // relative to player

    float healing_field_heal = 0;
    float healing_field_radius_rel = 1;
    
    float dark_power_charge_time = -1;     // Time required to summon dark powers (Abyssal Countdown)
    
        
    // Weird effects    
    float chase_speed_multiplier = 1.0f;   // Speed multiplier when moving towards the opponent
};


struct Player {
public:
    Actor actor;
    // {
    Actor leg = Actor(ActorType::PlayerLeg, b2_staticBody, Circle, {}, {}, {});
    // ActorType legAT = ActorType::PlayerLeg;
    // PhysicalBindings leg;
        b2JointId legJoint;
    // }
    Player() : actor(ActorType::Player, b2_dynamicBody, Circle, 
            twpp::pink(500), vec2(0), {.CIRCLE_radius = float(1.0)}) {}

    Player(float mass, float radius)
        : actor(ActorType::Player, b2_dynamicBody, Circle, twpp::pink(600),
                vec2(0), {.CIRCLE_radius = float(radius)}),
          props{.mass = mass, .radius = radius} {}

    //updates on local data, no contact with outside classes
    //TODO separation with controlled player
    void update(PhysicalWorld* world, float dTime /*for later?*/, vec2 view_pos);
    void updateGun(float dTime);
    void updateLegPositions(PhysicalWorld* world);
    void updateThrusters(PhysicalWorld* world, float dTime);

    void resetState();
    void addToWorld(PhysicalWorld* world);
    Shape constructShape();
    void draw(VisualView* view, ParticleSystem* psys, float dTime);
    void drawBody(VisualView* view);
    void drawBlock(VisualView* view);
    void drawLegs(VisualView* view);
    void drawEyes(VisualView* view);
    // void drawEyeBrows(VisualView* view);
    void drawArmsAndGun(VisualView* view);
        void drawBlockArm(VisualView* view, bool is_mirrored);
        void drawHand(VisualView* view, float handRadius, vec2 handPos);
        void drawMagazine(VisualView* view, vec2 magazinePos, float magazineWidth, float magazineHeight, float rotAngle);
        void drawBarrel(VisualView* view, vec2 barrelPos, float barrelLength, float barrelWidth, float rotAngle);
        void drawAmmo(VisualView* view, vec2 ammoStartPos, float ammoSize, int bulletsPerRow, int totalAmmo, vec2 aimDir, vec2 perpDir, float rot, bool isMirrored);
        void drawShotDelay(VisualView* view, vec2 pos, float fill, float size);
        void drawReloadDelay(VisualView* view, vec2 pos, float fill, float size);
    void drawThrusters(VisualView* view, ParticleSystem* psys, float dTime);
    void drawThrusterCapsule(VisualView* view, vec2 pos, vec2 dir, float length, float radius);
    void drawThrusterBand(VisualView* view, vec2 pos, vec2 dir, float length, float radius);


    void drawCard(const Card* card);
    vector<const Card*> cards = {};

    // ProjectileProps createBullet();

    void processBulletHit(PhysicalWorld* world, Projectile* projectile);
    void processDamage(float damage);
    void processHeal(float heal);
    void processBorderTouch(PhysicalWorld* world, vec2 normal);
    void processField(Field* field, float multiplier);

    bool tryJump(PhysicalWorld* world);
    bool tryBlock();

    struct PlayerState state = {};
    struct PlayerProps props = {};

    //for inverse kinematics
    vec2 real_left_leg_pos = vec2(0);
    vec2 real_right_leg_pos = vec2(0);
    vec2 smooth_left_leg_pos = vec2(0);
    vec2 smooth_right_leg_pos = vec2(0);
    //smooth movement. TODO: move to b2 & make bullet collider via bitset
    vec2 visual_pos = vec2(0);
    vec2 visual_vel = vec2(0);
    float springConstant = 600.0f;
    float dampingFactor = 30.0f; 
    void updateVisualPos(float dTime);
// private:
    void softLimitVelocity(PhysicalWorld* world);
    void refillJumpsIfNeeded(float dTime);

    void logState() const;
    struct Thruster{
        vec2 dir;
        float time_left;
        float hlen;
        float force;
        int spawned_particles=0;
    };
    vector<Thruster> thrusters;

    struct HpBarState {
        float current_ratio = 1.0;
    } hp_bar;
    void drawHpBar(VisualView* view);
    void updateHpBar(float dTime);
    void updateBlock(float dTime);
    bool blocking();
};
#endif // __PLAYER_HPP__