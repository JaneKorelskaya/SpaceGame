#pragma once
#include <iostream>
#include <cmath>
using namespace std;
struct Point
{
  double x = 0;
  double y = 0;

  Point() = default;
  Point (double _x, double _y) : x(_x), y(_y) {}

};

bool operator== (const Point & lhs, const Point & rhs);

bool operator!= (const Point & lhs, const Point & rhs);

ostream& operator<<(ostream& os, const Point& p);


