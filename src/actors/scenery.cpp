#include "scenery.hpp"

Shape Scenery::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Square;
        shape.shapeType = actor.properties.shape_type;
        //TODO?
    return shape;
}

void Scenery::addToWorld(PhysicalWorld* world){
    //for now, just box ¯\_(ツ)_/¯
    //TODO:
    b2Polygon poly;
    if(actor.properties.shape_type == Square){
        poly = b2MakeBox(actor.shapeProps.SQUARE_half_width, actor.shapeProps.SQUARE_half_height);
    } 
    else if(actor.properties.shape_type == Trapezoid){
        float bottom = actor.shapeProps.TRAPEZOID_half_bottom_size;
        float top = actor.shapeProps.TRAPEZOID_half_top_size;
        float height = actor.shapeProps.TRAPEZOID_half_height;
        b2Vec2 points[] = {
            {-bottom, -height},
            {+bottom, -height},
            {-top, +height},
            {+top, +height},
        };
        b2Hull hull = b2ComputeHull(points, 4);
        poly = b2MakePolygon(&hull, 0);
    } else {assert(false);}
    
    world->addActor<b2Polygon, b2CreatePolygonShape>(&actor.bindings, &actor.state, &actor.properties, this, &poly);
}

void Scenery::draw(VisualView* view){
    // view->draw_dynamic_shape(constructShape(), FBMstyle);
    view->draw_dynamic_shape(constructShape(), GRIDstyle);
}
