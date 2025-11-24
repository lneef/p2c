#pragma once

#include "IR/Builder.h"
#include "IR/ColTypes.h"
#include "IR/Defs.h"
#include "IR/Types.h"
#include "internal/tpch.h"
#include "Operator.h"

#include <cassert>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

namespace p2cllvm {
class Scan : public Operator {
public:
  void produce(IUSet &required, Builder &builder, ConsumerFn consumer,
               InitFn fn) override {
    auto [table_idx, colmap] = TPCH::tables_indices[table_name];
    auto &context = builder.getContext();
    TypeRef<llvm::StructType> table = InMemoryTPCH::createSingleTable(
        builder.query, builder.getContext(), table_idx);
    builder.createScanPipeline(table_idx);
    auto &scope = builder.getCurrentScope();
    ValueRef<llvm::Function> fun = scope.pipeline;
    assert(builder.builder.GetInsertBlock());
    ValueRef<> tableptr = fun->getArg(0);
    ValueRef<> begin = fun->getArg(1);
    ValueRef<> end = fun->getArg(2);
    fn(builder);
    std::vector<ValueRef<>> cols;
    cols.reserve(attributes.size());
    for (auto &col : required) {
      auto &[idx, type] = colmap[col->name];
      ValueRef<> colptr = builder.builder.CreateStructGEP(table, tableptr, idx);
      cols.push_back(builder.createColumnPtrLoad(
          colptr, createColumnType(context, col->type.typeEnum), col->type));
      scope.updatePtr(col, cols.back());
    }
    ValueRef<llvm::PHINode> iterphi = builder.createBeginIndexIter(begin, end);
    size_t i = 0;
    for (auto &col : required) {
      builder.createColumnAccess(iterphi, cols[i++], col);
    }
    consumer(builder);
    builder.createEndIndexIter();
  }

  Scan(std::string_view table_name) : table_name(table_name) {
    auto it = TPCH::tables_indices.find(table_name);
    attributes.reserve(it->second.second.size());
    for (auto &[name, type] : it->second.second) {
      attributes.emplace_back(std::string(name), type.second);
    }
  }

  IU *getIU(std::string_view name) {
    for (auto &attr : attributes) {
      if (attr.name == name) {
        return &attr;
      }
    }
    return nullptr;
  }

  IUSet availableIUs() override {
    IUSet iuSet;
    for (auto &attr : attributes) {
      iuSet.add(&attr);
    }
    return iuSet;
  }

private:
  std::string_view table_name;
  std::vector<IU> attributes;
};

}; // namespace p2cllvm
