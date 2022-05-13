#include "terrain.h"
#include "cube.h"
#include "biome.h"
#include <stdexcept>
#include <iostream>
#define CHUNK_LOADING_RADIUS 6

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {

}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        return EMPTY;
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context,glm::ivec2(x,z));
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }

    /**
    // DTY Milestone 1:
    for (int i = x; i < x + 16; i++)
    {
        for (int j = z; j < z + 16; j++)
        {
            fillYSpace(i, j);
        }
    }
    // DTY Milestone 1
    **/

    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::createvbos(int minX, int maxX, int minZ, int maxZ) {
    for(int x = minX; x < maxX; x += 16) {
            for(int z = minZ; z < maxZ; z += 16) {
                if(hasChunkAt(x,z))
                {  
                    const uPtr<Chunk> &chunk = getChunkAt(x, z);
                    if((!chunk->transpvbogenerated()) | (!chunk->opaquevbogenerated()))
                    {
                        std::vector<VertexData> pnu_Buffer_opaque_temp;
                        std::vector<VertexData> pnu_Buffer_transp_temp;
                        std::vector<GLuint> idx_Buffer_opaque_temp;
                        std::vector<GLuint> idx_Buffer_transp_temp;
                        int xFloor = static_cast<int>(glm::floor(x / 16.f)) * 16;
                        int zFloor = static_cast<int>(glm::floor(z / 16.f)) * 16;
                        createChunkData(chunk.get(),pnu_Buffer_opaque_temp,pnu_Buffer_transp_temp,
                                        idx_Buffer_opaque_temp,idx_Buffer_transp_temp,xFloor,zFloor);
                        chunk->createSingleOpaqueVBO(pnu_Buffer_opaque_temp,idx_Buffer_opaque_temp);
                        chunk->createSingleTranspVBO(pnu_Buffer_transp_temp,idx_Buffer_transp_temp);
                    }
                }
            }
    }
}

void Terrain::createChunkData(Chunk *cur_Chunk, std::vector<VertexData> &pnu_Buffer_opaque, std::vector<VertexData> &pnu_Buffer_transp,
                              std::vector<GLuint> &idx_Buffer_opaque, std::vector<GLuint> &idx_Buffer_transp, const int &m_x, const int &m_z)
{
    int idx_opaque = 0;
    int idx_transp = 0;
    for(int z = 0; z < 16; ++z)
        for(int y = 0; y < 256; ++y)
            for(int x = 0; x < 16; ++x)
            {
                BlockType current = cur_Chunk->getBlockAt(x,y,z);
                if(current != EMPTY)
                {
                    if(transparent_blocks.find(current) != transparent_blocks.end())
                    {
                    for(const BlockFace &neighbourFace : adjacentFaces){
                        BlockType neighbourType = getBlockAt(glm::vec3(neighbourFace.directionVec) + glm::vec3(x,y,z) + glm::vec3(m_x,0,m_z));
                        if(transparent_blocks.find(neighbourType) != transparent_blocks.end()){

                            if(neighbourType != current){
                                for(const VertexPUData &dat : neighbourFace.vertices)
                                {
                                    pnu_Buffer_transp.push_back(VertexData(dat,neighbourFace.directionVec,glm::vec4(x,y,z,0),blockFaceUVs.find(current)->second.find(neighbourFace.direction)->second));
                                }
                                idx_Buffer_transp.push_back(idx_transp);
                                idx_Buffer_transp.push_back(idx_transp + 1);
                                idx_Buffer_transp.push_back(idx_transp + 2);
                                idx_Buffer_transp.push_back(idx_transp);
                                idx_Buffer_transp.push_back(idx_transp + 2);
                                idx_Buffer_transp.push_back(idx_transp + 3);
                                idx_transp = idx_transp + 4;
                            }
                        }
                    }
                    }
                    else{
                        for(const BlockFace &neighbourFace : adjacentFaces){
                            BlockType neighbourType = getBlockAt(glm::vec3(neighbourFace.directionVec) + glm::vec3(x,y,z) + glm::vec3(m_x,0,m_z));
                            if(transparent_blocks.find(neighbourType) != transparent_blocks.end()){
                                for(const VertexPUData &dat : neighbourFace.vertices)
                                {
                                    pnu_Buffer_opaque.push_back(VertexData(dat,neighbourFace.directionVec,glm::vec4(x,y,z,0),blockFaceUVs.find(current)->second.find(neighbourFace.direction)->second));
                                }
                                idx_Buffer_opaque.push_back(idx_opaque);
                                idx_Buffer_opaque.push_back(idx_opaque + 1);
                                idx_Buffer_opaque.push_back(idx_opaque + 2);
                                idx_Buffer_opaque.push_back(idx_opaque);
                                idx_Buffer_opaque.push_back(idx_opaque + 2);
                                idx_Buffer_opaque.push_back(idx_opaque + 3);
                                idx_opaque = idx_opaque + 4;
                                }
                        }
                    }
                }
            }
}

