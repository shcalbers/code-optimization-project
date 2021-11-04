#ifndef SPATIAL_HASHER_H
#define SPATIAL_HASHER_H

#include "precomp.h"
#include "boundingbox.h"

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
    Entry findNearestNeighbour(vec2 position) const;

private:

    using Container_T = std::vector<std::vector<Entry>>;

    vec2 lowerbound, upperbound;
    float cellSize;

    int rows, cols;
    Container_T cells;

    static bool pointWithinBounds(vec2 position, vec2 lowerbound, vec2 upperbound) noexcept;

    int calculateIndex(vec2 position) const;
    vec2 calculateIndex2D(vec2 position) const;


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
std::vector<typename SpatialHasher<T>::Entry> SpatialHasher<T>::getObjectsBetween(vec2 lowerbound, vec2 upperbound) const
{
    std::vector<Entry> objects;

    lowerbound = {max(this->lowerbound.x, lowerbound.x), max(this->lowerbound.y, lowerbound.y)};
    upperbound = {min(this->upperbound.x-1, upperbound.x), min(this->upperbound.y-1, upperbound.y)};
    for (int index = calculateIndex(lowerbound), last_index = calculateIndex({lowerbound.x, upperbound.y}), max_offset = ceil((upperbound.x - lowerbound.x) / cellSize); index <= last_index; index += cols)
    {
        for (int offset = 0; offset < max_offset; offset++)
        {
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
typename SpatialHasher<T>::Entry SpatialHasher<T>::findNearestNeighbour(vec2 position) const
{
    position = vec2{ min(max(lowerbound.x, position.x), upperbound.x),
                     min(max(lowerbound.y, position.y), upperbound.y)  };

    float nearest_distance_sqr = std::numeric_limits<float>::infinity();
    const Entry* nearest_neighbour = nullptr;

    auto search_boundary = BoundingBox{vec2{0, 0}, vec2{float(cols-1), float(rows-1)}};
    auto search_index = calculateIndex2D(position);
    auto search_range = 3;

    for (const auto& entry : cells[search_index.y * cols + search_index.x])
    {
        auto sqr_dist = (position - entry.position).sqrLength();
        if (sqr_dist < nearest_distance_sqr)
        {
            nearest_distance_sqr = sqr_dist;
            nearest_neighbour = &entry;
        }
    }

    while (nearest_neighbour == nullptr)
    {
        enum Direction
        {
            RIGHT, DOWN, LEFT, UP, COUNT
        };

        static const vec2 step[Direction::COUNT] = {{+1, +0}, {+0, +1}, {-1, -0}, {-0, -1}};

        for (int direction = Direction::RIGHT; direction < Direction::COUNT; direction++)
        {
            for (auto i = 1; i < search_range; i++)
            {
                if (contains(search_boundary, search_index))
                {
                    auto& current_cell = cells[search_index.y * cols + search_index.x];

                    for (const auto& entry : current_cell)
                    {
                        auto sqr_dist = (position - entry.position).sqrLength();
                        if (sqr_dist < nearest_distance_sqr)
                        {
                            nearest_distance_sqr = sqr_dist;
                            nearest_neighbour = &entry;
                        }
                    }
                }

                search_index += step[direction];
            }
        }

        search_index += {-1, -1};
        search_range += 2;
    }

    return *nearest_neighbour;
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

template <typename T>
vec2 SpatialHasher<T>::calculateIndex2D(vec2 position) const
{
    return {floor((position.x - lowerbound.x) / cellSize), floor((position.y - lowerbound.y) / cellSize)};
}


#endif
