#pragma once
#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "actor.hpp"

class Player {
public:
    Actor actor;

    Player() : actor(ActorType::Player, b2_dynamicBody, b2_circleShape, 
            twpp::pink(700), vec2(0), {.CIRCLE_radius = float(1.0)}) {}

    Player(float mass, float radius) : 
        actor(ActorType::Player, b2_dynamicBody, b2_circleShape, 
            twpp::pink(700), vec2(0), {.CIRCLE_radius = float(radius)}),
        props{.mass=mass, .radius=radius} {}
    
    //updates on local data, no contact with outside classes
    //TODO separation with controlled player
    void update(PhysicalWorld* world /*for later?*/, vec2 view_pos);
    
    void addToWorld(PhysicalWorld* world);

    //potentially makes it much harder to optimize, but still
    // const std::function<void()> tester = [this]() -> void {
    //     pl(this);
    // };
    // void processBulletHit(Projectile)
    void processBulletHit(ProjectileState* projectile);

    struct PlayerState state;
    struct PlayerProps props;

    Shape constructShape();

private:
    void softLimitVelocity(PhysicalWorld* world);
    void refillJumpsIfNeeded();
};
#endif // __PLAYER_HPP__