void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram)
{
    for(int x = minX; x < maxX; x += 16) {
            for(int z = minZ; z < maxZ; z += 16) {
                if(hasChunkAt(x,z))
                {
                    const uPtr<Chunk> &chunk = getChunkAt(x, z);
                    if((chunk->transpvbogenerated()) & (chunk->opaquevbogenerated()))
                    {
                        int xFloor = static_cast<int>(glm::floor(x / 16.f)) * 16;
                        int zFloor = static_cast<int>(glm::floor(z / 16.f)) * 16;
                        shaderProgram->setModelMatrix(glm::translate(glm::mat4(),glm::vec3(xFloor,0,zFloor)));
                        shaderProgram->drawChunkOpaque(*chunk);
                    }
                }
            }
    }
    for(int x = minX; x < maxX; x += 16) {
            for(int z = minZ; z < maxZ; z += 16) {
                if(hasChunkAt(x,z))
                {
                    const uPtr<Chunk> &chunk = getChunkAt(x, z);
                    if((chunk->transpvbogenerated()) & (chunk->opaquevbogenerated()))
                    {
                        int xFloor = static_cast<int>(glm::floor(x / 16.f)) * 16;
                        int zFloor = static_cast<int>(glm::floor(z / 16.f)) * 16;
                        shaderProgram->setModelMatrix(glm::translate(glm::mat4(),glm::vec3(xFloor,0,zFloor)));
                        shaderProgram->drawChunkTransp(*chunk);
                    }
                }
            }
    }
}

void Terrain::check_to_create_chunk(float x, float z)
{
    int xFloor = static_cast<int>(glm::floor(x / 16.f)) * 16;
    int zFloor = static_cast<int>(glm::floor(z / 16.f)) * 16;
    for(int chunk_posx = -1 * CHUNK_LOADING_RADIUS;chunk_posx < CHUNK_LOADING_RADIUS + 1;chunk_posx++)
    {
        for(int chunk_posz = -1 * CHUNK_LOADING_RADIUS;chunk_posz < CHUNK_LOADING_RADIUS + 1;chunk_posz++)
        {
            if(!hasChunkAt(xFloor + 16 * chunk_posx, zFloor + 16 * chunk_posz))
            {
                instantiateChunkAt(xFloor + 16 * chunk_posx, zFloor + 16 * chunk_posz);
                create_chunk_terrian(xFloor + 16 * chunk_posx, zFloor + 16 * chunk_posz);
            }
        }
    }
}

void Terrain::create_chunk_terrian(int m_x, int m_z)
{
    for(int block_posx = 0; block_posx < 16; ++block_posx) {
        for(int block_posz = 0; block_posz < 16; ++block_posz) {
            fillYSpace(m_x + block_posx, m_z + block_posz);
        }
    }
}

void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setBlockAt(x, 128, z, STONE);
            }
            else {
                setBlockAt(x, 128, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setBlockAt(32, y, 32, GRASS);
    }
}


