#include "poly_fy/polygon.h"

#include <algorithm>
#include <ctime>
#include <random>

std::default_random_engine generator(
        static_cast<long unsigned int>(std::time(0)));

std::uniform_real_distribution<GLfloat> randomFloat(0.0, 1.0);

static GLfloat average(GLfloat x, GLfloat y)
{
    return 0.5 * (x + y);
}

static GLfloat clamp(GLfloat x, GLfloat min, GLfloat max)
{
    return std::max(min, std::min(x, max));
}

static GLfloat nudge(GLfloat x, GLfloat percent)
{
    GLfloat d = (2 * randomFloat(generator) - 1) * percent; 
    return clamp(x + d, 0, 1);
}

static GLfloat mutate_nudge(GLfloat x, double probability, GLfloat percent)
{
    if (randomFloat(generator) < probability)
    {
        return nudge(x, percent);
    }
    return x;
}

static GLfloat mutate_replace(GLfloat x, double probability)
{
    if (randomFloat(generator) < probability)
    {
        return randomFloat(generator);
    }
    return x;
}

Polygon::Polygon()
{
    for (int i = 0; i < genome.size(); i++)
    {
        genome[i] = randomFloat(generator);
    }
}

Polygon::Polygon(Polygon p1, Polygon p2)
{
    bool choice = randomFloat(generator) < 0.5;
    for (int i = 0; i < 4; i++)
    {
        if (choice)
        {
            genome[i] = p1.genome[i];
        }
        else
        {
            genome[i] = p2.genome[i];
        }
    }
    
    choice = randomFloat(generator) < 0.5;
    for (int i = 4; i < 6; i++)
    {
        if (choice)
        {
            genome[i] = p1.genome[i];
        }
        else
        {
            genome[i] = p2.genome[i];
        }
    }

    choice = randomFloat(generator) < 0.5;
    if (choice)
    {
        genome[6] = p1.genome[6];
    }
    else
    {
        genome[6] = p2.genome[6];
    }

    choice = randomFloat(generator) < 0.5;
    if (choice)
    {
        genome[7] = p1.genome[7];
    }
    else
    {
        genome[7] = p2.genome[7];
    }

    for (int i = 0; i < genome.size(); i++)
    {
        genome[i] = mutate_replace(genome[i], 0.005);
        genome[i] = mutate_nudge(genome[i], 0.05, 0.05);
    }
}

void Polygon::pushTo(std::vector<GLfloat> &v)
{
    v.insert(v.end(), genome.begin(), genome.end());
}

Polygon tournament(std::vector<Polygon>& population, int k, double p)
{
    k = population.size() > k ? k : population.size() - 1;

    std::shuffle(population.begin(), population.end(), generator);
    std::sort(population.begin(), population.begin() + k,
            [](Polygon a, Polygon b) { return a.fitness > b.fitness; });

    double choice =
        std::uniform_real_distribution<double>(0.0, 1.0)(generator);
    for (int i = 0; i < k; i++)
    {
        p += p * pow(1 - p, i);
        if (choice < p)
        {
            return population[i];
        }
    }
    return population[k - 1];
}

