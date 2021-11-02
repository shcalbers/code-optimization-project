#ifndef SPATIAL_HASHER_H
#define SPATIAL_HASHER_H

#include "precomp.h"

template<typename T>
class SpatialHasher
{

public:

    struct Entry
    {
        vec2 position;
        T object;
    };

    SpatialHasher(vec2 lowerbound, vec2 upperbound, float cellSize);

    bool tryInsertAt(vec2 position, T gameObject);
    bool tryRemoveAt(vec2 position);

    template<typename Function_T>
    void getObjectsBetween(vec2 lowerbound, vec2 upperbound, Function_T function) const;

private:
   
    using Container_T = std::vector<std::vector<Entry>>;

    vec2 lowerbound, upperbound;
    float cellSize;

    int rows, cols;
    Container_T cells;

    static bool pointWithinBounds(vec2 position, vec2 lowerbound, vec2 upperbound) noexcept;

    int calculateIndex(vec2 position) const;

};

template<typename T>
SpatialHasher<T>::SpatialHasher(vec2 lowerbound, vec2 upperbound, float cellSize)
{
    this->lowerbound = lowerbound;
    this->upperbound = upperbound;
    this->cellSize = cellSize;

    this->rows = ceil((upperbound.y - lowerbound.y) / this->cellSize);
    this->cols = ceil((upperbound.x - lowerbound.x) / this->cellSize);
    this->cells = std::vector<std::vector<Entry>>(rows * cols);
}

template<typename T>
bool SpatialHasher<T>::tryInsertAt(vec2 position, T gameObject)
{
    if (pointWithinBounds(position, lowerbound, upperbound))
    {
        cells[calculateIndex(position)].push_back(Entry{position, gameObject});
        return true;
    }

    return false;
}

template<typename T>
bool SpatialHasher<T>::tryRemoveAt(vec2 position)
{
    if (pointWithinBounds(position, lowerbound, upperbound))
    {
        auto& cell = cells[calculateIndex(position)];
        for (auto it = cell.begin(); it != cell.end(); it++)
        {
            if (it->position.x == position.x && it->position.y == position.y)
            {
                cell.erase(it);
                return true;
            }
        }
    }

    return false;
}

template<typename T>
template<typename Function_T>
void SpatialHasher<T>::getObjectsBetween(vec2 lowerbound, vec2 upperbound, Function_T function) const
{
    static ThreadPool threadpool(std::thread::hardware_concurrency());
    std::atomic<int> running_tasks = 0;

    lowerbound = {max(this->lowerbound.x, lowerbound.x), max(this->lowerbound.y, lowerbound.y)};
    upperbound = {min(this->upperbound.x-1, upperbound.x), min(this->upperbound.y-1, upperbound.y)};
    for (int index = calculateIndex(lowerbound), last_index = calculateIndex({lowerbound.x, upperbound.y}), max_offset = ceil((upperbound.x - lowerbound.x) / cellSize); index <= last_index; index += cols)
    {
        for (int offset = 0; offset < max_offset; offset++)
        {
            if (auto& cell = cells[index + offset]; cell.size() >= 400 && running_tasks <= std::thread::hardware_concurrency())
            {
                running_tasks++;
                threadpool.enqueue([&]() -> void {
                    for (auto& entry : cell)
                    {
                        if (pointWithinBounds(entry.position, lowerbound, upperbound))
                            function(entry);
                    }

                    running_tasks--;
                });
            }
            else
            {
                for (auto& entry : cell)
                {
                    if (pointWithinBounds(entry.position, lowerbound, upperbound))
                        function(entry);
                }
            }
        }
    }

    while (running_tasks != 0) continue;
}

template<typename T>
bool SpatialHasher<T>::pointWithinBounds(vec2 position, vec2 lowerbound, vec2 upperbound) noexcept
{
    return (lowerbound.x < position.x && position.x < upperbound.x) && (lowerbound.y < position.y && position.y < upperbound.y);
}

template<typename T>
int SpatialHasher<T>::calculateIndex(vec2 position) const
{
    return int((floor((position.y - lowerbound.y) / cellSize) * cols) + floor((position.x - lowerbound.x) / cellSize));
}

#endif
