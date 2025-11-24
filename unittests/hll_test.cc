#include <gtest/gtest.h>
#include <string>

#include "internal/tpch.h"
#include "runtime/Murmur.h"

TEST(HLLTEST, HLL_VAL) {
  std::string dir{"../../data-generator/output"};
  InMemoryTPCH db{dir};
  Sketch sketch;
  for (size_t i = 0; i < db.part.tuple_count; ++i) {
    sketch.add(
        murmurHash(reinterpret_cast<const char *>(&db.part.p_partkey.data[i]),
                   sizeof(int32_t)));
  }
  EXPECT_GE(sketch.estimate(), db.part.tuple_count * 0.8);
}
