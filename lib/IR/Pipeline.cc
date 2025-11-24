#include "IR/Pipeline.h"
#include "internal/tpch.h"
#include <cstdint>
namespace p2cllvm {
    uint64_t Query::getLineItemCount() const{
        return dbref.db.lineitem.tuple_count;
    }
}
