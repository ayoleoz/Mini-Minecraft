#include "chunk.h"


Chunk::Chunk(OpenGLContext *context,glm::ivec2 global_pos) :  Drawable(context),m_countOpaque(-1),m_countTransp(-1),
    m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
    m_bufIdxOpaque(),m_bufIdxTransp(),m_bufSingleOpaque(),m_bufSingleTransp(),
    m_singleOpaqueGenerated(false),m_singleTranspGenerated(false),m_idxOpaqueGenerated(false),m_idxTranspGenerated(false),m_global_pos(global_pos)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    if((y > 255) | (y < 0)){
        return EMPTY;
    }
    if(x > 15){
        if(this->m_neighbors.at(XPOS) != nullptr)
        {
            return this->m_neighbors.at(XPOS)->getBlockAt(0,y,z);
        }
        return EMPTY;
    }
    if(x < 0){
        if(this->m_neighbors.at(XNEG) != nullptr)
        {
            return this->m_neighbors.at(XNEG)->getBlockAt(15,y,z);
        }
        return EMPTY;
    }
    if(z > 15){
        if(this->m_neighbors.at(ZPOS) != nullptr)
        {
            return this->m_neighbors.at(ZPOS)->getBlockAt(x,y,0);
        }
        return EMPTY;
    }
    if(z < 0){
        if(this->m_neighbors.at(ZNEG) != nullptr)
        {
            return this->m_neighbors.at(ZNEG)->getBlockAt(x,y,15);
        }
        return EMPTY;
    }
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}


// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

BlockType Chunk::getBlockAtRTC(int x, int y, int z) const{
    if(y < 0 || y >= 256) {
        return EMPTY;
    }
    if(x < 0){
        return this->m_neighbors.at(XNEG)->getBlockAt(15,y,z);
    }
    if(x > 15){
        return this->m_neighbors.at(XPOS)->getBlockAt(0,y,z);
    }
    if(z < 0){
        this->m_neighbors.at(ZNEG)->getBlockAt(x,y,15);
    }
    if(z >15){
        this->m_neighbors.at(ZPOS)->getBlockAt(x,y,0);
    }
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}


void Chunk::createVBOdata()
{}

void Chunk::generateSingleOpaqueBuf() {
    m_singleOpaqueGenerated = true;
    mp_context->glGenBuffers(1, &m_bufSingleOpaque);
}

void Chunk::generateSingleTranspBuf() {
    m_singleTranspGenerated = true;
    mp_context->glGenBuffers(1, &m_bufSingleTransp);
}

void Chunk::generateIdxOpaque(){
    m_idxOpaqueGenerated = true;
    mp_context->glGenBuffers(1, &m_bufIdxOpaque);
}

void Chunk::generateIdxTransp(){
    m_idxTranspGenerated = true;
    mp_context->glGenBuffers(1, &m_bufIdxTransp);
}

bool Chunk::bindSingleOpaqueBuf() {
    if(m_singleOpaqueGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufSingleOpaque);
    }
    return m_singleOpaqueGenerated;
}

bool Chunk::bindSingleTranspBuf() {
    if(m_singleTranspGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufSingleTransp);
    }
    return m_singleTranspGenerated;
}

bool Chunk::bindIdxOpaque() {
    if(m_idxOpaqueGenerated){
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    }
    return m_idxOpaqueGenerated;
}

bool Chunk::bindIdxTransp() {
    if(m_idxTranspGenerated){
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransp);
    }
    return m_idxTranspGenerated;
}

void Chunk::clearSingleOpaqueBuf() {
    if(m_singleOpaqueGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufSingleOpaque);
        m_singleOpaqueGenerated = false;
    }
}

void Chunk::clearSingleTranspBuf() {
    if(m_singleTranspGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufSingleTransp);
        m_singleTranspGenerated = false;
    }
}

void Chunk::clearIdxOpaqueBuf() {
    if(m_idxOpaqueGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufIdxOpaque);
        m_idxOpaqueGenerated = false;
        m_countOpaque = -1;
    }
}

void Chunk::clearIdxTranspBuf() {
    if(m_idxTranspGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufIdxTransp);
        m_idxTranspGenerated = false;
        m_countTransp = -1;
    }
}

void Chunk::createSingleOpaqueVBO(const std::vector<VertexData>& pnu_Buffer,const std::vector<GLuint>& idx_Buffer)
{
    m_countOpaque = idx_Buffer.size();
    generateIdxOpaque();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_Buffer.size() * sizeof(GLuint), idx_Buffer.data(), GL_STATIC_DRAW);

    generateSingleOpaqueBuf();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufSingleOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pnu_Buffer.size() * sizeof(VertexData), pnu_Buffer.data(), GL_STATIC_DRAW);
}

void Chunk::createSingleTranspVBO(const std::vector<VertexData>& pnu_Buffer,const std::vector<GLuint>& idx_Buffer)
{
    m_countTransp = idx_Buffer.size();
    generateIdxTransp();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransp);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_Buffer.size() * sizeof(GLuint), idx_Buffer.data(), GL_STATIC_DRAW);

    generateSingleTranspBuf();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufSingleTransp);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pnu_Buffer.size() * sizeof(VertexData), pnu_Buffer.data(), GL_STATIC_DRAW);
}

bool Chunk::opaquevbogenerated() const
{
    return m_singleOpaqueGenerated;
}

bool Chunk::transpvbogenerated() const
{
    return m_singleTranspGenerated;
}

void Chunk::destroy()
{
    clearSingleOpaqueBuf();
    clearSingleTranspBuf();
    clearIdxOpaqueBuf();
    clearIdxTranspBuf();
}

int Chunk::elemCountOpaque(){
    return m_countOpaque;
}

int Chunk::elemCountTransp(){
    return m_countTransp;
}
