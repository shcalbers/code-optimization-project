#pragma once

namespace Tmpl8
{

template <typename T>
class SpatialHasher
{

public:

    struct Entry
    {
        vec2 position;
        T object;
    };

    SpatialHasher(BoundingBox boundary, float cellSize) noexcept;

    bool tryInsertAt(vec2 position, T gameObject) noexcept;
    bool tryUpdateAt(vec2 old_position, vec2 new_position, T gameObject) noexcept;
    bool tryRemoveAt(vec2 position) noexcept;

    template <typename Callable_T>
    void forEachWithinBounds(BoundingBox boundary, const Callable_T& callable) const noexcept;

private:

    using Container_T = std::vector<std::vector<Entry>>;

    BoundingBox boundary;
    float cellSize;

    int rows, cols;
    Container_T cells;
    mutable std::vector<std::shared_mutex> mutices;

    int calculateIndex(vec2 position) const noexcept;

};

template <typename T>
inline SpatialHasher<T>::SpatialHasher(BoundingBox boundary, float cellSize) noexcept
{
    static const auto bucket_prototype = []() noexcept
    {
        std::vector<Entry> v{};
        v.reserve(30);
        return v;
    }();

    this->boundary = boundary;
    this->cellSize = cellSize;

    this->rows = ceil((boundary.max.y+1 - boundary.min.y) / this->cellSize);
    this->cols = ceil((boundary.max.x+1 - boundary.min.x) / this->cellSize);

    this->cells = std::vector<std::vector<Entry>>(rows*cols, bucket_prototype);
    this->mutices = std::vector<std::shared_mutex>(rows*cols);
}

template <typename T>
inline bool SpatialHasher<T>::tryInsertAt(vec2 position, T gameObject) noexcept
{
    if (contains(boundary, position))
    {
        std::unique_lock<std::shared_mutex> lock(mutices[calculateIndex(position)]);
        cells[calculateIndex(position)].push_back(Entry{position, gameObject});
        return true;
    }

    return false;
}

template <typename T>
inline bool SpatialHasher<T>::tryUpdateAt(vec2 old_position, vec2 new_position, T gameObject) noexcept
{
    if (contains(boundary, old_position))
    {
        if (contains(boundary, new_position))
        {
            const auto old_index = calculateIndex(old_position);
            const auto new_index = calculateIndex(new_position);

            std::unique_lock<std::shared_mutex> lock(mutices[old_index]);
            auto& cell = cells[old_index];
            for (auto it = cell.begin(); it != cell.end(); it++)
            {
                if (it->position.x == old_position.x && it->position.y == old_position.y)
                {
                    if (old_index == new_index)
                    {
                        // Object remains within the same cell, so just update its position to avoid costly reallocations.
                        it->position = new_position;
                    }
                    else
                    {
                        // Object moves to a different cell, so remove from the current and reinsert in the new cell.
                        cell.erase(it);
                        lock.unlock();

                        std::unique_lock<std::shared_mutex> lock(mutices[new_index]);
                        cells[new_index].push_back(Entry{new_position, gameObject});
                    }

                    return true;
                }
            }
        }
        else
        {
            // Object isn't within bounds anymore, so just remove it, without reinserting.
            return tryRemoveAt(old_position);
        }
    }
    else
    {
        // Object wasn't within the bounds before, but might be again so try to reinsert.
        return tryInsertAt(new_position, gameObject);
    }

    return false;
}

template <typename T>
inline bool SpatialHasher<T>::tryRemoveAt(vec2 position) noexcept
{
    if (contains(boundary, position))
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

template <typename T>
template <typename Callable_T>
inline void SpatialHasher<T>::forEachWithinBounds(BoundingBox boundary, const Callable_T& callable) const noexcept
{
    boundary.min = {max(this->boundary.min.x, boundary.min.x), max(this->boundary.min.y, boundary.min.y)};
    boundary.max = {min(this->boundary.max.x, boundary.max.x), min(this->boundary.max.y, boundary.max.y)};

    const auto y0 = int((boundary.min.y - this->boundary.min.y) / cellSize);
    const auto x0 = int((boundary.min.x - this->boundary.min.x) / cellSize);
    const auto yE = int((boundary.max.y - this->boundary.min.y) / cellSize);
    const auto xE = int((boundary.max.x - this->boundary.min.x) / cellSize);

    for (auto y = y0; y <= yE; y++)
    {
        for (auto x = x0; x <= xE; x++)
        {
            const auto index = y*cols + x;
            std::shared_lock<std::shared_mutex> lock(mutices[index]);
            for (auto& entry : cells[index])
            {
                if (contains(boundary, entry.position))
                    callable(entry);
            }
        }
    }
}

template <typename T>
inline int SpatialHasher<T>::calculateIndex(vec2 position) const noexcept
{
    assert(contains(boundary, position) && "'position' should be within the boundaries of SpatialHasher!");
    return (int((position.y - boundary.min.y) / cellSize) * cols) + int((position.x - boundary.min.x) / cellSize);
}

} // namespace Tmpl8
