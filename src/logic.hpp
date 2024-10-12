#pragma once
#ifndef __LOGIC_HPP__
#define __LOGIC_HPP__

// #include <vector>

#include "physics.hpp"
#include "visual.hpp"
#include "input.hpp"
#include "data_structures/dlist.hpp"
// using namespace glm;

/*
this is where all the parts are glued together
*/

#include "actors/player.hpp"
#include "actors/scenery.hpp"
#include "actors/projectile.hpp"

enum class SceneState {
    Round,
    Card,
    Menu,
};

//there is no actual reason to draw them separately
//it potentially saves like 2 bytes, which is nothing
//and for less than 50k state changes would probably be more harmfull
//still have to be copied twice tho
struct Particle{
    Shape shape;
    vec2 vel;
    float lifetime;
    // float init_radius;
    void draw(VisualView* view);
    void update(double dTime);
};

#include <cstdint>
#include <functional>


struct b2BodyIdHash{
    std::size_t operator()(const b2BodyId& bodyId) const noexcept{
        std::size_t hash = 17;
        hash = hash * 31 + std::hash<int32_t>()(bodyId.index1);
        hash = hash * 31 + std::hash<uint16_t>()(bodyId.world0);
        hash = hash * 31 + std::hash<uint16_t>()(bodyId.revision);
        return hash;
    }
};

struct b2BodyIdEqual{
    bool operator()(const b2BodyId& lhs, const b2BodyId& rhs) const noexcept{
        return lhs.index1 == rhs.index1 && 
               lhs.world0 == rhs.world0 && 
               lhs.revision == rhs.revision;
    }
};

//for processor .manifold.normal and .normal
template <typename Type>
concept HasNormal = requires(Type event) {
    { event.normal };
};
template <typename Type>
concept HasManifoldNormal = requires(Type event) {
    { event.manifold.normal };
};

// somewhat like a full-game-object 
// TODO: separate menu from game (or should i?)
class LogicalScene {
public:
    PhysicalWorld world = {};
    VisualView view = {};
    InputHandler input = {};
    // for projectiles, it is perfomance requirement. Others are in list for structural coherence
    List<Player> players = {16};
    List<Scenery> sceneries = {128};
    List<Scenery> borders = {8};
    List<Projectile> projectiles = {1<<16};

    float avg_radius = 0;
    vector<Particle> particles;

    SceneState current_state;

    // why map not vector? Body can have multiple reasons to be deleted in the same tick
    // i believe engine could be structured to not have such situatuations, but map is not that slow 
    // and there is no sequantil load peeks, only random ones
    // until 1m bullets its probably enough
    ankerl::unordered_dense::set<b2BodyId, b2BodyIdHash, b2BodyIdEqual> body_garbage;
    // controlled_player
    Player* slave = nullptr;
    Player* bot = nullptr;

    vec2 scene_hsize = {1920/float(1080)*10.0, 1*10.0};
    // camera effect props
    float earth_shake = 0.0; // HIITING THE GROUND
    DoublePendulum light_pendulum;
    float blast_shake = 0.0; //like speed
    float chromatic_abb = 0.0; //like position

    void setup(int player_count = 1);
    void cleanup(void);
    void setupActionCallbacks();

    void tick(double dTime);

    void loadScene(); //TODO: from file?

    // players travel between worlds
    // everything else doesnt

    // is called once game match begins
    void addNewPlayer(/*some visual props*/);
    void resetPlayersState();
    // is called in the end of the match
    void removeAllPlayers();
    
    void       addScenery(Scenery scenery);
    void    removeScenery(ListElem<Scenery>* scenery);
    void removeAllScenery();
        
    void       addProjectile(Projectile projectile, float vel_var);
    void       addProjectilesPack(Projectile projectile, int count, float vel_var);
    void    removeProjectile(ListElem<Projectile>* projectile);
    void removeAllProjectiles();

    void startNewRound();
    void giveCardsToDeadPlayers();
    void endRound();
    void genRndScenery();
    void genBorderScenery();

    void clearWorld(void);

    void addParticle(u8vec3 color, vec2 pos, vec2 vel, float size, float lifetime);
    void updateDrawParticles();
    void addEffect(
        u8vec3 baseColor, u8 colorVariation, 
        vec2 basePos, float posVariation, 
        vec2 baseVel, float velVariation, 
        float baseSize, float sizeVariation, 
        float baseLifetime, float lifetimeVariation, 
        int numParticles);

    //effect presets. Just shortucts, no logic
    void BulletSceneryHitEffect(Projectile* bullet, Scenery* scenery);
    void BulletPlayerHitEffect(Projectile* bullet, Player* player);
    void BulletBullethitEffect(Projectile* bullet1, Projectile* bullet2);
    void PlayerJumpEffect(Player* player);
    void PlayerDieEffect(Player* player);
    void PlayerSceneryHitEffect(Player* player, Scenery* scnery);

    using collisionProcessFun = std::function<void(ActorType, void*, ActorType, void*, vec2)>;
    // template <typename b2ContactEvent> 
    void processCollisionEvent(auto& touch, collisionProcessFun caseProcessor) {
        // std::cout << "A: " << touch.shapeIdA.index1 << " B: " << touch.shapeIdB.index1 << '\n';
        void* udataA = b2Shape_GetUserData(touch.shapeIdA);
        void* udataB = b2Shape_GetUserData(touch.shapeIdB);
        // assert(udataA && udataB);
        if(! (udataA && udataB)) return;
        ActorType typeA = ((Actor*)(udataA))->actorType;
        ActorType typeB = ((Actor*)(udataB))->actorType;
        vec2 normal;
        if constexpr(HasNormal<decltype(touch)>) normal = b2glm(touch.normal);
        if constexpr(HasManifoldNormal<decltype(touch)>) normal = b2glm(touch.manifold.normal);
        caseProcessor(typeA, udataA, typeB, udataB, normal); 
    }

    void processBeginEvents(b2ContactEvents contacts);
    void processEndEvents(b2ContactEvents contacts);
    void processHitEvents(b2ContactEvents contacts);
    void processMoveEvents(b2BodyEvents moves, double dTime);

    int countActivePlayers();
    // double getTime() {return glfwGetTime();}
};

#endif // __LOGIC_HPP__