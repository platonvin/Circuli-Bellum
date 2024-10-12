#pragma once
// #include "actors/scenery.hpp"
#ifndef __PROJECTILE_HPP__
#define __PROJECTILE_HPP__

#include "actor.hpp"
// #include <functional>

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
        state{.damage=damage, .radius=radius},
        props{.damage=float(damage), .radius=radius} {}

    void setupFromPlayer(PlayerState* ownerState, PlayerProps* ownerProps, Actor* ownerActor);

    void update(PhysicalWorld* world /*for later?*/, float dTime);
    void updateTrailData();
    void drawTrail(VisualView* view);
    void addToWorld(PhysicalWorld* world, int group = 0);
    //return false if destroy
    bool processSceneryHit(SceneryState* scenery);
    bool processPlayerHit(PlayerState* player);

    struct ProjectileState state;
    struct ProjectileProps props;
    
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
