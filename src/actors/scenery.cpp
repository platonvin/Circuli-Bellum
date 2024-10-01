#include "scenery.hpp"

Shape Scenery::constructShape(){
    Shape shape = actor.constructActorShape();
        shape.shapeType = Square;
        shape.shapeType = convertShapeType(actor.properties.shape_type);
        //TODO?
    return shape;
}

void Scenery::addToWorld(PhysicalWorld* world){
    //for now, just box ¯\_(ツ)_/¯
    //TODO:
    b2Polygon poly = b2MakeBox(actor.shapeProps.SQUARE_half_width, actor.shapeProps.SQUARE_half_height);
    world->addActor<b2Polygon, b2CreatePolygonShape>(&actor.bindings, &actor.state, &actor.properties, this, &poly);
}