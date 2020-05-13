#include "point.h"

bool
operator== (const Point & lhs, const Point & rhs)
{
  if (abs(lhs.x - rhs.x) < 0.0001 && abs(lhs.y - rhs.y) < 0.0001)
    return true;
  return false;
}

bool
operator!= (const Point & lhs, const Point & rhs)
{
  return !(lhs == rhs);
}

ostream& operator<<(ostream& os, const Point& p) {
        os << "(" << p.x << "; " << p.y << ")" ;
        return os;
}

