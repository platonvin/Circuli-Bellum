#pragma once
#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "actor.hpp"
#include "card.hpp"

class Player {
public:
    Actor actor;

    Player() : actor(ActorType::Player, b2_dynamicBody, Circle, 
            twpp::pink(700), vec2(0), {.CIRCLE_radius = float(1.0)}) {}

    Player(float mass, float radius) : 
        actor(ActorType::Player, b2_dynamicBody, Circle, 
            twpp::pink(700), vec2(0), {.CIRCLE_radius = float(radius)}),
        props{.mass=mass, .radius=radius} {}
    
    //updates on local data, no contact with outside classes
    //TODO separation with controlled player
    void update(PhysicalWorld* world /*for later?*/, vec2 view_pos);
    
    void addToWorld(PhysicalWorld* world);
    Shape constructShape();
    void draw(VisualView* view);

    void drawCard(Card card);

    //potentially makes it much harder to optimize, but still
    // const std::function<void()> tester = [this]() -> void {
    //     pl(this);
    // };
    // void processBulletHit(Projectile)
    void processBulletHit(PhysicalWorld* world, ProjectileState* projectile);

    struct PlayerState state = {};
    struct PlayerProps props = {};
    vector<Card> cards;
private:
    void softLimitVelocity(PhysicalWorld* world);
    void refillJumpsIfNeeded();
};
#endif // __PLAYER_HPP__