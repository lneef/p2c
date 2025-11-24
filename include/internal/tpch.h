#pragma once

#include "IR/Builder.h"
#include "IR/ColTypes.h"
#include "IR/Defs.h"
#include "IR/Types.h"
#include "File.h"
#include "internal/basetypes.h"
#include <cstdint>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <string_view>

namespace p2cllvm {

struct InMemoryTPCH {

  std::string path;
  struct Part {
    ColumnMapping<int32_t> p_partkey;
    ColumnMapping<StringView> p_name;
    ColumnMapping<StringView> p_mfgr;
    ColumnMapping<StringView> p_brand;
    ColumnMapping<StringView> p_type;
    ColumnMapping<int32_t> p_size;
    ColumnMapping<StringView> p_container;
    ColumnMapping<double> p_retailprice;
    ColumnMapping<StringView> p_comment;
    uint64_t tuple_count{p_partkey.size};
    Part(const std::string_view path)
        : p_partkey(path, "p_partkey"), p_name(path, "p_name"),
          p_mfgr(path, "p_mfgr"), p_brand(path, "p_brand"),
          p_type(path, "p_type"), p_size(path, "p_size"),
          p_container(path, "p_container"),
          p_retailprice(path, "p_retailprice"), p_comment(path, "p_comment") {}
  } part{path + "/part"};
  struct Supplier {
    ColumnMapping<int32_t> s_suppkey;
    ColumnMapping<StringView> s_name;
    ColumnMapping<StringView> s_address;
    ColumnMapping<int32_t> s_nationkey;
    ColumnMapping<StringView> s_phone;
    ColumnMapping<double> s_acctbal;
    ColumnMapping<StringView> s_comment;
    uint64_t tuple_count{s_suppkey.size};
    Supplier(const std::string_view path)
        : s_suppkey(path, "s_suppkey"), s_name(path, "s_name"),
          s_address(path, "s_address"), s_nationkey(path, "s_nationkey"),
          s_phone(path, "s_phone"), s_acctbal(path, "s_acctbal"),
          s_comment(path, "s_comment") {}
  } supplier{path + "/supplier"};
  struct PartSupp {
    ColumnMapping<int32_t> ps_partkey;
    ColumnMapping<int32_t> ps_suppkey;
    ColumnMapping<int32_t> ps_availqty;
    ColumnMapping<double> ps_supplycost;
    ColumnMapping<StringView> ps_comment;
    uint64_t tuple_count{ps_partkey.size};
    PartSupp(const std::string_view path)
        : ps_partkey(path, "ps_partkey"), ps_suppkey(path, "ps_suppkey"),
          ps_availqty(path, "ps_availqty"),
          ps_supplycost(path, "ps_supplycost"), ps_comment(path, "ps_comment") {
    }
  } partsupp{path + "/partsupp"};
  struct Customer {
    ColumnMapping<int32_t> c_custkey;
    ColumnMapping<StringView> c_name;
    ColumnMapping<StringView> c_address;
    ColumnMapping<int32_t> c_nationkey;
    ColumnMapping<StringView> c_phone;
    ColumnMapping<double> c_acctbal;
    ColumnMapping<StringView> c_mktsegment;
    ColumnMapping<StringView> c_comment;
    uint64_t tuple_count{c_custkey.size};
    Customer(const std::string_view path)
        : c_custkey(path, "c_custkey"), c_name(path, "c_name"),
          c_address(path, "c_address"), c_nationkey(path, "c_nationkey"),
          c_phone(path, "c_phone"), c_acctbal(path, "c_acctbal"),
          c_mktsegment(path, "c_mktsegment"), c_comment(path, "c_comment") {}

  } customer{path + "/customer"};
  struct Orders {
    ColumnMapping<int64_t> o_orderkey;
    ColumnMapping<int32_t> o_custkey;
    ColumnMapping<char> o_orderstatus;
    ColumnMapping<double> o_totalprice;
    ColumnMapping<Date> o_orderdate;
    ColumnMapping<StringView> o_orderpriority;
    ColumnMapping<StringView> o_clerk;
    ColumnMapping<int32_t> o_shippriority;
    ColumnMapping<StringView> o_comment;
    uint64_t tuple_count{o_orderkey.size};
    Orders(const std::string_view path)
        : o_orderkey(path, "o_orderkey"), o_custkey(path, "o_custkey"),
          o_orderstatus(path, "o_orderstatus"),
          o_totalprice(path, "o_totalprice"), o_orderdate(path, "o_orderdate"),
          o_orderpriority(path, "o_orderpriority"), o_clerk(path, "o_clerk"),
          o_shippriority(path, "o_shippriority"), o_comment(path, "o_comment") {
    }
  } orders{path + "/orders"};
  struct LineItem {
    ColumnMapping<int64_t> l_orderkey;
    ColumnMapping<int32_t> l_partkey;
    ColumnMapping<int32_t> l_suppkey;
    ColumnMapping<int32_t> l_linenumber;
    ColumnMapping<double> l_quantity;
    ColumnMapping<double> l_extendedprice;
    ColumnMapping<double> l_discount;
    ColumnMapping<double> l_tax;
    ColumnMapping<char> l_returnflag;
    ColumnMapping<char> l_linestatus;
    ColumnMapping<Date> l_shipdate;
    ColumnMapping<Date> l_commitdate;
    ColumnMapping<Date> l_receiptdate;
    ColumnMapping<StringView> l_shipinstruct;
    ColumnMapping<StringView> l_shipmode;
    ColumnMapping<StringView> l_comment;
    uint64_t tuple_count{l_orderkey.size};
    LineItem(const std::string_view path)
        : l_orderkey(path, "l_orderkey"), l_partkey(path, "l_partkey"),
          l_suppkey(path, "l_suppkey"), l_linenumber(path, "l_linenumber"),
          l_quantity(path, "l_quantity"),
          l_extendedprice(path, "l_extendedprice"),
          l_discount(path, "l_discount"), l_tax(path, "l_tax"),
          l_returnflag(path, "l_returnflag"),
          l_linestatus(path, "l_linestatus"), l_shipdate(path, "l_shipdate"),
          l_commitdate(path, "l_commitdate"),
          l_receiptdate(path, "l_receiptdate"),
          l_shipinstruct(path, "l_shipinstruct"),
          l_shipmode(path, "l_shipmode"), l_comment(path, "l_comment") {}
  } lineitem{path + "/lineitem"};
  struct Nation {
    ColumnMapping<int32_t> n_nationkey;
    ColumnMapping<StringView> n_name;
    ColumnMapping<int32_t> n_regionkey;
    ColumnMapping<StringView> n_comment;
    uint64_t tuple_count{n_nationkey.size};
    Nation(const std::string_view path)
        : n_nationkey(path, "n_nationkey"), n_name(path, "n_name"),
          n_regionkey(path, "n_regionkey"), n_comment(path, "n_comment") {}
  } nation{path + "/nation"};
  struct Region {
    ColumnMapping<int32_t> r_regionkey;
    ColumnMapping<StringView> r_name;
    ColumnMapping<StringView> r_comment;
    uint64_t tuple_count{r_regionkey.size};
    Region(const std::string_view path)
        : r_regionkey(path, "r_regionkey"), r_name(path, "r_name"),
          r_comment(path, "r_comment") {}
  } region{path + "/region"};

