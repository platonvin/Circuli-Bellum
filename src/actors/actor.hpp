#pragma once
#ifndef __ACTOR_HPP__
#define __ACTOR_HPP__
#include "box2d/types.h"
#include "data_structures/fixed_map.hpp"
#include "physics.hpp"
#include "visual.hpp"

// also used for box2d bits
enum class ActorType : int {
    Projectile     = 1 << 0,
    Player         = 1 << 1,
    PlayerLeg      = 1 << 2,
    StaticScenery  = 1 << 3,
    DynamicScenery = 1 << 4,
    Border         = 1 << 5, // does not refill jump btw
    StaticField    = 1 << 6, // stays in place
    playerField    = 1 << 7, // attached to player

    ProjectilePlayer = Projectile|Player,
    PlayerScenery = Player|StaticScenery,
    SceneryProjectile = StaticScenery|Projectile,
    StaticSceneryBorder = StaticScenery|Border, // literally scene bounding box (border)
    DynamicSceneryBorder = DynamicScenery|Border, // literally scene bounding box (border)
    PlayerBorder = Player|Border,
    PlayerLegBorder = PlayerLeg|Border,
    PlayerPlayerLeg = Player|PlayerLeg, // literally scene bounding box (border)
    SceneryPlayerLeg = StaticScenery|PlayerLeg, // literally scene bounding box (border)
};

inline constexpr ActorType operator&(ActorType x, ActorType y){
    return static_cast<ActorType> (to_underlying(x) & to_underlying(y));
}   
inline constexpr ActorType operator|(ActorType x, ActorType y){
    return static_cast<ActorType> (to_underlying(x) | to_underlying(y));
}
inline std::ostream& operator<<(std::ostream& o, ActorType a){
    std::cout << to_underlying(a);
    return o;
}

// not full
ShapeType convertShapeType(b2ShapeType b2type);
// loses info
b2ShapeType convertShapeType(ShapeType mineType);

//has no update/destroy, instead is changed from move events and by higher-level enteties
struct Actor {
public:
    //both type in actor and actor in anything else
    //should be first for custom rtti :)
    ActorType actorType = {};
    PhysicalState state = {};
    PhysicalProperties properties = {};
    PhysicalBindings bindings = {};

    ShapeProps shapeProps = {};

    // i didn't want to use constructors, but it is really not doing anythinh
    // The road to unreadable code is paved with readability intentions
    Actor(ActorType actorType, b2BodyType bodyType, ShapeType shapeType, u8vec4 coloring_info, vec2 pos, ShapeProps shapeProps) : 
        actorType{actorType}, 
        state{.pos=pos},
        properties{ .color=coloring_info, .shape_type=shapeType, .body_type=bodyType},
        shapeProps(shapeProps) {}

    Shape constructActorShape() const;
};

#endif // __ACTOR_HPP__