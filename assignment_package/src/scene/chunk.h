#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include <unordered_set>

#define BLK_UVX * 0.0625
#define BLK_UVY * 0.0625
#define BLK_UV  0.0625

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, BEDROCK, LAVA
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

struct VertexPUData {
    glm::vec4 pos;
    glm::vec2 uv;
    VertexPUData(glm::vec4 m_pos,glm::vec2 m_uv)
        :pos(m_pos),uv(m_uv)
    {}
};

struct VertexData {
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec2 uv;
    VertexData(const VertexPUData& PU,const glm::vec4& m_nor,const glm::vec4& posoffset,const glm::vec2& UVoffset)
        :pos(PU.pos + posoffset),nor(m_nor),uv(PU.uv + UVoffset)
    {}
};

struct BlockFace
{
    Direction direction;
    glm::vec4 directionVec;
    std::array<VertexPUData, 4> vertices;
    BlockFace(Direction dir, glm::vec4 dirv,const VertexPUData &a,const VertexPUData &b, const VertexPUData &c,const VertexPUData &d)
        :direction(dir),directionVec(dirv),vertices{a,b,c,d}
    {}
};

const static std::array<BlockFace,6> adjacentFaces {
    BlockFace(XPOS,glm::vec4(1,0,0,0),VertexPUData(glm::vec4(1,0,1,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(1,0,0,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(1,1,0,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(1,1,1,1),glm::vec2(0,BLK_UV))),

    BlockFace(XNEG,glm::vec4(-1,0,0,0),VertexPUData(glm::vec4(0,0,0,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(0,0,1,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(0,1,1,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(0,1,0,1),glm::vec2(0,BLK_UV))),

    BlockFace(YPOS,glm::vec4(0,1,0,0),VertexPUData(glm::vec4(0,1,1,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(1,1,1,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(1,1,0,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(0,1,0,1),glm::vec2(0,BLK_UV))),

    BlockFace(YNEG,glm::vec4(0,-1,0,0),VertexPUData(glm::vec4(0,0,0,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(1,0,0,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(1,0,1,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(0,0,1,1),glm::vec2(0,BLK_UV))),

    BlockFace(ZPOS,glm::vec4(0,0,1,0),VertexPUData(glm::vec4(0,0,1,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(1,0,1,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(1,1,1,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(0,1,1,1),glm::vec2(0,BLK_UV))),

    BlockFace(ZNEG,glm::vec4(0,0,-1,0),VertexPUData(glm::vec4(1,0,0,1),glm::vec2(0,0)),
                                      VertexPUData(glm::vec4(0,0,0,1),glm::vec2(BLK_UV,0)),
                                      VertexPUData(glm::vec4(0,1,0,1),glm::vec2(BLK_UV,BLK_UV)),
                                      VertexPUData(glm::vec4(1,1,0,1),glm::vec2(0,BLK_UV)))
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};
const static std::unordered_set<BlockType,EnumHash> transparent_blocks{
    EMPTY,WATER,LAVA
};



const static std::unordered_map<BlockType,std::unordered_map<Direction,glm::vec2,EnumHash>,EnumHash> blockFaceUVs{
    {GRASS,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(3 BLK_UVX, 15 BLK_UVY)},
                                                            {XNEG,glm::vec2(3 BLK_UVX, 15 BLK_UVY)},
                                                            {YPOS,glm::vec2(8 BLK_UVX, 13 BLK_UVY)},
                                                            {YNEG,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {ZPOS,glm::vec2(3 BLK_UVX, 15 BLK_UVY)},
                                                            {ZNEG,glm::vec2(3 BLK_UVX, 15 BLK_UVY)}}},
    {DIRT,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {XNEG,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {YPOS,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {YNEG,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {ZPOS,glm::vec2(2 BLK_UVX, 15 BLK_UVY)},
                                                            {ZNEG,glm::vec2(2 BLK_UVX, 15 BLK_UVY)}}},
    {STONE,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(1 BLK_UVX, 15 BLK_UVY)},
                                                            {XNEG,glm::vec2(1 BLK_UVX, 15 BLK_UVY)},
                                                            {YPOS,glm::vec2(1 BLK_UVX, 15 BLK_UVY)},
                                                            {YNEG,glm::vec2(1 BLK_UVX, 15 BLK_UVY)},
                                                            {ZPOS,glm::vec2(1 BLK_UVX, 15 BLK_UVY)},
                                                            {ZNEG,glm::vec2(1 BLK_UVX, 15 BLK_UVY)}}},
    {WATER,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(13 BLK_UVX, 3 BLK_UVY)},
                                                            {XNEG,glm::vec2(13 BLK_UVX, 3 BLK_UVY)},
                                                            {YPOS,glm::vec2(13 BLK_UVX, 3 BLK_UVY)},
                                                            {YNEG,glm::vec2(13 BLK_UVX, 3 BLK_UVY)},
                                                            {ZPOS,glm::vec2(13 BLK_UVX, 3 BLK_UVY)},
                                                            {ZNEG,glm::vec2(13 BLK_UVX, 3 BLK_UVY)}}},
    {SNOW,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(2 BLK_UVX, 11 BLK_UVY)},
                                                            {XNEG,glm::vec2(2 BLK_UVX, 11 BLK_UVY)},
                                                            {YPOS,glm::vec2(2 BLK_UVX, 11 BLK_UVY)},
                                                            {YNEG,glm::vec2(2 BLK_UVX, 11 BLK_UVY)},
                                                            {ZPOS,glm::vec2(2 BLK_UVX, 11 BLK_UVY)},
                                                            {ZNEG,glm::vec2(2 BLK_UVX, 11 BLK_UVY)}}},
    {BEDROCK,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(1 BLK_UVX, 14 BLK_UVY)},
                                                            {XNEG,glm::vec2(1 BLK_UVX, 14 BLK_UVY)},
                                                            {YPOS,glm::vec2(1 BLK_UVX, 14 BLK_UVY)},
                                                            {YNEG,glm::vec2(1 BLK_UVX, 14 BLK_UVY)},
                                                            {ZPOS,glm::vec2(1 BLK_UVX, 14 BLK_UVY)},
                                                            {ZNEG,glm::vec2(1 BLK_UVX, 14 BLK_UVY)}}},
    {LAVA,std::unordered_map<Direction,glm::vec2,EnumHash>{{XPOS,glm::vec2(13 BLK_UVX, 1 BLK_UVY)},
                                                            {XNEG,glm::vec2(13 BLK_UVX, 1 BLK_UVY)},
                                                            {YPOS,glm::vec2(13 BLK_UVX, 1 BLK_UVY)},
                                                            {YNEG,glm::vec2(13 BLK_UVX, 1 BLK_UVY)},
                                                            {ZPOS,glm::vec2(13 BLK_UVX, 1 BLK_UVY)},
                                                            {ZNEG,glm::vec2(13 BLK_UVX, 1 BLK_UVY)}}}
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable
{
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    GLuint m_bufIdxOpaque; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint m_bufIdxTransp; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint m_bufSingleOpaque;
    GLuint m_bufSingleTransp;
    bool m_singleOpaqueGenerated;
    bool m_singleTranspGenerated;
    bool m_idxOpaqueGenerated;
    bool m_idxTranspGenerated;

public:
    const glm::ivec2 m_global_pos;
    int m_countOpaque;
    int m_countTransp;
    Chunk(OpenGLContext *context,glm::ivec2 global_pos);
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAtRTC(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    void createVBOdata() override;
    void generateIdxOpaque();
    void generateIdxTransp();
    void generateSingleOpaqueBuf();
    void generateSingleTranspBuf();
    bool bindSingleOpaqueBuf();
    bool bindSingleTranspBuf();
    bool bindIdxOpaque();
    bool bindIdxTransp();
    void createSingleOpaqueVBO(const std::vector<VertexData>& pnu_Buffer,const std::vector<GLuint>& idx_Buffer);
    void createSingleTranspVBO(const std::vector<VertexData>& pnu_Buffer,const std::vector<GLuint>& idx_Buffer);
    void clearSingleOpaqueBuf();
    void clearSingleTranspBuf();
    void clearIdxOpaqueBuf();
    void clearIdxTranspBuf();
    bool opaquevbogenerated() const;
    bool transpvbogenerated() const;
    void destroy();
    int elemCountOpaque();
    int elemCountTransp();
    virtual ~Chunk(){};
};


// Milestone 2 Multi-threadding
struct ChunkVBOData {
    Chunk* mp_chunk;
    std::vector<VertexData> m_vboDataOpaque, m_vboDataTransparent;
    std::vector<GLuint> m_idxDataOpaque, m_idxDataTransparent;

    ChunkVBOData(Chunk* c) : mp_chunk(c),
                             m_vboDataOpaque{}, m_vboDataTransparent{},
                             m_idxDataOpaque{}, m_idxDataTransparent{}
    {}
};