  InMemoryTPCH(const std::string_view path) : path(path) {}

  static TypeRef<llvm::StructType>
  createSingleTable(Query &query, llvm::LLVMContext &context, size_t idx) {
    switch (idx) {
    case 0:
      return createTable<int32_t, StringView, StringView, StringView,
                         StringView, int32_t, StringView, double, StringView>(
          context, "part");
    case 1:
      return createTable<int32_t, StringView, StringView, int32_t, StringView,
                         double, StringView>(context, "supplier");
    case 2:
      return createTable<int32_t, int32_t, int32_t, double, StringView>(
          context, "partsupp");
    case 3:
      return createTable<int32_t, StringView, StringView, int32_t, StringView,
                         double, StringView, StringView>(context, "customer");
    case 4:
      return createTable<int64_t, int32_t, char, double, Date, StringView,
                         StringView, int32_t, StringView>(context, "orders");
    case 5:
      return createTable<int64_t, int32_t, int32_t, int32_t, double, double,
                         double, double, char, char, Date, Date, Date,
                         StringView, StringView, StringView>(context,
                                                             "lineitem");
    case 6:
      return createTable<int32_t, StringView, int32_t, StringView>(context,
                                                                   "nation");
    case 7:
      return createTable<int32_t, StringView, int32_t>(context, "region");
    default:
      throw std::runtime_error("Invalid table index");
    }
  }
};

struct TPCH {
  using Info = std::pair<uint32_t, TypeEnum>;
  InMemoryTPCH db;
  TPCH(const std::string_view path) : db(path) {}
  inline static std::unordered_map<
      std::string_view,
      std::pair<uint16_t, std::unordered_map<std::string_view, Info>>>
      tables_indices{{"part",
                      {0,
                       {{"p_partkey", {0, TypeEnum::Integer}},
                        {"p_name", {1, TypeEnum::String}},
                        {"p_mfgr", {2, TypeEnum::String}},
                        {"p_brand", {3, TypeEnum::String}},
                        {"p_type", {4, TypeEnum::String}},
                        {"p_size", {5, TypeEnum::Integer}},
                        {"p_container", {6, TypeEnum::String}},
                        {"p_retailprice", {7, TypeEnum::Double}},
                        {"p_comment", {8, TypeEnum::String}}}}},
                     {"supplier",
                      {1,
                       {{"s_suppkey", {0, TypeEnum::Integer}},
                        {"s_name", {1, TypeEnum::String}},
                        {"s_address", {2, TypeEnum::String}},
                        {"s_nationkey", {3, TypeEnum::Integer}},
                        {"s_phone", {4, TypeEnum::String}},
                        {"s_acctbal", {5, TypeEnum::Double}},
                        {"s_comment", {6, TypeEnum::String}}}}},
                     {"partsupp",
                      {2,
                       {{"ps_partkey", {0, TypeEnum::Integer}},
                        {"ps_suppkey", {1, TypeEnum::Integer}},
                        {"ps_availqty", {2, TypeEnum::Integer}},
                        {"ps_supplycost", {3, TypeEnum::Double}},
                        {"ps_comment", {4, TypeEnum::String}}}}},
                     {"customer",
                      {3,
                       {{"c_custkey", {0, TypeEnum::Integer}},
                        {"c_name", {1, TypeEnum::String}},
                        {"c_address", {2, TypeEnum::String}},
                        {"c_nationkey", {3, TypeEnum::Integer}},
                        {"c_phone", {4, TypeEnum::String}},
                        {"c_acctbal", {5, TypeEnum::Double}},
                        {"c_mktsegment", {6, TypeEnum::String}},
                        {"c_comment", {7, TypeEnum::String}}}}},
                     {"orders",
                      {4,
                       {{"o_orderkey", {0, TypeEnum::BigInt}},
                        {"o_custkey", {1, TypeEnum::Integer}},
                        {"o_orderstatus", {2, TypeEnum::String}},
                        {"o_totalprice", {3, TypeEnum::Double}},
                        {"o_orderdate", {4, TypeEnum::Date}},
                        {"o_orderpriority", {5, TypeEnum::String}},
                        {"o_clerk", {6, TypeEnum::String}},
                        {"o_shippriority", {7, TypeEnum::Integer}},
                        {"o_comment", {8, TypeEnum::String}}}}},
                     {"lineitem",
                      {5,
                       {{"l_orderkey", {0, TypeEnum::BigInt}},
                        {"l_partkey", {1, TypeEnum::Integer}},
                        {"l_suppkey", {2, TypeEnum::Integer}},
                        {"l_linenumber", {3, TypeEnum::Integer}},
                        {"l_quantity", {4, TypeEnum::Double}},
                        {"l_extendedprice", {5, TypeEnum::Double}},
                        {"l_discount", {6, TypeEnum::Double}},
                        {"l_tax", {7, TypeEnum::Double}},
                        {"l_returnflag", {8, TypeEnum::Char}},
                        {"l_linestatus", {9, TypeEnum::Char}},
                        {"l_shipdate", {10, TypeEnum::Date}},
                        {"l_commitdate", {11, TypeEnum::Date}},
                        {"l_receiptdate", {12, TypeEnum::Date}},
                        {"l_shipinstruct", {13, TypeEnum::String}},
                        {"l_shipmode", {14, TypeEnum::String}},
                        {"l_comment", {15, TypeEnum::String}}}}},
                     {"nation",
                      {6,
                       {{"n_nationkey", {0, TypeEnum::Integer}},
                        {"n_name", {1, TypeEnum::String}},
                        {"n_regionkey", {2, TypeEnum::Integer}},
                        {"n_comment", {3, TypeEnum::String}}}}},
                     {"region",
                      {7,
                       {{"r_regionkey", {0, TypeEnum::Integer}},
                        {"r_name", {1, TypeEnum::String}},
                        {"r_comment", {2, TypeEnum::String}}}}}};

  static ValueRef<> createColumnAccess(Builder &builder, llvm::Value *ptr,
                                         std::string_view rel,
                                         std::string_view col) {
    auto &[idx, map] = tables_indices[rel];
    auto &[idxt, type] = map[col];
    auto &context = builder.getContext();
    auto *structType =
        InMemoryTPCH::createSingleTable(builder.query, context, idx);
    return builder.builder.CreateStructGEP(structType, ptr, idxt);
  }

  int64_t estimate_scale_factor() {
    return lround(static_cast<double>(db.lineitem.tuple_count) /
                  (6e6 + 2e4)); // approx. scale factor
  }

  std::pair<void *, size_t> getTable(size_t idx) {
#define CASE(idx, table)                                                       \
  case idx:                                                                    \
    return {&db.table, db.table.tuple_count};

    switch (idx) {
      CASE(0, part)
      CASE(1, supplier)
      CASE(2, partsupp)
      CASE(3, customer)
      CASE(4, orders)
      CASE(5, lineitem)
      CASE(6, nation)
      CASE(7, region)
    default:
      throw std::runtime_error("Invalid table index");
    }
#undef CASE
  }
};

}; // namespace p2cllvm
