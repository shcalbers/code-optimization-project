#pragma once

namespace Tmpl8
{

enum allignments
{
    BLUE,
    RED
};

class Tank
{
  public:
    Tank() = default;
    Tank(float pos_x, float pos_y, allignments allignment, Sprite* tank_sprite, Sprite* smoke_sprite, float tar_x, float tar_y, float collision_radius, int health, float max_speed);

    ~Tank();

    void Tick();

    vec2 GetPosition() const { return position; }
    float GetCollisionRadius() const { return collision_radius; }
    int GetHealth() const { return health; }
    bool IsActive() const { return active; }
    bool RocketReloaded() const { return reloaded; }
    allignments GetAllignment() const { return allignment; }

    void ReloadRocket();

    void Deactivate();
    bool Hit(int hit_value);

    void Draw(Surface* screen);

    int CompareHealth(const Tank& other) const;

    void Push(vec2 direction, float magnitude);

private:
    vec2 position;
    vec2 speed;
    vec2 target;

    int health;

    float collision_radius;
    vec2 force;

    float max_speed;
    float reload_time;

    bool reloaded;
    bool active;

    allignments allignment;

    int current_frame;
    Sprite* tank_sprite;
    Sprite* smoke_sprite;

};

} // namespace Tmpl8