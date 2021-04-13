#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "Template.h"

namespace Tmpl8
{

struct BoundingBox
{
    vec2 top_left;
    vec2 bottom_right;
};

inline bool contains(BoundingBox boundary, vec2 point) noexcept;

inline bool intersects(BoundingBox boundary, BoundingBox test) noexcept;

bool contains(BoundingBox boundary, vec2 point) noexcept
{
    return (boundary.top_left.x <= point.x && point.x <= boundary.bottom_right.x) &&
           (boundary.top_left.y <= point.y && point.y <= boundary.bottom_right.y);
}

bool intersects(BoundingBox a, BoundingBox b) noexcept
{
    return (a.top_left.x <= b.bottom_right.x && a.bottom_right.x >= b.top_left.x) &&
           (a.top_left.y <= b.bottom_right.y && a.bottom_right.y >= b.top_left.y);
}

}

#endif
