#include "Point.h"

int correct_quad(const Point &center, const Point &new_point)
{
    if (center.x > new_point.x &&
        center.y <= new_point.y)
        return NW;
    else if (center.x <= new_point.x &&
             center.y <= new_point.y)
        return NE;
    else if (center.x > new_point.x &&
             center.y > new_point.y)
        return SW;
    else
        return SE;
}

bool operator==(const Point &lhs, const Point &rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}