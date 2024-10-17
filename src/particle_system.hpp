#include "visual.hpp"

struct ParticleSystem {
public:
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
    float avg_radius = 0;
    vector<Particle> particles;

    void addParticle(u8vec4 color, vec2 pos, vec2 vel, float size, float lifetime);
    void addEffect(
        u8vec4 baseColor, u8 colorVariation, 
        vec2 basePos, float posVariation, 
        vec2 baseVel, float velVariation, 
        float baseSize, float sizeVariation, 
        float baseLifetime, float lifetimeVariation, 
        int numParticles);

    void update(float dTime);
};