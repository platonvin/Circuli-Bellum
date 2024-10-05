#pragma once
#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "actor.hpp"
#include "card.hpp"

class Player {
public:
    Actor actor;

    Player() : actor(ActorType::Player, b2_dynamicBody, Circle, 
            twpp::pink(500), vec2(0), {.CIRCLE_radius = float(1.0)}) {}

    Player(float mass, float radius) : 
        actor(ActorType::Player, b2_dynamicBody, Circle, 
            twpp::pink(600), vec2(0), {.CIRCLE_radius = float(radius)}),
        props{.mass=mass, .radius=radius} {}
    
    //updates on local data, no contact with outside classes
    //TODO separation with controlled player
    void update(PhysicalWorld* world /*for later?*/, vec2 view_pos);
    
    void addToWorld(PhysicalWorld* world);
    Shape constructShape();
    void draw(VisualView* view);
    void drawBody(VisualView* view);
    void drawLegs(VisualView* view);
    void drawArmsAndGun(VisualView* view);
    void drawAmmo(VisualView* view);

    void drawCard(Card card);

    //potentially makes it much harder to optimize, but still
    void processBulletHit(PhysicalWorld* world, ProjectileState* projectile);

    struct PlayerState state = {};
    struct PlayerProps props = {};
    vector<Card> cards;

    //smooth movement. TODO: move to b2 & make bullet collider via bitset
    vec2 visual_pos = vec2(0);
    vec2 visual_vel = vec2(0);
    float springConstant = 600.0f;
    float dampingFactor = 20.0f; 
    void updateVisualPos(float dTime);
private:
    void softLimitVelocity(PhysicalWorld* world);
    void refillJumpsIfNeeded();
};
#endif // __PLAYER_HPP__