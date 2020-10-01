#include "polygons/polygon.h"

#include <ctime>
#include <random>

std::default_random_engine generator(
        static_cast<long unsigned int>(std::time(0)));

std::uniform_real_distribution<GLfloat> randomFloat(0.0, 1.0);

Polygon::Polygon()
{
    for (GLfloat& colorComponent : color)
    {
        colorComponent = randomFloat(generator);
    }

    for (GLfloat& centerCoordinate : center)
    {
        centerCoordinate = randomFloat(generator);
    }

    radius = randomFloat(generator);
    rotation = randomFloat(generator);
}

void Polygon::pushTo(std::vector<GLfloat> &v)
{
    v.insert(v.end(), color.begin(), color.end());
    v.insert(v.end(), center.begin(), center.end());
    v.push_back(radius);
    v.push_back(rotation);
}
