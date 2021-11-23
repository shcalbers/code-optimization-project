#pragma once

namespace Tmpl8
{

using TanksHasher = SpatialHasher<Tank*>;

template <>
class SpatialHasher<Tank*>
{

public:

    struct Entry
    {
        vec2 position;
        Tank* object;
    };

    SpatialHasher(BoundingBox boundary, float cellSize) noexcept;

    bool tryInsertAt(vec2 position, Tank* tank) noexcept;
    bool tryRemoveAt(vec2 position) noexcept;

    template <typename Callable_T>
    void forEachWithinBounds(BoundingBox boundary, const Callable_T& callable) const noexcept;

    Tank* findClosestEnemy(Tank* tank) noexcept;

private:

    using Container_T = std::vector<std::vector<Entry>>;

    BoundingBox boundary;
    float cellSize;

    int rows, cols;
    Container_T cells;

    std::vector<std::vector<int>> tanks_in_col;
    std::vector<std::vector<std::vector<int>>> tanks_in_cell;

    mutable std::vector<std::shared_mutex> row_mutices;
    mutable std::vector<std::shared_mutex> col_mutices;

    int calculateIndex(vec2 position) const noexcept;

    int calculateRowIndex(vec2 position) const noexcept;
    int calculateColIndex(vec2 position) const noexcept;

};

inline TanksHasher::SpatialHasher(BoundingBox boundary, float cellSize) noexcept
{
    static const auto bucket_prototype = []() noexcept
    {
        std::vector<Entry> v{};
        v.reserve(30);
        return v;
    }();

    this->boundary = boundary;
    this->cellSize = cellSize;

    this->rows = ceil((boundary.max.y + 1 - boundary.min.y) / this->cellSize);
    this->cols = ceil((boundary.max.x + 1 - boundary.min.x) / this->cellSize);

    this->cells = std::vector<std::vector<Entry>>(rows * cols, bucket_prototype);

    this->tanks_in_col = std::vector<std::vector<int>>(cols, {0, 0});
    this->tanks_in_cell = std::vector<std::vector<std::vector<int>>>(cols, std::vector<std::vector<int>>(rows, {0, 0}));

    this->row_mutices = std::vector<std::shared_mutex>(rows);
    this->col_mutices = std::vector<std::shared_mutex>(cols);
}

inline bool TanksHasher::tryInsertAt(vec2 position, Tank* tank) noexcept
{
    if (contains(boundary, position))
    {
        std::unique_lock<std::shared_mutex> lock_row(row_mutices[calculateRowIndex(position)], std::defer_lock);
        std::unique_lock<std::shared_mutex> lock_col(col_mutices[calculateColIndex(position)], std::defer_lock);
        std::lock(lock_row, lock_col);

        cells[calculateIndex(position)].push_back(Entry{position, tank});

        tanks_in_col[calculateColIndex(position)][tank->allignment]++;
        tanks_in_cell[calculateColIndex(position)][calculateRowIndex(position)][tank->allignment]++;
        return true;
    }

    return false;
}

inline bool TanksHasher::tryRemoveAt(vec2 position) noexcept
{
    if (contains(boundary, position))
    {
        std::unique_lock<std::shared_mutex> lock_row(row_mutices[calculateRowIndex(position)], std::defer_lock);
        std::unique_lock<std::shared_mutex> lock_col(col_mutices[calculateColIndex(position)], std::defer_lock);
        std::lock(lock_row, lock_col);

        auto& cell = cells[calculateIndex(position)];
        for (auto it = cell.begin(); it != cell.end(); it++)
        {
            if (it->position.x == position.x && it->position.y == position.y)
            {
                tanks_in_col[calculateColIndex(position)][it->object->allignment]--;
                tanks_in_cell[calculateColIndex(position)][calculateRowIndex(position)][it->object->allignment]--;
                cell.erase(it);
                return true;
            }
        }
    }

    return false;
}

template <typename Callable_T>
inline void TanksHasher::forEachWithinBounds(BoundingBox boundary, const Callable_T& callable) const noexcept
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
            const auto index = y * cols + x;
            std::shared_lock<std::shared_mutex> lock_row(row_mutices[y], std::defer_lock);
            std::shared_lock<std::shared_mutex> lock_col(col_mutices[x], std::defer_lock);
            std::lock(lock_row, lock_col);

