#pragma once
#include "Point.h"

class Rectangle
{
public:
    double xmin, xmax, ymin, ymax;

    Rectangle() : xmin(0), ymin(0), xmax(0), ymax(0) {}
    Rectangle(double xmin, double ymin, double xmax, double ymax) : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax) {}

    bool contains(const Point &p) const
    {
        return (p.x >= xmin && p.x <= xmax) && (p.y >= ymin && p.y <= ymax);
    }

    bool intersects(const Rectangle &other) const
    {
        return (xmin <= other.xmax && xmax >= other.xmin) && (ymin <= other.ymax && ymax >= other.ymin);
    }

    void print() const
    {
        std::cout << "R: (" << xmin << ", " << ymin << ") (" << xmax << ", " << ymax << ")" << std::endl;
    }
};