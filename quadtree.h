#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "boundingbox.h"

#include <array>
#include <optional>
#include <list>
#include <iostream>

template <typename T>
struct Data
{
    vec2 position;
    T data;
};

template <typename T>
class QuadTreeNode
{

public:

    QuadTreeNode(BoundingBox boundary, int capacity);
    ~QuadTreeNode();

    BoundingBox getBoundary() const;
    int getCapacity() const;

    std::list<Data<T>> getPointsWithinBounds(BoundingBox boundary) const;

    //Find objects in given quadrant
    template<typename Callable>
    void forEachWithinBounds(BoundingBox boundary, Callable& callable) const;

    bool tryInsertAt(vec2 position, T data);
    bool tryRemoveAt(vec2 position);

private:

    std::array<std::array<QuadTreeNode*, 2>, 2> quadrants = {std::array<QuadTreeNode*, 2>{nullptr, nullptr}, std::array<QuadTreeNode*, 2>{nullptr, nullptr}};
    BoundingBox boundary;

    std::list<Data<T>> points;
    int capacity;

    bool isFull() const;

    bool isSubdivided() const;
    void subdivide();

    template <typename Callable>
    void forEachWithinBounds(BoundingBox boundary, Callable& callable, std::atomic<int>&) const;


};

template <typename T>
QuadTreeNode<T>::QuadTreeNode(BoundingBox boundary, int capacity)
{
    this->boundary = boundary;
    this->capacity = capacity;
}

template <typename T>
QuadTreeNode<T>::~QuadTreeNode()
{
    for (auto& row : this->quadrants)
    {
        for (auto& quadrant : row)
        {
            delete quadrant;
        }
    }
}

template <typename T>
bool QuadTreeNode<T>::isFull() const
{
    return this->points.size() == this->capacity;
}

template <typename T>
bool QuadTreeNode<T>::isSubdivided() const
{
    return this->quadrants[0][0] != nullptr;
}

template <typename T>
std::list<Data<T>> QuadTreeNode<T>::getPointsWithinBounds(BoundingBox boundary) const
{
    std::list<Data<T>> result;

    if (intersects(this->boundary, boundary))
    {
        for (const auto& point : this->points)
            if (contains(boundary, point.position)) result.push_back(point);

        if (this->isSubdivided())
        {
            for (const auto& row : this->quadrants)
            {
                for (const auto& quadrant : row)
                {
                    result.splice(result.end(), quadrant->getPointsWithinBounds(boundary));
                }
            }
        }
    }

    return result;
}

template <typename T>
template <typename Callable>
void QuadTreeNode<T>::forEachWithinBounds(BoundingBox boundary, Callable& callable) const
{
    std::atomic<int> a = 0;
    this->forEachWithinBounds(boundary, callable, a);

    //while (a > 0)
    //    ;
}

template <typename T>
template <typename Callable>
void QuadTreeNode<T>::forEachWithinBounds(BoundingBox boundary, Callable& callable, std::atomic<int>& a) const
{
    if (intersects(this->boundary, boundary))
    {
        for (const auto& point : this->points)
            if (contains(boundary, point.position)) callable(point);

        a++;
        auto task = ThreadPool::getInstance().enqueue([&]() -> void {
            a--;
        });

        if (this->isSubdivided())
        {
            for (const auto& row : this->quadrants)
            {
                for (const auto& quadrant : row)
                {       
                    quadrant->forEachWithinBounds(boundary, callable, a);
                }
            }
        }

        task.wait();
    }
}

template <typename T>
bool QuadTreeNode<T>::tryInsertAt(vec2 position, T object)
{
    if (!contains(this->boundary, position))
        return false;

    if (this->isFull())
    {
        if (!this->isSubdivided()) this->subdivide();

        for (auto& row : this->quadrants)
        {
            for (auto& quadrant : row)
            {
                if (quadrant->tryInsertAt(position, object))
                    return true;
            }
        }
    }
    else
    {
        this->points.push_back({position, object});
        return true;
    }
}

template <typename T>
bool QuadTreeNode<T>::tryRemoveAt(vec2 position)
{
    if (!contains(this->boundary, position))
        return false;

    for (auto it = this->points.begin(); it != this->points.end(); it++) {
        if (it->position.x == position.x && it->position.y == position.y) {
            this->points.erase(it);
            return true;
        }
    }

    if (this->isSubdivided())
    {
        for (const auto& row : this->quadrants)
        {
            for (const auto& quadrant : row)
            {
                if (quadrant->tryRemoveAt(position))
                    return true;
            }
        }
    }

    return false;
}

template <typename T>
void QuadTreeNode<T>::subdivide()
{
    assert((this->isSubdivided() == false) && "The Quad Tree Node has already been divided.");
    
    BoundingBox starting_quadrant = {this->boundary.top_left, (this->boundary.top_left + this->boundary.bottom_right) / 2.0};
    vec2 offset = starting_quadrant.bottom_right - starting_quadrant.top_left;

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++) {
            quadrants[y][x] = new QuadTreeNode(
                {starting_quadrant.top_left + vec2(x, y) * offset, starting_quadrant.bottom_right + vec2(x, y) * offset}, this->capacity
            );
        }
    }
}

template <typename T>
BoundingBox QuadTreeNode<T>::getBoundary() const
{
    return this->boundary;
}

template <typename T>
int QuadTreeNode<T>::getCapacity() const
{
    return this->capacity;
}

#endif
