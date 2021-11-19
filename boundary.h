#pragma once

namespace Tmpl8
{

struct BoundingBox
{
    vec2 min;
    vec2 max;

    BoundingBox() noexcept = default;
    BoundingBox(const BoundingBox& other) noexcept = default;
    BoundingBox(BoundingBox&& other) = default;

    BoundingBox(vec2 min, vec2 max) noexcept;
    BoundingBox(vec2 center, float radius) noexcept;

    BoundingBox& operator=(const BoundingBox& other) noexcept = default;
    BoundingBox& operator=(BoundingBox&& other) noexcept = default;

    ~BoundingBox() noexcept = default;

};

inline BoundingBox::BoundingBox(const vec2 min, const vec2 max) noexcept
    : min(min), max(max)
{
    assert((min.x < max.x && min.y < max.y) && "'min' should be less than 'max'!");
}

inline BoundingBox::BoundingBox(const vec2 center, const float radius) noexcept
    : BoundingBox(center-radius, center+radius)
{
    // Empty.
}

inline bool contains(BoundingBox boundary, vec2 point) noexcept;

inline bool intersects(BoundingBox boundary, BoundingBox test) noexcept;

bool contains(const BoundingBox boundary, const vec2 point) noexcept
{
    return (boundary.min.x <= point.x && point.x <= boundary.max.x) &&
           (boundary.min.y <= point.y && point.y <= boundary.max.y);
}

bool intersects(const BoundingBox a, const BoundingBox b) noexcept
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y);
}

} // namespace Tmpl8
