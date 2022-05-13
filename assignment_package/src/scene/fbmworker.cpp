#include "fbmworker.h"
#include "biome.h"

FBMWorker::FBMWorker(int x, int z, std::vector<Chunk*> chunksToFill, std::unordered_set<Chunk *> *chunksCompleted, QMutex* chunksCompletedLock)
    : m_xCorner(x), m_zCorner(z), m_chunksToFill(chunksToFill),
      mp_chunksCompleted(chunksCompleted), mp_chunksCompletedLock(chunksCompletedLock)
{}

void FBMWorker::run() {
    for(Chunk* c: m_chunksToFill) {
        // TODO: check_to_create_chunk() function in Terrain
        for(int block_posx = 0; block_posx < 16; ++block_posx) {
            for(int block_posz = 0; block_posz < 16; ++block_posz) {
                fillYSpace(c, block_posx, block_posz);
            }
        }
    }

    mp_chunksCompletedLock->lock();
    for (Chunk* c : m_chunksToFill) {
        // Add the Chunks to the list of Chunks that are ready
        // for VBO creation by the main thread
        mp_chunksCompleted->insert(c);
    }
    mp_chunksCompletedLock->unlock();
}

void FBMWorker::fillYSpace(Chunk *cur_chunk, int x, int z)
{
    int grassMin = 130;
    int grassMax = 150;
    int mountainMin = 150;
    int mountainMax = 250;
    int global_x = x + cur_chunk->m_global_pos.x;
    int global_z = z + cur_chunk->m_global_pos.y;

    int height = Biome::getHeight(global_x, global_z);

    // all blocks at Y = 0 should be made BEDROCK, and made to be unbreakable when left-clicked
    cur_chunk->setBlockAt(x, 0, z, BEDROCK);
    if (height < grassMax)
    {
        for (int y = 1; y < height; y++)
        {
            // fill the space from Y = 0 to Y = 128 entirely with STONE blocks
            if (y < 129)
            {

                if (y > 63 && Biome::isCave(global_x, y, global_z))
                {
                    //if (y < 25)
                    if (y < 85)
                    {
                        cur_chunk->setBlockAt(x, y, z, LAVA);
                        //std::cout << "grass lava: (" << x << ", " << y << ", " << z << ")" << std::endl;
                    }
                    else
                    {
                        cur_chunk->setBlockAt(x, y, z, EMPTY);
                        //std::cout << "grass cave: (" << x << ", " << y << ", " << z << ")" << std::endl;
                    }
                }
                else
                {

                    cur_chunk->setBlockAt(x, y, z, STONE);
                }
            }

            // when 128 < Y <= 255, the terrain should be filled with blocks of a type
            // dependent on the biome and up to a height dictated by the biome's height field.
            else
            {
                // In the grassland biome, each column should be filled with DIRT except for the very top block,
                // which should be GRASS
                if (y < height - 2)
                {
                    cur_chunk->setBlockAt(x, y, z, DIRT);
                }
                else
                {
                    cur_chunk->setBlockAt(x, y, z, GRASS);
                }
            }

        }
    }
    else
    {
        for (int y = 1; y < height; y++)
        {
            // In the mountain biome, you should fill each column with more STONE,
            // but if the column rises above Y = 200 then the very top block in that column should be SNOW
            if (y < 129)
            {

                if (y > 63 && Biome::isCave(global_x, y, global_z))
                {
                    //if (y < 25)
                    if (y < 85)
                    {
                        //std::cout << "mountain lava: (" << x << ", " << y << ", " << z << ")" << std::endl;
                        cur_chunk->setBlockAt(x, y, z, LAVA);
                    }
                    else
                    {
                        //std::cout << "mountain cave: (" << x << ", " << y << ", " << z << ")" << std::endl;
                        cur_chunk->setBlockAt(x, y, z, EMPTY);
                    }
                }
                else
                {

                    cur_chunk->setBlockAt(x, y, z, STONE);
                }
            }
            else if (y < height - 2)
            {
                cur_chunk->setBlockAt(x, y, z, STONE);
            }
            else
            {
                if (height > 200)
                {
                    cur_chunk->setBlockAt(x, y, z, SNOW);
                }
                else
                {
                    cur_chunk->setBlockAt(x, y, z, STONE);
                }

            }


        }
    }

    // In both biomes, any EMPTY blocks that fall between a height of 128 and 138 should be replaced with WATER.
    for (int y = 128; y < 139; y++)
    {
        if (cur_chunk->getBlockAt(x, y, z) == EMPTY)
        {
            cur_chunk->setBlockAt(x, y, z, WATER);
        }
    }

}
