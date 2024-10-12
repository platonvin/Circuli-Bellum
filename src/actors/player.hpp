#pragma once
#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include "actor.hpp"
#include "card.hpp"
// #include <iostream>


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
    void updateThrusters(float dTime);

    void resetState();
    void addToWorld(PhysicalWorld* world);
    Shape constructShape();
    void draw(VisualView* view);
    void drawBody(VisualView* view);
    void drawLegs(VisualView* view);
    void drawEyes(VisualView* view);
    // void drawEyeBrows(VisualView* view);
    void drawArmsAndGun(VisualView* view);
        void drawHand(VisualView* view, float handRadius, vec2 handPos);
        void drawMagazine(VisualView* view, vec2 magazinePos, float magazineWidth, float magazineHeight, float rotAngle);
        void drawBarrel(VisualView* view, vec2 barrelPos, float barrelLength, float barrelWidth, float rotAngle);
        void drawAmmo(VisualView* view, vec2 ammoStartPos, float ammoSize, int bulletsPerRow, int totalAmmo, vec2 aimDir, vec2 perpDir, float rot, bool isMirrored);
        void drawShotDelay(VisualView* view, vec2 pos, float fill, float size);
        void drawReloadDelay(VisualView* view, vec2 pos, float fill, float size);
    void drawThrusters(VisualView* view);
    void drawThrusterCapsule(VisualView* view, vec2 pos, vec2 dir, float length, float radius);
    void drawThrusterBand(VisualView* view, vec2 pos, vec2 dir, float length, float radius);


    void drawCard(const Card* card);
    vector<const Card*> cards = {};

    ProjectileProps createBullet();

    void processBulletHit(PhysicalWorld* world, ProjectileState* projectile);
    void processDamage(float damage);
    void processBorderTouch(PhysicalWorld* world, vec2 normal);
    void processJump(PhysicalWorld* world);

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
    };
    vector<Thruster> thrusters;

    struct HpBarState {
        float current_ratio = 1.0;
    } hp_bar;
    void drawHpBar(VisualView* view);
    void updateHpBar(float dTime);
};
#endif // __PLAYER_HPP__