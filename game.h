#pragma once

namespace Tmpl8
{

class Game
{

  public:

    void SetTarget(Surface* surface) { screen = surface; }
    void Init();
    void Shutdown();
    void Update(float deltaTime);
    void Draw();
    void Tick(float deltaTime);
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
    SpatialHasher<Tank*> tanks_hash = SpatialHasher<Tank*>({{-100, -100}, {1400, 1700}}, 25);
    KDTree red_tree;
    KDTree blue_tree;

    vector<Rocket> rockets;
    vector<Smoke> smokes;
    vector<Explosion> explosions;
    vector<Particle_beam> particle_beams;

    Font* frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;

    template<typename Callable_T>
    void RunParallel(const Callable_T& callable, int N, unsigned int max_threads = thread_count) noexcept;

    void splitmerge_tanks_health_p(std::vector<Tank*>& A, std::vector<Tank*>& B, UINT16 begin, UINT16 end, int d = 1) noexcept;
    void splitmerge_tanks_health(std::vector<Tank*>& A, std::vector<Tank*>& B, UINT16 begin, UINT16 end) noexcept;
    void merge_tanks_health(std::vector<Tank*>& A, std::vector<Tank*>& B, UINT16 begin, UINT16 middle, UINT16 end) noexcept;

};

}; // namespace Tmpl8