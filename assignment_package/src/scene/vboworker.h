#pragma once

#include <QRunnable>
#include <QMutex>
#include "chunk.h"
#include "terrain.h"
#include <unordered_set>

class VBOWorker : public QRunnable
{
protected:
    Chunk* mp_chunk;
    std::vector<ChunkVBOData>* mp_chunkVBOsCompleted;
    QMutex *mp_chunkVBOsCompletedLock;
public:
    VBOWorker(Chunk* c, std::vector<ChunkVBOData>* dat, QMutex *datLock);
    ~VBOWorker(){};
    void run() override;
};

