#include "actors/player.hpp"
#include "data_structures/dlist.hpp"
#include "field.hpp"

using namespace glm;

extern template class List<Player>;

bool Field::update(PhysicalWorld* world, List<Player>* players, float dTime){
    state.time_left -= dTime;
    bool ended = (state.time_left <= 0);
    bool ticks = (not state.one_shot) or (state.one_shot and ended);
    if(attached) pos = master->actor.state.pos;
    if(attached) visual_pos = master->visual_pos;
    for(mut p : *players){
        if((&p == master) and !impacts_master) continue;
        bool inside;
        inside = distance(p.actor.state.pos, pos) < p.props.radius + radius;
        if(inside and ticks){
            float multiplier = (state.one_shot) ? 1.0 : dTime;
            p.processField(this, multiplier);
        }
    }
    return ended;
}

void Field::draw(VisualView* view){    
    float left = state.time_left / state.duration;
    float fill = 1.0 - left;
        fill = sqrt(fill);
        // fill = 1;

    Shape shape = {};
        shape.coloring_info = color;
        shape.pos = visual_pos;
        // shape.rounding_radius = radius * fill;
        shape.props.CAPSULE_radius = radius;
        shape.props.value_2 = fill;
        shape.shapeType = Circle;
    view->draw_dynamic_shape(shape, coloring_type);
        shape.rounding_radius = 0.02;
        shape.props.CAPSULE_radius = (radius+0.01)*sqrtf(sqrtf(fill));
    view->draw_dynamic_shape(shape, SolidColor);
}

void setupFieldsFromPlayer(List<Field>* fields, Player* owner){
    Field generic = {};
        generic.pos = generic.visual_pos = owner->actor.state.pos;
        generic.master = owner;

    mut ops = owner->props;
    mut ora = owner->props.radius;
    // generic.state.suction
    if(ops.healing_field_heal > 0){
        Field healing = generic;
            healing.state.heal = ops.healing_field_heal;
            healing.radius = ops.healing_field_radius_rel * ora;
            healing.color = twpp::lime(300);
            healing.color.w = 185;
            healing.state.duration = healing.state.time_left = 1.2;
            healing.coloring_type = HealingFieldStyle;
            healing.state.one_shot = true;
            healing.impacts_master = true;
            healing.attached = false;
        fields->appendBack(healing);
    }

    if(ops.saw_damage > 0){
        static u8 unique_hash_seed = 0;
        // saw saw saw
        Field saw = generic;
            saw.state.damage = ops.saw_damage;
            saw.radius = ops.saw_radius_rel * ora;
            saw.color = twpp::rose(700);
            saw.color.w = unique_hash_seed++;
            saw.state.duration = saw.state.time_left = 3.3;
            saw.coloring_type = SawFieldStyle;
            saw.state.one_shot = false;
            saw.impacts_master = false;
            saw.attached = true;
        fields->appendBack(saw);
    }
}
