#include "scenery.hpp"

Shape Scenery::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Rectangle;
        shape.shapeType = actor.properties.shape_type;
        //TODO?
    return shape;
}

void Scenery::addToWorld(PhysicalWorld* world){
    b2Polygon poly;
    b2Filter sceneryFilter = {};
        sceneryFilter.categoryBits = to_underlying(ActorType::Scenery);
        sceneryFilter.maskBits = to_underlying(ActorType::Projectile|ActorType::Player|ActorType::Scenery|ActorType::PlayerLeg);
        pl((sceneryFilter.maskBits));
    b2ShapeDef shape = b2DefaultShapeDef();
        shape.filter = sceneryFilter;

    const float rounding = 0.15;
    actor.properties.rounding_radius = rounding;
    if(actor.properties.shape_type == Rectangle){
        poly = b2MakeRoundedBox(actor.shapeProps.RECTANGLE_half_width-rounding, actor.shapeProps.RECTANGLE_half_height-rounding, rounding);
        world->addActor<b2Polygon, b2CreatePolygonShape>(&actor.bindings, &actor.state, &actor.properties, this, &poly, nullptr, &shape);
    } 
    else if(actor.properties.shape_type == Trapezoid){
        float bottom = actor.shapeProps.TRAPEZOID_half_bottom_size-rounding;
        float top = actor.shapeProps.TRAPEZOID_half_top_size-rounding;
        float height = actor.shapeProps.TRAPEZOID_half_height-rounding;
        b2Vec2 points[] = {
            {-bottom, -height},
            {+bottom, -height},
            {-top, +height},
            {+top, +height},
        };
        b2Hull hull = b2ComputeHull(points, 4);
        poly = b2MakePolygon(&hull, rounding);
        world->addActor<b2Polygon, b2CreatePolygonShape>(&actor.bindings, &actor.state, &actor.properties, this, &poly, nullptr, &shape);
    } else if(actor.properties.shape_type == Circle){
        b2Circle circle = {};
            circle.radius = actor.shapeProps.CIRCLE_radius;
        // b2ShapeDef shape = b2DefaultShapeDef();
        //     shape.restitution = 0.1;
        //     shape.friction = 0.05;
        // b2BodyDef bdef = b2DefaultBodyDef();
        //     bdef.fixedRotation = true;

        world->addActor<b2Circle, b2CreateCircleShape>(&actor.bindings, &actor.state, &actor.properties, this, &circle, nullptr, &shape);
    } else if(actor.properties.shape_type == Capsule){
        b2Capsule capsule = {};
            capsule.radius = actor.shapeProps.CIRCLE_radius;
            capsule.center1 = {-actor.shapeProps.CAPSULE_half_length};
            capsule.center2 = {+actor.shapeProps.CAPSULE_half_length};
        // b2ShapeDef shape = b2DefaultShapeDef();
        //     shape.restitution = 0.1;
        //     shape.friction = 0.05;
        // b2BodyDef bdef = b2DefaultBodyDef();
        //     bdef.fixedRotation = true;

        world->addActor<b2Capsule, b2CreateCapsuleShape>(&actor.bindings, &actor.state, &actor.properties, this, &capsule, nullptr, &shape);
    } 
    else {assert(false);}
    
}

void Scenery::draw(VisualView* view){
    // view->draw_dynamic_shape(constructShape(), FBMstyle);
    view->draw_dynamic_shape(constructShape(), FBMstyle);
}
