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

    std::vector<Entry> getObjectsBetween(vec2 lowerbound, vec2 upperbound) const;

private:
   
    using Container_T = std::vector<std::vector<Entry>>;

    vec2 lowerbound, upperbound;
    float cellSize;

    int rows, cols;
    Container_T cells;
    mutable std::vector<std::shared_mutex> mutices;

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
    this->mutices = std::vector<std::shared_mutex>(rows * cols);
}

template<typename T>
bool SpatialHasher<T>::tryInsertAt(vec2 position, T gameObject)
{
    if (pointWithinBounds(position, lowerbound, upperbound))
    {
        std::unique_lock<std::shared_mutex> lock(mutices[calculateIndex(position)]);
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
        std::unique_lock<std::shared_mutex> lock(mutices[calculateIndex(position)]);
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
std::vector<typename SpatialHasher<T>::Entry> SpatialHasher<T>::getObjectsBetween(vec2 lowerbound, vec2 upperbound) const
{
    std::vector<Entry> objects;

    lowerbound = {max(this->lowerbound.x, lowerbound.x), max(this->lowerbound.y, lowerbound.y)};
    upperbound = {min(this->upperbound.x-1, upperbound.x), min(this->upperbound.y-1, upperbound.y)};
    for (int index = calculateIndex(lowerbound), last_index = calculateIndex({lowerbound.x, upperbound.y}), max_offset = ceil((upperbound.x - lowerbound.x) / cellSize); index <= last_index; index += cols)
    {
        for (int offset = 0; offset < max_offset; offset++)
        {
            std::shared_lock<std::shared_mutex> lock(mutices[index + offset]);
            for (auto& entry : cells[index + offset])
            {
                if (pointWithinBounds(entry.position, lowerbound, upperbound))
                    objects.push_back(entry);
            }
        }
    }

    return objects;
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
