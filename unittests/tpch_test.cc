#include "internal/Tpch.h"

#include <gtest/gtest.h>

TEST(TPCH, PART_READ_CORRECTLY) {
  const std::string dir{"../../data-generator/output"};
  InMemoryTPCH db{dir};
  EXPECT_EQ(db.part.tuple_count, 200000);
  EXPECT_EQ(db.lineitem.tuple_count, 6001215);
  EXPECT_EQ(db.partsupp.tuple_count, 800000);
  EXPECT_EQ(db.customer.tuple_count, 150000);
  EXPECT_EQ(db.region.tuple_count, 5);
  EXPECT_EQ(db.orders.tuple_count, 1500000);
}
