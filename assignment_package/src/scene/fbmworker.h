#pragma once

#include <QRunnable>
#include <QMutex>
#include "chunk.h"
#include "terrain.h"
#include <unordered_set>

class FBMWorker : public QRunnable {
private:
    int m_xCorner, m_zCorner;
    std::vector<Chunk*> m_chunksToFill;
    QMutex *mp_chunksCompletedLock;
    std::unordered_set<Chunk*>* mp_chunksCompleted;
public:
    FBMWorker(int x, int z, std::vector<Chunk*> chunksToFill,
                  std::unordered_set<Chunk*>* chunksCompleted, QMutex* chunksCompletedLock);
    ~FBMWorker(){};
    void fillYSpace(Chunk* cur_chunk,int x, int z);
    void run() override;
};
