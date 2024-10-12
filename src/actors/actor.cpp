#include "actors/actor.hpp"

b2ShapeType convertShapeType(ShapeType mineType) {
    switch (mineType) {
        case Circle:    return b2_circleShape;
        case Capsule:   return b2_capsuleShape;
        case Rectangle: return b2_polygonShape;
        case Trapezoid: return b2_polygonShape;
        default:
            assert(false);
    }
}
ShapeType convertShapeType(b2ShapeType b2type) {
    switch (b2type) {
        case b2_circleShape:  return Circle;
        case b2_capsuleShape: return Capsule;
        case b2_polygonShape: return Rectangle;
        default:
            assert(false);
    }
}

Shape Actor::constructActorShape() const {
    Shape shape = {};
        shape.coloring_info = properties.color;
        shape.pos = state.pos;
        shape.rot_angle = b2Rot_GetAngle({state.rot.x, state.rot.y});
        shape.rounding_radius = properties.rounding_radius;
        shape.props = shapeProps;
    return shape;
}