#ifndef BIOME_H
#define BIOME_H

#include <glm_includes.h>

class Biome
{
public:
    Biome();

    static int getHeight(int x, int z);
    static int getGrasslandHeight(int x, int z);
    static int getMountainHeight(int x, int z);

    static bool isCave(int x, int y, int z);

    ~Biome();
};

#endif // BIOME_H
