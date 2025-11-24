#pragma once


#include "IR/Tuple.h"
#include "runtime/Hashtables.h"
namespace p2cllvm{
    template<typename T = HashTableEntry>
    size_t calculateElemSize(Tuple& t){
        return sizeof(T) + t.getSize();
    }
};
