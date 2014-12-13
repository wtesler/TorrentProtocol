#include "Chunk.h"

using namespace std;

#pragma once
class ChunkList : public vector<Chunk*> {
public:

    unsigned int size;

    Chunk* head;
    Chunk* tail;

    ChunkList::ChunkList(unsigned int size);

    ChunkList::~ChunkList(void);

    void ChunkList::destroy();

    void ChunkList::reprioritize(Chunk* Chunk);

    void ChunkList::excise(Chunk * Chunk);

    void ChunkList::print();

};

