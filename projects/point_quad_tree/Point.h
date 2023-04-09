#pragma once

#include <iostream>
#include <algorithm>
#include <climits>
#include <ostream>
#include <memory>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <time.h>
using namespace std;

enum CP
{
    NW,
    NE,
    SW,
    SE
}; // 0 1 2 3 4
const int NINF = INT_MIN, INF = INT_MAX;

class Point
{
public:
    int x, y;

    Point() : x(0), y(0){};
    Point(int x, int y) : x(x), y(y){};
};

int correct_quad(const Point &center, const Point &new_point);

bool operator==(const Point &lhs, const Point &rhs);