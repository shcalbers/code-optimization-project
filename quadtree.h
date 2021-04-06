#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "point2D.h"

#include <optional>
#include <list>

template <typename T>
class QuadTreeNode;

struct BoundingBox
{
    vec2 top_left;
    vec2 bottom_right;
};

template <typename T>
bool BoundingBoxContainsData(BoundingBox boundary, QuadTreeNodeData<T> data);

inline 
bool BoundingBoxIntersectsBoundingBox(BoundingBox boundary, BoundingBox test);

template <typename T>
class QuadTreeNode
{

public:

    struct Data
    {
        vec2 position;
        T data;
    };

    QuadTreeNode(BoundingBox boundary, int capacity);
    ~QuadTreeNode();

    BoundingBox getBoundary() const;
    int getCapacity() const;

    //Find objects in given quadrant
    template<typename Callable>
    void forEachWithinBounds(BoundingBox boundary, Callable& callable) const;

    bool insertAt(vec2 position, T data);
    bool removeAt(vec2 position);

private:

    std::array<std::array<QuadTreeNode*, 2>, 2> quadrants;
    BoundingBox boundary;

    std::vector<Data<T>> points;
    int capacity;

    bool isFull() const;

    bool isDivided() const;
    void subdivide();

};

template <typename T>
class QuadTree : public QuadTreeNode<T>
{

public:
    
    QuadTree(QuadTreeNodeData<T>* data, BoundingBox boundary, int capacity);

};

template <typename T>
bool BoundingBoxContainsData(BoundingBox boundary, QuadTreeNode<T>::Data data)
{

    bool containsX = boundary.top_left.x <= data.position.x && data.position.x <= boundary.bottom_right.x;
    bool containsY = boundary.top_left.y <= data.position.y && data.position.y <= boundary.bottom_right.y;

    return containsX && containsY;
}

inline 
bool BoundingBoxIntersectsBoundingBox(BoundingBox boundary, BoundingBox test)
{
    return (boundary.top_left.x <= test.bottom_right.x && boundary.bottom_right.x >= test.top_left.x && boundary.top_left.y <= test.bottom_right.y && boundary.bottom_right.y >= test.top_left.y);
}

template <typename T>
QuadTreeNode<T>::QuadTreeNode(BoundingBox boundary, int capacity)
{
    this->boundary = boundary;
    this->capacity = capacity;
    this->points.reserve(capacity);
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

template <typename T, typename Callable>
void QuadTreeNode<T>::forEachWithinBounds(BoundingBox boundary, Callable& callable) const
{

    // If range is not contained in the node's boundingBox then bail
    if (!BoundingBoxIntersectsBoundingBox(this->boundary, boundary))
    {
        return;
    }

    for (int i = 0; i < this->count; i++)
    {
        // Gather points contained in range
        if (BoundingBoxContainsData(boundary, this->points[i]))
        {
            block(this->points[i]);
        }
    }

    // Bail if node is leaf
    if (this->northWest == NULL)
    {
        return;
    }

    // Otherwise traverse down the tree
    this->northWest->gatherDataWithinBoundary(boundary, block);
    this->northEast->gatherDataWithinBoundary(boundary, block);
    this->southWest->gatherDataWithinBoundary(boundary, block);
    this->southEast->gatherDataWithinBoundary(boundary, block);
}

template <typename T>
bool QuadTreeNode<T>::insertAt(vec2 position, T object)
{

    // Return if our coordinate is not in the boundingBox
    if (!BoundingBoxContainsData(this->boundary, data))
    {
        return false;
    }

    // Add the coordinate to the points array.
    if (this->count < this->capacity)
    {
        this->points[this->count++] = data;
        return true;
    }

    // Check to see if the current node is a leaf, if it is, split.
    if (this->northWest == NULL)
    {
        this->subdivide();
    }

    // Traverse the tree
    if (this->northWest->insert(data)) return true;
    if (this->northEast->insert(data)) return true;
    if (this->southWest->insert(data)) return true;
    if (this->southEast->insert(data)) return true;

    // Default. Was unable to add the node.
    return false;
}

template <typename T>
bool QuadTreeNode<T>::removeAt(vec2 position)
{

}

template <typename T>
void QuadTreeNode<T>::subdivide()
{
    assert((this->isDivided() == false) && "The Quad Tree Node has already been divided.");
    
    BoundingBox starting_quadrant = {this->boundary.top_left, (this->boundary.top_left + this->boundary.bottom_right) / 2.0};
    vec2 offset = (bottom_right - top_left) / 2.0;

    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++) {
            vec2 offset = ;

            quadrants[y][x] = new QuadTreeNode(
                {bottom_right + {x, y}*offset, top_left + {x, y}*offset}, this->capacity
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

template <typename T>
QuadTree<T>::QuadTree(QuadTreeNodeData<T>* data, BoundingBox bourdary, int capacity) : QuadTreeNode(boundary, capacity)
{

    for (int i = 0; i < sizeof(data) / sizeof(QuadTreeNodeData<T>); i++)
    {
        this->insert(data[i]);
    }
}

template <typename T>
QuadTree<T>* QuadTree<T>::QuadTreeMake(QuadTreeNodeData<T>* data, BoundingBox boundary, int capacity)
{
    QuadTree* tree = new QuadTree(data, boundary, capacity);
    return tree;
}

#endif
