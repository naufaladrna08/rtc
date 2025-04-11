#ifndef INTERVAL_H
#define INTERVAL_H
#include "utils.h"

class interval {
  public:
    double min, max;

    interval() : min(+infinity), max(-infinity) { }
    interval(double min, double max) : min(min), max(max) { }

    double size() const {
      return max - min;
    }

    // TODO: check book
    double contains(double x) const {
      return x >= min && x <= max;
    }

    // TODO: check book
    bool surrounds(double x) const {
      return x > min && x < max;
    }
    
    double clamp(double x) const {
      if (x < min) return min;
      if (x > max) return max;
      return x;
    }

    static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif