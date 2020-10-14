#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "coord.hpp"
#include <iostream>

class Circle {
  private:
    Coord center;
    int rad;
  public:
    Circle (Coord c, int r) : center{c}, rad{r} { }

    void move(const Coord& c) {
        center += c;
    }

    void draw() const {
        std::cout << "circle at " << center
                  << " with radius " << rad << '\n';
    }
};

#endif