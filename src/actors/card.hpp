#include <cmath>

#define DEFINE_CARD(_name) const Card _name = {\
    .name = #_name,

#include "glm/ext/vector_uint3_sized.hpp"
#include <iostream>
struct Card {
    // Card Identification
    const char* name = ""; // The name of the card

    // --- Stat Buffs (Multipliers and Additives) ---
    
    // Health and Damage Buffs
    float hp_mul = 1.0f;                // *HP multiplier (0.0f = no change)
    float damage_mul = 1.0f;            // *Damage multiplier (1.0f = no change)

    // Bullet and Shooting Attributes
    float bullet_speed_mul = 1.0f;      // *Bullet speed multiplier (1.0f = no change)
    float reload_time_mul = 1.0f;       // *Reload time multiplier (1.0f = no change)
    float reload_time_add = 0.0f;       // +Flat increase to reload time
    float shoot_time_mul = 1.0f;        // *Time between shots multiplier (1.0f = no change)
    float spread_add = 0.0f;            // +Additional spread for bullets (0.0f = no additional spread)
    float bullet_radius_mul = 1.0f;     // +Flat addition to bullet radius
    float bullet_radius_add = 0.0f;     // +Flat addition to bullet radius
    int ammo_add = 0;                   // +Additional ammo capacity (0 = no additional ammo)

    // Player Movement and Control Buffs
    float knockback_add = 0.0f;         // +Knockback effect on hit (0.0f = no additional knockback)
    float jump_impulse_add = 0.0f;       // +Jump height increase (0.0f = no additional jump height)
    int jumps_add = 0;                  // +Additional jumps (0 = no extra jumps)
    float chase_speed_add = 0.0f;       // +Chase speed when moving towards the target
    float movement_speed_add = 0.0f;    // +Flat movement speed bonus

    // Defensive Buffs
    int lives_add = 0;                  // +Extra lives (0 = no extra lives)
    float block_cooldown_add = 0.0f;    // +Flat decrease to block cooldown

    // --- Weapon Modifiers and Effects ---

    // Bullet/Weapon Effects
    int bullet_bounces_add = 0;             // +Additional bullet bounces (0 = no bounces)
    int extra_bullets_per_shot_add = 0;     // +Extra bullets fired per shot (0 = normal shot)
    float splash_damage_add = 0.0f;         // +Flat splash damage (0.0f = no splash damage)
    float bullet_slow_percentage_add = 0.0f; // +Percentage slowdown applied by bullets (0.0f = no slow effect)
    float damage_over_time_add = 0.0f;      // +Damage over time (e.g., poison, parasite)
    float life_steal_percentage_add = 0.0f; // +Life steal percentage (0.0f = no life steal)
    float explosion_radius_add = 0.0f;      // +Explosion radius on bullet impact
    float homing_force_add = 0.0f;          // +Homing force applied to bullets
    float poison_damage_percentage_add = 0.0f; // +Percentage of damage converted to poison damage
    float stun_duration_add = 0.0f;         // +Stun duration (0.0f = no stun effect)

    // Special Effects on Block
    float teleport_distance_on_block_add = 0.0f; // +Teleport distance when blocking (0.0f = no teleport)
    int bombs_on_block_add = 0;                  // +Bombs spawned when blocking (0 = no bombs)
    float saw_damage_on_block_add = 0.0f;        // +Saw damage when blocking (0.0f = no saw)
    float dark_power_charge_time_add = 0.0f;     // +Dark power charge time (0.0f = no dark power)

    // Visual Effects
    glm::u8vec3 bullet_color = {255, 255, 255};  // =Bullet color (default white)
    bool has_custom_bullet_color = false;        // Whether the custom bullet color is active

    // Function to print card details
    void printCard() const {
        std::cout << "Card Name: " << name << "\n";

        // Print stat buffs
        std::cout << "Stat Buffs:\n";
        std::cout << "  HP Multiplier: " << hp_mul << "\n";
        std::cout << "  Damage Multiplier: " << damage_mul << "\n";
        std::cout << "  Bullet Speed Multiplier: " << bullet_speed_mul << "\n";
        std::cout << "  Reload Time Add: " << reload_time_add << "\n";
        std::cout << "  Reload Time Multiplier: " << reload_time_mul << "\n";
        std::cout << "  Shoot Time Multiplier: " << shoot_time_mul << "\n";
        std::cout << "  Ammo Add: " << ammo_add << "\n";
        std::cout << "  Bullet Radius Add: " << bullet_radius_add << "\n";
        
        // Print movement and control buffs
        std::cout << "Movement Buffs:\n";
        std::cout << "  Knockback Add: " << knockback_add << "\n";
        std::cout << "  Jump Height Add: " << jump_impulse_add << "\n";
        std::cout << "  Additional Jumps: " << jumps_add << "\n";
        std::cout << "  Chase Speed Add: " << chase_speed_add << "\n";
        std::cout << "  Movement Speed Add: " << movement_speed_add << "\n";
        std::cout << "  Extra Lives Add: " << lives_add << "\n";
        std::cout << "  Block Cooldown Add: " << block_cooldown_add << "\n";

        // Print weapon effects and modifiers
        std::cout << "Weapon Effects:\n";
        std::cout << "  Bullet Bounces Add: " << bullet_bounces_add << "\n";
        std::cout << "  Extra Bullets per Shot Add: " << extra_bullets_per_shot_add << "\n";
        std::cout << "  Splash Damage Add: " << splash_damage_add << "\n";
        std::cout << "  Bullet Slow Percentage Add: " << bullet_slow_percentage_add << "\n";
        std::cout << "  Damage Over Time Add: " << damage_over_time_add << "\n";
        std::cout << "  Life Steal Percentage Add: " << life_steal_percentage_add << "\n";
        std::cout << "  Explosion Radius Add: " << explosion_radius_add << "\n";
        std::cout << "  Homing Force Add: " << homing_force_add << "\n";
        std::cout << "  Poison Damage Percentage Add: " << poison_damage_percentage_add << "\n";
        std::cout << "  Stun Duration Add: " << stun_duration_add << "\n";

        // Print block effects
        std::cout << "Block Effects:\n";
        std::cout << "  Teleport Distance on Block: " << teleport_distance_on_block_add << "\n";
        std::cout << "  Bombs on Block Add: " << bombs_on_block_add << "\n";
        std::cout << "  Saw Damage on Block Add: " << saw_damage_on_block_add << "\n";
        std::cout << "  Dark Power Charge Time Add: " << dark_power_charge_time_add << "\n";

        // Print visual effects
        std::cout << "Visual Effects:\n";
        if (has_custom_bullet_color) {
            std::cout << "  Bullet Color: (" << (int)bullet_color.r << ", " 
                      << (int)bullet_color.g << ", " << (int)bullet_color.b << ")\n";
        } else {
            std::cout << "  Bullet Color: Default (White)\n";
        }
    }
};



DEFINE_CARD(Barrage)
    .damage_mul = 0.3f,
    .reload_time_add = 0.25f,
    .spread_add = 0.13f,
    .ammo_add = 5,
    .extra_bullets_per_shot_add = 4,
};

DEFINE_CARD(Big_bullet)
    .reload_time_add = 0.25f,
};

DEFINE_CARD(Bombs_away)
    .hp_mul = 1.3f,
    .block_cooldown_add = 0.25f,
    .bombs_on_block_add = 1,
};

DEFINE_CARD(Bouncy)
    .damage_mul = 1.25f,
    .reload_time_add = 0.25f,
    .bullet_bounces_add = 2,
};

DEFINE_CARD(Brawler)
    // No stat changes are provided,
};

DEFINE_CARD(Buckshot)
    .damage_mul = 0.4f,
    .reload_time_add = 0.25f,
    .shoot_time_mul = 3.5f,
    .spread_add = 0.5f,
    .ammo_add = 5,
    .extra_bullets_per_shot_add = 4,
};

