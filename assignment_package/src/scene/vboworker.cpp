#include "vboworker.h"

VBOWorker::VBOWorker(Chunk *c, std::vector<ChunkVBOData> *dat, QMutex *datLock)
    : mp_chunk(c), mp_chunkVBOsCompleted(dat), mp_chunkVBOsCompletedLock(datLock)
{}

void VBOWorker::run() {
    ChunkVBOData c(mp_chunk);

    // TODO: createvbos() function in Terrain
    int idx_opaque = 0;
        int idx_transp = 0;
        for(int z = 0; z < 16; ++z)
            for(int y = 0; y < 256; ++y)
                for(int x = 0; x < 16; ++x)
                {
                    BlockType current = mp_chunk->getBlockAt(x,y,z);
                    if(current != EMPTY)
                    {
                        if(transparent_blocks.find(current) != transparent_blocks.end())
                        {
                        for(const BlockFace &neighbourFace : adjacentFaces){
                            glm::vec3 blk_pos = glm::vec3(neighbourFace.directionVec) + glm::vec3(x,y,z);
                            BlockType neighbourType = mp_chunk->getBlockAt(int(blk_pos.x),int(blk_pos.y),int(blk_pos.z));
                            if(transparent_blocks.find(neighbourType) != transparent_blocks.end()){

                                if(neighbourType != current){
                                    for(const VertexPUData &dat : neighbourFace.vertices)
                                    {
                                        c.m_vboDataTransparent.push_back(VertexData(dat,neighbourFace.directionVec,glm::vec4(x,y,z,0),blockFaceUVs.find(current)->second.find(neighbourFace.direction)->second));
                                    }
                                    c.m_idxDataTransparent.push_back(idx_transp);
                                    c.m_idxDataTransparent.push_back(idx_transp + 1);
                                    c.m_idxDataTransparent.push_back(idx_transp + 2);
                                    c.m_idxDataTransparent.push_back(idx_transp);
                                    c.m_idxDataTransparent.push_back(idx_transp + 2);
                                    c.m_idxDataTransparent.push_back(idx_transp + 3);
                                    idx_transp = idx_transp + 4;
                                }
                            }
                        }
                        }
                        else{
                            for(const BlockFace &neighbourFace : adjacentFaces){
                                glm::vec3 blk_pos = glm::vec3(neighbourFace.directionVec) + glm::vec3(x,y,z);
                                BlockType neighbourType = mp_chunk->getBlockAt(int(blk_pos.x),int(blk_pos.y),int(blk_pos.z));
                                if(transparent_blocks.find(neighbourType) != transparent_blocks.end()){
                                    for(const VertexPUData &dat : neighbourFace.vertices)
                                    {
                                        c.m_vboDataOpaque.push_back(VertexData(dat,neighbourFace.directionVec,glm::vec4(x,y,z,0),blockFaceUVs.find(current)->second.find(neighbourFace.direction)->second));
                                    }
                                    c.m_idxDataOpaque.push_back(idx_opaque);
                                    c.m_idxDataOpaque.push_back(idx_opaque + 1);
                                    c.m_idxDataOpaque.push_back(idx_opaque + 2);
                                    c.m_idxDataOpaque.push_back(idx_opaque);
                                    c.m_idxDataOpaque.push_back(idx_opaque + 2);
                                    c.m_idxDataOpaque.push_back(idx_opaque + 3);
                                    idx_opaque = idx_opaque + 4;
                                    }
                            }
                        }
                    }
                }
    mp_chunkVBOsCompletedLock->lock();
    mp_chunkVBOsCompleted->push_back(c);
    mp_chunkVBOsCompletedLock->unlock();
}
