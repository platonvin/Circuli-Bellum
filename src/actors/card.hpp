#include "glm/ext/vector_uint3_sized.hpp"
struct Card {
    //multiplier. (0, +INF)
    float hp_buff = 1;
    float damage_buff = 1;
    float bullet_speed_buff = 1;
    float reload_time_buff = 1;
    float shoot_time_buff = 1;
    float knockback_buff = 1;
    float jump_buff = 1;
    int extra_jumps = +0;
    //cards do not impact on player size directly
    float bullet_radius_buff = 1;
    float block_delay_buff = 1;
    int lives_buff = +0;
    int ammunition_buff = +0;
    int bullet_bounce_buff = +0;
    glm::u8vec3 bullet_color_set = {};
    bool set_bullet_color = false;

    bool extra_bullets = false;
    bool slow_effect = false;
    bool life_steal = false;
    //delay damage
    bool explosive_bullets = false;
    bool homing_bullets = false;
    bool poison_effect = false;
    bool spawn_bombs_on_block = false;
    bool bullet_drill = false;
    bool stun_effect = false;
    bool teleport_on_block = false;
    bool spawn_cloud_on_bullet_hit = false;
    // bool summon_dark_powers = false;

    int additional_bullets = 0;
    float additional_damage = 0.0f;
    float splash_damage = 0.0f;
    float bullet_slow_percent = 0.0f;
    float damage_over_time = 0.0f; // Parasite/Poison
    float chasing_speed_buff = 1; // Chase or similar
    float life_steal_percent = 0.0f; // Leech, Taste of Blood, etc.
};

const Card Barrage {
    .damage_buff = 0.3,
    .reload_time_buff = 1.25,
    .ammunition_buff = 5,
    .extra_bullets = true,
    .additional_bullets = 4,
};

const Card BigBullet {
    .reload_time_buff = 1.25,
    .bullet_radius_buff = 1.25,
};

const Card BigFastBullets {
    .bullet_speed_buff = 3.0,
    .bullet_radius_buff = 3.0,
};


const Card BombsAway {
    .hp_buff = 1.3,
    .block_delay_buff = 1.25,
    .spawn_bombs_on_block = true,
};

const Card Bouncy {
    .damage_buff = 1.25,
    .reload_time_buff = 1.25,
    .bullet_bounce_buff = 2,
};

const Card Brawler {
    .hp_buff = 3.0,   // +200% HP for 3 seconds (temporary, not persistently)
};

const Card Buckshot {
    .damage_buff = 0.4,    // -60% damage => damage * (1 - 0.6) = 0.4
    .reload_time_buff = 1.25,
    .ammunition_buff = 5,
    .extra_bullets = true,
    .additional_bullets = 4,
};

const Card Burst {
    .damage_buff = 0.4,    // -60% damage
    .reload_time_buff = 1.25,
    .ammunition_buff = 3,
    .extra_bullets = true,
    .additional_bullets = 2,
};

const Card CarefulPlanning {
    .damage_buff = 2.0,    // +100% damage
    .reload_time_buff = 1.5,
    .shoot_time_buff = 0.5, // -150% attack speed means 50% attack speed
};

const Card Chase {
    .hp_buff = 1.3,
    .chasing_speed_buff = 1.6,
};

const Card ChillingPresence {
    .hp_buff = 1.25,
    .slow_effect = true,
};

const Card ColdBullets {
    .reload_time_buff = 1.25,
    .bullet_slow_percent = 0.3, // Bullets slow enemies by 70% (1 - 0.7)
};

// Add more cards following this structure...

const Card Leech {
    .hp_buff = 1.3,
    .life_steal_percent = 0.75,
};

const Card Phoenix {
    .hp_buff = 0.65, // -35% HP
    .lives_buff = 1, // Respawn once on death
};

const Card Parasite {
    .hp_buff = 1.25,
    .damage_buff = 1.25,
    .reload_time_buff = 1.25,
    .damage_over_time = 5.0, // Deal damage over 5 seconds
    .life_steal_percent = 0.5, // 50% life steal
};

const Card GlassCannon {
    .hp_buff = 0.0,     // -100% HP
    .damage_buff = 2.0, // +100% damage
    .reload_time_buff = 1.25,
};

const Card cards[] = {
    Barrage,
    BigBullet,
    BombsAway,
    Bouncy,
    Brawler,
    Buckshot,
    Burst,
    CarefulPlanning,
    Chase,
    ChillingPresence,
    ColdBullets,
    Leech,
    Phoenix,
    Parasite,
    GlassCannon,
};