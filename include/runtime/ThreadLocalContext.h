#pragma once

#include "Hashtables.h"
#include "Hyperloglog.h"
#include "Tuplebuffer.h"

#include <cstdint>
#include <queue>

namespace p2cllvm {
struct ThreadAggregationContext {
  static constexpr size_t localHtSize = 1024;  
  TupleBuffer tupleBuffer;
  HashTable ht{localHtSize};
  Sketch sketch;
  size_t inserted = 0; 

  void insertAgg(uint64_t hash, HashTableEntry* entry){
      if(inserted >= ht.getThreshold()){
          ht.flush();
          inserted = 0;
      }
      ht.insert(entry, hash);
      ++inserted;
  }

  void allocTupleBuffer(size_t tupleSize) {
    tupleBuffer = TupleBuffer(tupleSize);
  }

  TupleBuffer *getTupleBuffer() { return &tupleBuffer; }

  HashTable *getHashTable() { return &ht; }
};

struct ThreadJoinContext {
  TupleBuffer tupleBuffer;
  Sketch sketch;

  void allocTupleBuffer(size_t tupleSize) {
    tupleBuffer = TupleBuffer(tupleSize);
  }

  TupleBuffer *getTupleBuffer() { return &tupleBuffer; }

  Sketch *getSketch() { return &sketch; }
};

struct ThreadSortContext {
    TupleBuffer tb;
    size_t elems;

    TupleBuffer* getTupleBuffer() {return &tb;}
};
}; // namespace p2cllvm
