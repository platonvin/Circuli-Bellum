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
//and for less than 50k states change would probably be more harmfull
//still have to be copied twice tho
struct Particle{
    Shape shape;
    // vec2 vel;
    float lifetime;
    void draw(VisualView* view);
    void update(float dTime);
};

// somewhat like a full-game-object 
// TODO: separate menu from game (or should i?)
class LogicalScene {
public:
    PhysicalWorld world = {};
    VisualView view = {};
    InputHandler input = {};
    // for projectiles, it is perfomance requirement. Others are in list for structural coherence
    List<Player> players = {};
    List<Scenery> sceneries = {};
    List<Projectile> projectiles = {};
    vector<b2BodyId> body_garbage = {};
    Player* controlled_player = 0;

    void setup(int player_count = 1);
    void cleanup(void);
    void setupActionCallbacks();

    void tick(float dTime);

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
        
    void       addProjectile(Projectile projectile);
    void    removeProjectile(ListElem<Projectile>* projectile);
    void removeAllProjectiles();

    void startNewRound();
    void endRound();
    void genRndScenery();

    void clearWorld(void);

    SceneState current_state;

    vector<Particle> particles;
    void addParticle(u8vec3 color, vec2 pos, vec2 vel, float size, float lifetime);
    void updateDrawParticles();
    void addEffect(u8vec3 color, vec2 pos, int numParticles, float maxOffset, float maxVel, float size, float lifetime);

private:
    using collisionProcessFun = std::function<void(ActorType, void*, ActorType, void*)>;
    // template <typename b2ContactEvent> 
    void processCollisionEvent(auto& touch, collisionProcessFun caseProcessor) {
        // std::cout << "A: " << touch.shapeIdA.index1 << " B: " << touch.shapeIdB.index1 << '\n';
        void* udataA = b2Shape_GetUserData(touch.shapeIdA);
        void* udataB = b2Shape_GetUserData(touch.shapeIdB);
        assert(udataA && udataB);
        ActorType typeA = static_cast<Actor*>(udataA)->actorType;
        ActorType typeB = static_cast<Actor*>(udataB)->actorType;
        caseProcessor(typeA, udataA, typeB, udataB); 
    }

    void processBeginEvents(b2ContactEvents contacts);
    void processEndEvents(b2ContactEvents contacts);
    void processHitEvents(b2ContactEvents contacts);
    void processMoveEvents(b2BodyEvents moves, float dTime);

    // double getTime() {return glfwGetTime();}
};

#endif // __LOGIC_HPP__