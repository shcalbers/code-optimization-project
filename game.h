#pragma once

#include "spatial_hasher.h"

namespace Tmpl8
{
//forward declarations
class Tank;
class Rocket;
class Smoke;
class Particle_beam;

class Game
{
  public:
    void SetTarget(Surface* surface) { screen = surface; }
    void Init();
    void Shutdown();
    void Update(float deltaTime);
    void Draw();
    void Tick(float deltaTime);
    void insertion_sort_tanks_health(const std::vector<Tank*>& original, std::vector<const Tank*>& sorted_tanks, UINT16 begin, UINT16 end);
    void MeasurePerformance();

    Tank* FindClosestEnemy(Tank* current_tank);

    void MouseUp(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void MouseDown(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void MouseMove(int x, int y)
    { /* implement if you want to detect mouse movement */
    }

    void KeyUp(int key)
    { /* implement if you want to handle keys */
    }

    void KeyDown(int key)
    { /* implement if you want to handle keys */
    }

  private:

    static const unsigned int thread_count;
    ThreadPool pool{thread_count-1};

    mutex rockets_mutex;
    mutex smokes_mutex;
    mutex explosions_mutex;

    Surface* screen;

    vector<Tank*> tanks;
    SpatialHasher<Tank*> tanks_hash = SpatialHasher<Tank*>({0, 0}, {1280, 1700}, 25);

    vector<Rocket> rockets;
    vector<Smoke> smokes;
    vector<Explosion> explosions;
    vector<Particle_beam> particle_beams;

    Font* frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;

    template<typename Callable_T>
    void RunParallel(const Callable_T& callable, int N, unsigned int max_threads = thread_count) noexcept;

};

}; // namespace Tmpl8