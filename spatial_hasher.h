#ifndef SPATIAL_HASHER_H
#define SPATIAL_HASHER_H

#include "precomp.h"

template <typename T>
class SpatialHasher
{

public:

    struct Data
    {
        vec2 position;
        T object;
    };

    SpatialHasher(int sceneWidth, int sceneHeight, int cellSize);

    void tryInsertAt(T gameObject, vec2 position);
    void tryRemoveAt(T gameObject, vec2 position);

    std::vector<Data> getObjectsBetween(vec2 lowerbound, vec2 upperbound) const;

    ~SpatialHasher() = default;

private:
   
    using Container_T = std::vector<std::vector<Data>>;

    int sceneWidth, sceneHeight;
    int cellSize;

    int rows, cols;
    Container_T cells;

    int calculateIndex(vec2 position) const;

};

template<typename T>
SpatialHasher<T>::SpatialHasher(int sceneWidth, int sceneHeight, int cellSize)
{
    this->sceneWidth = sceneWidth;
    this->sceneHeight = sceneHeight;
    this->cellSize = cellSize;

    this->rows = this->sceneHeight / this->cellSize + (this->sceneHeight % this->cellSize ? 1 : 0);
    this->cols = this->sceneWidth  / this->cellSize + (this->sceneWidth  % this->cellSize ? 1 : 0);
    this->cells = std::vector<Data>(rows * cols);
}

template<typename T>
void SpatialHasher<T>::tryInsertAt(T gameObject, vec2 position)
{

}

template<typename T>
void SpatialHasher<T>::tryRemoveAt(T gameObject, vec2 position)
{

}

template<typename T>
std::vector<SpatialHasher<T>::Data> SpatialHasher<T>::getObjectsBetween(vec2 lowerbound, vec2 upperbound) const
{

}

template<typename T>
int SpatialHasher<T>::calculateIndex(vec2 position) const
{

}

#endif
