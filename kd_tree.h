#pragma once

namespace Tmpl8
{

class KDTree final
{

public:

    KDTree() noexcept = default;
    KDTree(std::vector<Tank*> values, int begin, int end) noexcept;
    KDTree(const KDTree& other) noexcept = delete;
    KDTree(KDTree&& other) noexcept = default;

    KDTree& operator=(const KDTree& tree) noexcept = delete;
    KDTree& operator=(KDTree&& tree) noexcept = default;

    void rebuild() noexcept;

    Tank* findNearestNeighbour(vec2 point) noexcept;
    Tank* findNearestNeighbour_norecursion(vec2 point) noexcept;

    ~KDTree() noexcept = default;

private:

    enum Axis
    {
        X = 0, Y = 1
    };

	struct Node
    {
        Tank* value = nullptr;
        Axis axis = Axis::X;

        Node* left = nullptr;
        Node* right = nullptr;
	};

    std::vector<Node> nodes;
    Node* root;

    Node* build(int begin, int end, int depth = 0) noexcept;
    void findNearestNeighbour(const vec2& point, Node* node, Tank*& closest_tank, float& closest_distance) noexcept;

};

inline KDTree::KDTree(std::vector<Tank*> values, int begin, int end) noexcept
{
    this->nodes.reserve(end-begin);
    for (int i = begin; i < end; i++) {
        nodes.push_back(Node{values[i]});
    }

    this->root = build(0, nodes.size());
}

inline void KDTree::rebuild() noexcept
{
    this->root = build(0, nodes.size());
}

inline KDTree::Node* KDTree::build(int begin, int end, int depth) noexcept
{
    if (begin >= end) return nullptr;

    Axis axis = Axis(depth % 2);
    const auto middle = begin + (end - begin) / 2;

    { // Partially sort 'nodes' such that the object pointed to by middle_it appears at the same index as if 'nodes' had been fully sorted.
        const auto begin_it = this->nodes.begin() + begin;
        const auto middle_it = this->nodes.begin() + middle;
        const auto end_it = this->nodes.begin() + end;
        const auto comparator = [=](const Node& a, const Node& b) noexcept { return a.value->position.cell[axis] < b.value->position.cell[axis]; };
        std::nth_element(begin_it, middle_it, end_it, comparator);
    } // Done sorting

    Node& current_node = this->nodes[middle];
    current_node.axis = axis;
    current_node.left = build(begin, middle, depth + 1);
    current_node.right = build(middle + 1, end, depth + 1);
    
    return &current_node;
}

inline Tank* KDTree::findNearestNeighbour(vec2 point) noexcept
{
    Tank* closest_tank = nullptr;
    float closest_distance = std::numeric_limits<float>::infinity();
    findNearestNeighbour(point, this->root, closest_tank, closest_distance);

    return closest_tank;
}

inline void KDTree::findNearestNeighbour(const vec2& point, Node* node, Tank*& closest_tank, float& closest_distance) noexcept
{
    if (node == nullptr) return;

    if (node->value->active)
    {
        const auto distance = (node->value->position - point).sqrLength();
        if (distance < closest_distance)
        {
            closest_distance = distance;
            closest_tank = node->value;
        }
    }

    if (closest_distance == 0)
        return;

    float dx = node->value->position.cell[node->axis] - point.cell[node->axis];
    findNearestNeighbour(point, dx > 0 ? node->left : node->right, closest_tank, closest_distance);
    if (dx * dx >= closest_distance)
        return;
    findNearestNeighbour(point, dx > 0 ? node->right : node->left, closest_tank, closest_distance);
}

inline Tank* KDTree::findNearestNeighbour_norecursion(vec2 point) noexcept
{
    Tank* closest_tank = nullptr;
    float closest_distance = std::numeric_limits<float>::infinity();

    std::deque<Node*> nodes;
    nodes.push_back(this->root);

    while (!nodes.empty())
    {
        Node* current_node = nodes.front();
        nodes.pop_front();

        if (current_node == nullptr) continue;

        if (current_node->value->active)
        {
            const auto distance = (current_node->value->position - point).sqrLength();
            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_tank = current_node->value;
            }
        }

        if (closest_distance == 0)
            break;

        float dx = current_node->value->position[current_node->axis] - point[current_node->axis];
        nodes.push_back(dx > 0 ? current_node->left : current_node->right);
        if (dx * dx >= closest_distance) continue;
        nodes.push_back(dx > 0 ? current_node->right : current_node->left);
    }

    return closest_tank;
}

} // namespace Tmpl8