            for (auto& entry : cells[index])
            {
                if (contains(boundary, entry.position))
                    callable(entry);
            }
        }
    }
}

inline Tank* TanksHasher::findClosestEnemy(Tank* tank) noexcept
{
    const auto tanks_row = calculateRowIndex(tank->position);
    const auto tanks_col = calculateColIndex(tank->position);
    const auto enemy_allignment = tank->allignment == BLUE ? RED : BLUE;

    // Find Closest Column with Enemies
    int closest_col = cols-1;

    // Check for closest column on the right first...
    for (int col = tanks_col; col < cols; col++)
    {
        std::shared_lock<std::shared_mutex> lock_col(col_mutices[col]);
        if (tanks_in_col[col][enemy_allignment] != 0)
        {
            closest_col = col;
            break;
        }
    }

    // Then check if there is a closer column on the left
    for (int col = tanks_col; col >= 0; col--)
    {
        std::shared_lock<std::shared_mutex> lock_col(col_mutices[col]);
        if (tanks_in_col[col][enemy_allignment] != 0)
        {
            if ((closest_col - tanks_col) >= (tanks_col - col))
            {
                closest_col = col;
            }
            break;
        }
    }

    // Find Closest Row with Enemies
    int closest_row = rows-1;

    // Check for closest column on the right first...
    for (int row = tanks_row; row < rows; row++)
    {
        std::shared_lock<std::shared_mutex> lock_row(row_mutices[row]);
        if (tanks_in_cell[closest_col][row][enemy_allignment] != 0)
        {
            closest_row = row;
            break;
        }
    }

    // Then check if there is a closer column on the left
    for (int row = tanks_row; row >= 0; row--)
    {
        std::shared_lock<std::shared_mutex> lock_row(row_mutices[row]);
        if (tanks_in_cell[closest_col][row][enemy_allignment] != 0)
        {
            if ((closest_row - tanks_row) >= (tanks_row - row))
            {
                closest_row = row;
            }
            break;
        }
    }

    // Find Closest Enemy
    Tank* closest_enemy = nullptr;
    float closest_distance = std::numeric_limits<float>::infinity();

    std::shared_lock<std::shared_mutex> lock_row(row_mutices[closest_row], std::defer_lock);
    std::shared_lock<std::shared_mutex> lock_col(col_mutices[closest_col], std::defer_lock);
    std::lock(lock_row, lock_col);

    for (const auto& entry : cells[closest_row*cols+closest_col])
    {
        if (entry.object->allignment != tank->allignment && entry.object->active)
        {
            float sqrDist = fabsf((entry.object->Get_Position() - tank->Get_Position()).sqrLength());
            if (sqrDist < closest_distance)
            {
                closest_distance = sqrDist;
                closest_enemy = entry.object;
            }
        }
    }

    return closest_enemy;
}

inline int TanksHasher::calculateIndex(vec2 position) const noexcept
{
    assert(contains(boundary, position) && "'position' should be within the boundaries of SpatialHasher!");
    return (int((position.y - boundary.min.y) / cellSize) * cols) + int((position.x - boundary.min.x) / cellSize);
}

inline int TanksHasher::calculateRowIndex(vec2 position) const noexcept
{
    return int((position.y - this->boundary.min.y) / cellSize);
}

inline int TanksHasher::calculateColIndex(vec2 position) const noexcept
{
    return int((position.x - this->boundary.min.x) / cellSize);
}

} // namespace Tmpl8
