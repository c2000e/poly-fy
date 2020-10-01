#ifndef POLYGON_H
#define POLYGON_H

#include "GL/glew.h"

#include <array>
#include <vector>

class Polygon
{
    private:
        // The RGBA color of a polygon represented with floats from 0 to 1.
        std::array<GLfloat, 4> color;

        // The center of a polygon in normalized coordinates.
        std::array<GLfloat, 2> center;

        // The radius of a polygon in normalized coordinates.
        GLfloat radius;

        // The rotation of the polygon scaled from 0 to 1.
        GLfloat rotation;

        // The fitness calculated for the polygon.
        float fitness = 0.0f;

    public:
        // Randomly generate a regular polygon.
        Polygon();

        // Generate a polygon through crossover.
        Polygon(Polygon p1, Polygon p2);

        // Push a polygon into a vector of floats.
        void pushTo(std::vector<GLfloat> &v);
};

#endif
