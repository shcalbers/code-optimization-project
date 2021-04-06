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

    vec2 Get_Position() const { return position; }
    float Get_Collision_Radius() const { return collision_radius; }
    int Get_Health() const { return health; }
    bool Get_Active() const { return active; }
    bool Rocket_Reloaded() const { return reloaded; }
    allignments Get_Allignment() const { return allignment; }

    void Reload_Rocket();

    void Deactivate();
    bool Hit(int hit_value);

    void Draw(Surface* screen);

    int Compare_Health(const Tank& other) const;

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