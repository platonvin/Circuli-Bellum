#pragma once
// #include "actors/scenery.hpp"
#ifndef __PROJECTILE_HPP__
#define __PROJECTILE_HPP__

#include "actor.hpp"
// #include <functional>
class Player;
class Scenery;

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
    float thruster_force = 0.0f; // Force of thrust rocket. does NOT scale with mass
};


struct Projectile {
public:
    //first for custom rtti :)
    Actor actor;

    Projectile() : 
        actor(ActorType::Projectile, b2_dynamicBody, Circle, \
            twpp::pink(700), vec2(0), {.CIRCLE_radius = 1}) {}
            
    Projectile(double damage, float radius) : 
        actor(ActorType::Projectile, b2_dynamicBody, Circle, \
            twpp::pink(700), vec2(0), {.CIRCLE_radius = radius}),
        state{.damage=damage, .radius=radius}
        // ,props{.damage=float(damage), .radius=radius} 
        {}

    void setupFromPlayer(Player* player);

    void update(PhysicalWorld* world /*for later?*/, float dTime);
    void updateTrailData();
    void drawTrail(VisualView* view);
    void addToWorld(PhysicalWorld* world, int group = 0);
    //return false if destroy
    bool processSceneryHit(Scenery* scenery);
    bool processPlayerHit(Player* player);

    struct ProjectileState state;
    // struct ProjectileProps props;
    Player* master;
    
    Shape constructShape();
    void draw(VisualView* view);


    static const int NUM_SEGMENTS = 10;
    static std::array<float, NUM_SEGMENTS> precomputedSizes;
    // static initializer
    static void initializePrecomputedSizes() {
        for (int i = 0; i < NUM_SEGMENTS; i++) {
            precomputedSizes[i] = sqrt(1.0f - (float(i) / NUM_SEGMENTS));
        }
    }

    //for a bullet trail
    struct TrailSegment {
        vec2 pos;
        vec2 dir;
        float angle;
        float len;
    };

    // 0->1
    // ...
    // 8->9
    // 9->None, this was the last one
    std::array<TrailSegment, NUM_SEGMENTS> trailSegments = {};
    //trail + others collision
    float time_elapsed = 0;
};
#endif // __PROJECTILE_HPP__
