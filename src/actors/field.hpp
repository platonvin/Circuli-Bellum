#pragma once
#include "data_structures/dlist.hpp"
#ifndef __FIELD_HPP__
#define __FIELD_HPP__

#include "actor.hpp"
//fwd dcl
class Player;

struct FieldState{
    // float time_until_effect;
    bool one_shot; // if false than contignious
    float heal = 0;
    float damage = 0;
    float freezing = 0;
    float suction = 0;
    float pulling = 0;
    bool silence = false;

    float time_left;
    float duration;
};
struct FieldProps{
};

class Field {
public:
    //no actor

    Field() : 
        attached(true) 
        {} //TODO

    Field(Player* master, float radius, uvec4 color, bool attached) : 
        color(color),
        radius(radius),
        attached(attached),
        master(master)
        {} //TODO
    
    bool update(PhysicalWorld* world /*for later?*/, List<Player>* players, float dTime);

    struct FieldState state = {};
    vec2 pos = {};
    vec2 visual_pos = {};
    float radius = {};
    u8vec4 color = {};
    Player* master = nullptr;
    bool attached = false;
    bool impacts_master = false;

    Shape constructShape();
    void draw(VisualView* view);
    ColoringType coloring_type = SolidColor;

};
// class FieldManager
void setupFieldsFromPlayer(List<Field>* fields, Player* owner);
#endif // __FIELD_HPP__