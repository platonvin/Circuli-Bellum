#pragma once
// #include "actors/scenery.hpp"
#ifndef __PROJECTILE_HPP__
#define __PROJECTILE_HPP__

#include "actor.hpp"
// #include <functional>

class Scenery;
class Player;

class Projectile {
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
        props{.damage=damage, .radius=radius} {}

    void setup(PlayerState* ownerState, PlayerProps* ownerProps, Actor* ownerActor);

    void update(PhysicalWorld* world /*for later?*/, float dTime);
    void updateTrailData();
    void drawTrail(VisualView* view);
    void addToWorld(PhysicalWorld* world);
    //return false if destroy
    bool processSceneryHit(SceneryState* scenery);
    bool processPlayerHit(PlayerState* player);

    struct ProjectileState state;
    struct ProjectileProps props;
    
    Shape constructShape();
    void draw(VisualView* view);

    //for a bullet trail
    std::array<vec2, 10> oldPositions = {};
    float time_elapsed = 0;
    
};
#endif // __PROJECTILE_HPP__