DEFINE_CARD(Burst)
    .damage_mul = 0.6f,
    .reload_time_add = 0.25f,
    .spread_add = 0.04f,
    .ammo_add = 3,
    .extra_bullets_per_shot_add = 2,
};

DEFINE_CARD(Careful_planning)
    .damage_mul = 2.0f,
    .reload_time_add = 0.5f,
    .shoot_time_mul = 2.5f,
};

DEFINE_CARD(Chase)
    .hp_mul = 1.3f,
    .chase_speed_add = 0.6f,
};

DEFINE_CARD(Chilling_presence)
    .hp_mul = 1.25f,
    .bullet_slow_percentage_add = 0.25f,
};

DEFINE_CARD(Cold_bullets)
    .reload_time_add = 0.25f,
    .bullet_slow_percentage_add = 0.7f,
};

DEFINE_CARD(Combine)
    .damage_mul = 2.0f,
    .reload_time_add = 0.5f,
    .ammo_add = -2,
};

DEFINE_CARD(Dazzle)
    .reload_time_add = 0.25f,
    .stun_duration_add = 0.5f // Assuming stun duration based on the description,
};

DEFINE_CARD(Decay)
    .hp_mul = 1.5f,
};

DEFINE_CARD(Defender)
    .hp_mul = 1.3f,
    .block_cooldown_add = -0.3f,
};

DEFINE_CARD(Demonic_pact)
    .reload_time_add = 0.25f,
    .ammo_add = 9,
    .splash_damage_add = 2.0f,
};

DEFINE_CARD(Drill_ammo)
    .reload_time_add = 0.25f,
};

DEFINE_CARD(Echo)
    .hp_mul = 1.3f,
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Empower)
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Emp)
    .hp_mul = 1.3f,
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Explosive_bullet)
    .reload_time_add = 0.25f,
    .shoot_time_mul = 2.0f,
};

DEFINE_CARD(Fastball)
    .bullet_speed_mul = 3.5f,
    .reload_time_add = 0.25f,
    .shoot_time_mul = 1.5f,
};

DEFINE_CARD(Fast_forward)
    .bullet_speed_mul = 2.0f,
    .reload_time_mul = 0.7f,
};

DEFINE_CARD(Frost_slam)
    .hp_mul = 1.3f,
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Glass_cannon)
    .hp_mul = 0.5f,
    .damage_mul = 2.0f,
    .reload_time_add = 0.25f,
};

DEFINE_CARD(Grow)
    .reload_time_add = 0.25f,
};

DEFINE_CARD(Healing_field)
    .hp_mul = 1.3f,
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Homing)
    .damage_mul = 0.75f,
    .bullet_speed_mul = 0.7f,
    .shoot_time_mul = 1.5f,
};

DEFINE_CARD(Huge)
    .hp_mul = 1.8f,
};

DEFINE_CARD(Implode)
    .hp_mul = 1.5f,
    .block_cooldown_add = 0.25f,
};

DEFINE_CARD(Leech)
    .hp_mul = 1.3f,
    .life_steal_percentage_add = 0.75f,
};

DEFINE_CARD(Lifestealer)
    .hp_mul = 1.25f,
    .life_steal_percentage_add = 0.5f,
};

const Card cards[] = {
    Barrage,
    Big_bullet,
    Bombs_away,
    Bouncy,
    Brawler,
    Buckshot,
    Burst,
    Careful_planning,
    Chase,
    Chilling_presence,
    Cold_bullets,
    Combine,
    Dazzle,
    Decay,
    Defender,
    Demonic_pact,
    Drill_ammo,
    Echo,
    Empower,
    Emp,
    Explosive_bullet,
    Fastball,
    Fast_forward,
    Frost_slam,
    Glass_cannon,
    Grow,
    Healing_field,
    Homing,
    Huge,
    Implode,
    Leech,
    Lifestealer,
};