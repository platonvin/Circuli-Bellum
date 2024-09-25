#pragma once

#include <vector>

#include "physics.hpp"
#include "visual.hpp"

class LogicalScene {
public:
    PhysicalWorld world;
    VisualView view;
    std::vector<Player*> players;
    std::vector<Scenery*> sceneries;
    std::vector<Projectile*> projectiles;

    // std::vector<hit> hits
    // 
    void create(void);
    void destroy(void);

    void tick(float dTime);

    b2Vec2 gravity = {0, -9.81};

    void addPlayer(const b2Vec2& position, float radius, float mass);
    // void addSceneryBlock(const b2Vec2& position, float radius, float mass);
    void addScenery(const b2Vec2& position, float radius, float mass);
    // void addSceneryTriangle(const b2Vec2& position, float radius, float mass);
    void addProjectile(const b2Vec2& position, float radius, float mass);
    // void addProjectileCapsule(const b2Vec2& position, float radius, float mass);
    void clearWorld(void);
};