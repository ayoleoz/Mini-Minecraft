#include "biome.h"
#include <math.h>
#include <iostream>
#include <glm_includes.h>
#include <glm/glm.hpp>

const float PI = 3.1415926f;

float perlinNoise(float x, float z);

Biome::Biome()
{}

float CosineInterpolate(float a1, float a2, float mu)
{
    float mu2 = (1 - cos(mu * PI)) / 2.f;

    return (a1 * (1 - mu2) + a2 * mu2);
}

// copied from p34 of lecture note "Noise Functions"
glm::vec2 random2(glm::vec2 p)
{
    return glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)),
    glm::dot(p, glm::vec2(269.5,183.3))))
                      * 43758.5453f);
}

glm::vec3 random3(glm::vec3 p)
{
return glm::fract(glm::sin(glm::vec3(glm::dot(p, glm::vec3(127.1, 311.7, 191.999)),
                                     glm::dot(p, glm::vec3(269.5, 183.3, 211.999)),
                                     glm::dot(p, glm::vec3(420.6, 631.2, 333.999)))) * 43758.5453f);
}

// copied from p35 of lecture note "Noise Functions"
float worleyNoise(float x, float z)
{
    glm::vec2 uv = glm::vec2(x, z);
    uv *= 10.0; // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.
    for(int y = -1; y <= 1; ++y)
    {
        for(int x = -1; x <= 1; ++x)
        {
            glm::vec2 neighbor = glm::vec2(float(x), float(y)); // Direction in which neighbor cell lies
            glm::vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            glm::vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighbor’s Voronoi point
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

// copied from p6 of lecture note "Noise Functions"
float noise2D(glm::vec2 p)
{
    return glm::fract(sin(glm::dot(p, glm::vec2(127.1, 311.7))) *
                      43758.5453);
}

// copied from p15 of lecture note "Noise Functions"
float interpNoise2D(float x, float z)
{
    int intX = int(floor(x));
    float fractX = glm::fract(x);
    int intZ = int(floor(z));
    float fractZ = glm::fract(z);
    float v1 = noise2D(glm::vec2(intX, intZ));
    float v2 = noise2D(glm::vec2(intX + 1, intZ));
    float v3 = noise2D(glm::vec2(intX, intZ + 1));
    float v4 = noise2D(glm::vec2(intX + 1, intZ + 1));
    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractZ);
}

// copied from p15 of lecture note "Noise Functions"
float fbm2D(float x, float y)
{
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;
    float freq = 2.f;
    float amp = 0.5f;
    for(int i = 1; i <= octaves; i++)
    {
        total += interpNoise2D(x * freq, y * freq) * amp;
        freq *= 2.f;
        amp *= persistence;
    }
    return total;
}


// copied from p49 of lecture note "Noise Functions"
float surflet(glm::vec2 P, glm::vec2 gridPoint)
{
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);

    // Get the random vector for the grid point
    glm::vec2 gradient = glm::normalize(2.f * random2(gridPoint) - glm::vec2(1.f));

    // Get the vector from the grid point to P
    glm::vec2 diff = P - gridPoint;

    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);

    //std::cout << (height * tX * tY) << std::endl;
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float surflet3D(glm::vec3 P, glm::vec3 gridPoint)
{
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float distZ = abs(P.z - gridPoint.z);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    float tZ = 1 - 6 * pow(distZ, 5.f) + 15 * pow(distZ, 4.f) - 10 * pow(distZ, 3.f);

    // Get the random vector for the grid point
    glm::vec3 gradient = glm::normalize(2.f * random3(gridPoint) - glm::vec3(1.f));

    // Get the vector from the grid point to P
    glm::vec3 diff = P - gridPoint;

    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);

    //std::cout << (height * tX * tY) << std::endl;
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY * tZ;
}

// copied from p48 of lecture note "Noise Functions"
float perlinNoise(float x, float z)
{

    glm::vec2 uv = glm::vec2(x, z);
    float surfletSum = 0.f;

    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx)
    {
        for(int dy = 0; dy <= 1; ++dy)
        {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }
    return surfletSum;
}

float perlinNoise3D(float x, float y, float z)
{

    glm::vec3 uvw = glm::vec3(x, y, z);
    float surfletSum = 0.f;

    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx)
    {
        for(int dy = 0; dy <= 1; ++dy)
        {
            for(int dz = 0; dz <= 1; ++dz)
            {
                surfletSum += surflet3D(uvw, glm::floor(uvw) + glm::vec3(dx, dy, dz));
            }
      }
    }
    return surfletSum;
}


int Biome::getHeight(int x, int z)
{
    float grasslandHeight = getGrasslandHeight(x, z);
    float mountainHeight = getMountainHeight(x, z);

    float perlin = glm::smoothstep(0.2, 0.7, (double)(abs(perlinNoise(x/50.f, z/50.f))));

    return glm::mix(grasslandHeight, mountainHeight, perlin);
}

int Biome::getGrasslandHeight(int x, int z)
{
    int grassMin = 130;
    int grassMax = 150;

    int rawHeight = grassMin + 0.8 * (grassMax - grassMin) * abs(fbm2D(x/128.f, z/128.f)) + 3;

    if (rawHeight > grassMax)
    {
        return grassMax;
    }
    else
    {
        return rawHeight;
    }
}

int Biome::getMountainHeight(int x, int z)
{
    int mountainMin = 150;
    int mountainMax = 250;

    int rawHeight = mountainMin + 5 * (mountainMax - mountainMin) * abs(worleyNoise(x/64.f, z/64.f));

    if (rawHeight > mountainMax)
    {
        return mountainMax;
    }
    else
    {
        return rawHeight;
    }
}

bool Biome::isCave(int x, int y, int z)
{
    float noiseValue = perlinNoise3D(x/50.f, y/50.f, z/50.f);
    const float threshold = 0.f;

    //std::cout << "noise value: " << noiseValue << std::endl;
    if (noiseValue < threshold)
    {
        return true;
    }
    else
    {
        return false;
    }
}

Biome::~Biome()
{}