void Terrain::fillYSpace(int x, int z)
{
    int grassMin = 130;
    int grassMax = 150;
    int mountainMin = 150;
    int mountainMax = 250;

    int height = Biome::getHeight(x, z);

    // all blocks at Y = 0 should be made BEDROCK, and made to be unbreakable when left-clicked
    setBlockAt(x, 0, z, BEDROCK);
    if (height < grassMax)
    {
        for (int y = 1; y < height; y++)
        {
            // fill the space from Y = 0 to Y = 128 entirely with STONE blocks
            if (y < 129)
            {

                if (y > 63 && Biome::isCave(x, y, z))
                {
                    //if (y < 25)
                    if (y < 85)
                    {
                        setBlockAt(x, y, z, LAVA);
                        //std::cout << "grass lava: (" << x << ", " << y << ", " << z << ")" << std::endl;
                    }
                    else
                    {
                        setBlockAt(x, y, z, EMPTY);
                        //std::cout << "grass cave: (" << x << ", " << y << ", " << z << ")" << std::endl;
                    }
                }
                else
                {

                    setBlockAt(x, y, z, STONE);
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
                    setBlockAt(x, y, z, DIRT);
                }
                else
                {
                    setBlockAt(x, y, z, GRASS);
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

                if (y > 63 && Biome::isCave(x, y, z))
                {
                    //if (y < 25)
                    if (y < 85)
                    {
                        //std::cout << "mountain lava: (" << x << ", " << y << ", " << z << ")" << std::endl;
                        setBlockAt(x, y, z, LAVA);
                    }
                    else
                    {
                        //std::cout << "mountain cave: (" << x << ", " << y << ", " << z << ")" << std::endl;
                        setBlockAt(x, y, z, EMPTY);
                    }
                }
                else
                {

                    setBlockAt(x, y, z, STONE);
                }
            }
            else if (y < height - 2)
            {
                setBlockAt(x, y, z, STONE);
            }
            else
            {
                if (height > 200)
                {
                    setBlockAt(x, y, z, SNOW);
                }
                else
                {
                    setBlockAt(x, y, z, STONE);
                }

            }


        }
    }

    // In both biomes, any EMPTY blocks that fall between a height of 128 and 138 should be replaced with WATER.
    for (int y = 128; y < 139; y++)
    {
        if (getBlockAt(x, y, z) == EMPTY)
        {
            setBlockAt(x, y, z, WATER);
        }
    }

}


bool Terrain::terrainZoneExists(int x, int z) const {
    int64_t key = toKey(x, z);
    return terrainZoneExists(key);
}


bool Terrain::terrainZoneExists(int64_t id) const {
    return this->m_generatedTerrain.contains(id);
}


QSet<int64_t> Terrain::terrainZonesBorderingZone(glm::ivec2 zoneCoords, unsigned int radius, bool onlyCircumference) const {
    int radiusInZoneScale = static_cast<int>(radius) * 64;
    QSet<int64_t> result;
    // Only want to look at terrain zones exactly at our radius
    if (onlyCircumference) {
        for (int i = -radiusInZoneScale; i <= radiusInZoneScale; i += 64) {
            // Nx1 to the right
            result.insert(toKey(zoneCoords.x + radiusInZoneScale, zoneCoords.y + i));
            // Nx1 to the left
            result.insert(toKey(zoneCoords.x -radiusInZoneScale, zoneCoords.y + i));
            // Nx1 above
            result.insert(toKey(zoneCoords.x + i, zoneCoords.y + radiusInZoneScale));
            // Nx1 below
            result.insert(toKey(zoneCoords.x + i, zoneCoords.y - radiusInZoneScale));
        }
    }
    // Want to look at all terrain zones encompassed by the radius
    else {
        for (int i = -radiusInZoneScale; i <= radiusInZoneScale; i += 64) {
            for (int j = -radiusInZoneScale; j <= radiusInZoneScale; j += 64) {
                result.insert(toKey(zoneCoords.x + i, zoneCoords.y + j));
            }
        }
    }
    return result;
}


void Terrain::tryExpansion(glm::vec3 playerPos, glm::vec3 playerPosPrev) {
    // Find the player's position relative
    // to their current terrain gen zone
    glm::ivec2 currZone(64.f * glm::floor(playerPos.x / 64.f), 64.f * glm::floor(playerPos.z / 64.f));
    glm::ivec2 prevZone(64.f * glm::floor(playerPosPrev.x / 64.f), 64.f * glm::floor(playerPosPrev.z / 64.f));
    // Determine which terrain zones border our current position and our previous position
    // This *will* include un-generated terrain zones, so we can compare them to our global set
    // and know to generate them
    // TERRAIN_CREATE_RADIUS = 4
    QSet<int64_t> terrainZonesBorderingCurrPos = terrainZonesBorderingZone(currZone, 4, false);
    QSet<int64_t> terrainZonesBorderingPrevPos = terrainZonesBorderingZone(prevZone, 4, false);
    // Check which terrain zones need to be destroy()ed
    // by determining which terrain zones were previously in our radius and are now not
    for (auto id : terrainZonesBorderingPrevPos) {
        if (!terrainZonesBorderingCurrPos.contains(id)) {
            glm::ivec2 coord = toCoords(id);
            for (int x = coord.x; x < coord.x + 64; x += 16) {
                for (int z = coord.y; z < coord.y + 64; z += 16) {
                    auto &chunk = getChunkAt(x, z);
                    chunk->destroy();
                }
            }
        }
    }

    // Determine if any terrain zones around our current position need VBO data.
    // Send these to VBOWorkers.
    // DO NOT send zones to workers if they do not exist in our global map.
    // Instead, send these to FBMWorkers.
    for (auto id : terrainZonesBorderingCurrPos) {
        // If it exists already AND IS NOT IN PREV SET, send it to a VBOWorker
        // If it's in the prev set, then it's already been sent to a VBOWorker
        // at some point, and may even already have VBOs
        if (terrainZoneExists(id)) {
            if (!terrainZonesBorderingPrevPos.contains(id)) {
                            glm::ivec2 coord = toCoords(id);
                            for (int x = coord.x; x < coord.x + 64; x += 16) {
                                for (int z = coord.y; z < coord.y + 64; z += 16) {
                                    auto &chunk = getChunkAt(x, z);
                                    spawnVBOWorker(chunk.get());
                                }
                            }
                        }
        }
        // If it does not yet exist, send it to an FBMWorker
        // This also adds it to the set of generated terrain zones
        // so we don't try to repeatedly generate it.
        else {
            spawnFBMWorker(id);
        }
    }
}


void Terrain::spawnFBMWorker(int64_t zoneToGenerate) {
    m_generatedTerrain.insert(zoneToGenerate);
    std::vector<Chunk*> chunksForWorker;
    glm::ivec2 coords = toCoords(zoneToGenerate);
    for (int x = coords.x; x < coords.x + 64; x += 16) {
        for (int z = coords.y; z < coords.y + 64; z += 16) {
            Chunk* c = instantiateChunkAt(x, z);
            c->m_countOpaque = 0; // Allow it to be "drawn" even with no VBO data
            c->m_countTransp = 0; // Allow it to be "drawn" even with no VBO data
            chunksForWorker.push_back(c);
        }
    }
    FBMWorker *worker = new FBMWorker(coords.x, coords.y, chunksForWorker,
                                      &m_chunksThatHaveBlockData, &m_chunksThatHaveBlockDataLock);
    QThreadPool::globalInstance()->start(worker);
}


void Terrain::spawnFBMWorkers(const QSet<int64_t> &zonesToGenerate) {
    // Spawn worker threads to generate more Chunks
    for (int64_t zone : zonesToGenerate) {
        spawnFBMWorker(zone);
    }
}


void Terrain::spawnVBOWorker(Chunk* chunkNeedingVBOData) {
    VBOWorker *worker = new VBOWorker(chunkNeedingVBOData, &m_chunksThatHaveVBOs, &m_chunksThatHaveVBOsLock);
    QThreadPool::globalInstance()->start(worker);
}


void Terrain::spawnVBOWorkers(const std::unordered_set<Chunk*> &chunksNeedingVBOs) {
    for (Chunk* c : chunksNeedingVBOs) {
        spawnVBOWorker(c);
    }
}


void Terrain::checkThreadResults() {
    // Send Chunks that have been processed by FBMWorkers
    // to VBOWorkers for VBO data
    m_chunksThatHaveBlockDataLock.lock();
    spawnVBOWorkers(m_chunksThatHaveBlockData);
    m_chunksThatHaveBlockData.clear();
    m_chunksThatHaveBlockDataLock.unlock();

    // Collect the Chunks that have been given VBO data
    // by VBOWorkers and send that VBO data to the GPU.
    m_chunksThatHaveVBOsLock.lock();
    for (ChunkVBOData cd : m_chunksThatHaveVBOs) {
        cd.mp_chunk->createSingleOpaqueVBO(cd.m_vboDataOpaque, cd.m_idxDataOpaque);
        cd.mp_chunk->createSingleTranspVBO(cd.m_vboDataTransparent, cd.m_idxDataTransparent);
    }
    m_chunksThatHaveVBOs.clear();
    m_chunksThatHaveVBOsLock.unlock();
}
