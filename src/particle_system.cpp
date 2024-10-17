#include "particle_system.hpp"

float randFloat(float min, float max);
int randInt(int min, int max);

void ParticleSystem::addParticle(u8vec4 color, vec2 pos, vec2 vel, float size, float lifetime){
    Particle p = {};
        p.shape.coloring_info = color;
        p.shape.shapeType = Circle;
        p.shape.props = {.CIRCLE_radius = size};
        p.shape.pos = pos;
        p.vel = vel;
    p.lifetime = lifetime;

    particles.push_back(p);
}

void ParticleSystem::addEffect(
    u8vec4 baseColor, u8 colorVariation, 
    vec2 basePos, float posVariation, 
    vec2 baseVel, float velVariation, 
    float baseSize, float sizeVariation, 
    float baseLifetime, float lifetimeVariation, 
    int numParticles
) {
    for (int i = 0; i < numParticles; i++) {
        vec2 particlePos = basePos + vec2(randFloat(-posVariation, posVariation), randFloat(-posVariation, posVariation));
        vec2 particleVel = baseVel + vec2(randFloat(-velVariation, velVariation), randFloat(-velVariation, velVariation));

        u8vec4 modifiedColor = {
            glm::clamp(int(baseColor.x) + randInt(-colorVariation, colorVariation), 0, 255),
            glm::clamp(int(baseColor.y) + randInt(-colorVariation, colorVariation), 0, 255),
            glm::clamp(int(baseColor.z) + randInt(-colorVariation, colorVariation), 0, 255),
            baseColor.w
        };

        float particleSize = glm::max(baseSize + randFloat(-sizeVariation, sizeVariation), 0.01f);
        float particleLifetime = glm::max(baseLifetime + randFloat(-lifetimeVariation, lifetimeVariation), 0.01f);

        if(
            (particleLifetime > 0) &&
            (particleSize > 0) &&
            true
        ){
            addParticle(modifiedColor, particlePos, particleVel, particleSize, particleLifetime);
        }
    }
}

void ParticleSystem::Particle::draw(VisualView* view){
    view->draw_dynamic_shape(shape, SolidColor);
}
void ParticleSystem::Particle::update(double dTime){
    shape.props.CIRCLE_radius *= (1.0 - (double)dTime / (double)lifetime);
}

void ParticleSystem::update(float dTime){
    //soft limit
    const float TARGET_PARTICLE_COUNT = 10000; //maybe thats too little
    // const float    MAX_PARTICLE_COUNT = 100; //maybe thats too little
    //higher bias -> particles removed earlier  
    float current_bias = 0.006;
    if(particles.size() > TARGET_PARTICLE_COUNT){
        float ratio = particles.size() / TARGET_PARTICLE_COUNT; //>1
        //square for aggressive clamping
        //TODO: clamp particle count?
        current_bias = current_bias * ratio*ratio;
    }
    // avg_radius = 0;

    // pl(current_bias)
    // pl(particles.size())
    
    // Basically processes and removes dead ones
    int write_index = 0;
    for (int i = 0; i < particles.size(); i++) {
        //TODO:
        // bool should_keep = particles[i].shape.props.CIRCLE_radius > 0.001;
        bool should_keep = particles[i].shape.props.CIRCLE_radius > current_bias; // TODO until visible
        // avg_radius += particles[i].shape.props.CIRCLE_radius;
        if (should_keep) {
            particles[write_index] = particles[i];
            particles[write_index].shape.pos += particles[write_index].vel * float(dTime);
            // particles[write_index].lifetime -= dTime; // if enabled, then dTime/(0 /-value) might happen
            particles[write_index].shape.props.CIRCLE_radius *= exp(-dTime/particles[write_index].lifetime);
            write_index++;
        }
    }
    // avg_radius /= float(write_index);
    particles.resize (write_index);
}