#include <vector>
#include <algorithm>
#include "ChunkList.h"
#include <array>
#include <iostream>

using namespace std;

ChunkList::ChunkList(unsigned int size) : vector<Chunk*>(size) {
    
    this->size = size;

    Chunk* ChunkBehind = nullptr;
    for (unsigned int i = 0; i < size; i++) {
        Chunk * Chunk = new Chunk();
        if (ChunkBehind) {
            ChunkBehind->ahead = Chunk;
            Chunk->behind = ChunkBehind;
        }
        this->operator[](i) = Chunk;
        ChunkBehind = Chunk;
    }
    head = this->operator[](0);
    tail = this->operator[](size-1);
}


ChunkList::~ChunkList(void){
}

void ChunkList::destroy() {
    for (unsigned int i = 0; i < this->size; i++) {
        delete this->operator[](i);
    }
}


void ChunkList::reprioritize(Chunk* Chunk) {

    excise(Chunk);

    Chunk* iter = head;
    int i = 0;
    while (iter != nullptr, i++) {
        if (Chunk->checksum > iter->checksum) {
            iter->behind->ahead = Chunk;
            Chunk->behind = iter -> behind;
            Chunk->ahead = iter;
            iter-> behind = Chunk;
            break;
        }
        iter = iter->ahead;
    }

    if (i == 0) {
        head->behind = Chunk;
        Chunk->ahead = head;
        Chunk->behind = nullptr;
        head = Chunk;
    }

    if (i == size) {
        tail->ahead = Chunk;
        Chunk->behind = tail;
        Chunk->ahead = nullptr;
        tail = Chunk;
    }
}


void ChunkList::excise(Chunk * Chunk) {
    if (Chunk->behind) {
        Chunk->behind->ahead = Chunk->ahead;
    } else {
        head = Chunk->ahead;
    }
    if (Chunk->ahead) {
        Chunk->ahead->behind = Chunk->behind;
    } else {
        tail = Chunk->behind;
    }
}

void ChunkList::print() {
    Chunk* Chunk = head;
    while (Chunk != nullptr) {
        cout << Chunk->checksum << endl;
        Chunk = Chunk->ahead;
    }
}
