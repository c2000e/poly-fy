#ifndef POLYGON_H
#define POLYGON_H

#include "glad/glad.h"

#include <array>
#include <vector>

struct Polygon
{
    public:
        std::array<GLfloat, 8> genome;

        GLfloat fitness = 0.0f;

    public:
        Polygon();
        Polygon(Polygon p1, Polygon p2);

        void pushTo(std::vector<GLfloat> &v);
};

Polygon tournament(std::vector<Polygon>& population, int k, double p);

#endif
