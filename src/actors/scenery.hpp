#pragma once
#ifndef __SCENERY_HPP__
#define __SCENERY_HPP__

#include "actor.hpp"

class Scenery {
public:
    //first for custom rtti :)
    Actor actor;
    Scenery() : 
        actor(ActorType::StaticScenery, b2_staticBody, Circle, \
            twpp::pink(700), vec2(0), {.value_1 = 1., .value_2 = 1.}),
        props{} {} //TODO

    Scenery(ShapeType type, ShapeProps shapeProps, vec2 pos, bool dynamic) : 
        actor(ActorType::StaticScenery, (dynamic? b2_dynamicBody : b2_staticBody), (type), \
            twpp::pink(700), pos, shapeProps),
        props{} {} //TODO
    
    void update(PhysicalWorld* world /*for later?*/){}
    void addToWorld(PhysicalWorld* world, ActorType extraType = ActorType::StaticScenery);

    struct SceneryState state;
    struct SceneryProps props;


    Shape constructShape();
    void draw(VisualView* view, ColoringType style = SolidColor);
};
#endif // __SCENERY_HPP__