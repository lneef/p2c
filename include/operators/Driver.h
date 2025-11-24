#pragma once
#include "IR/Builder.h"
#include "IR/Types.h"
#include "internal/BaseTypes.h"
#include "Operator.h"
#include "operators/OperatorContext.h"
#include "runtime/Test.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <llvm/Support/ErrorHandling.h>

namespace p2cllvm {
class Sink {
public:
  virtual ~Sink() = default;
  virtual void produce(std::unique_ptr<Operator> &parent,
                       std::span<IU *> required, std::span<std::string> names,
                       Builder &builder) = 0;
};
class PrintTupleSink : public Sink {
public:
  ~PrintTupleSink() override = default;

  void produce(std::unique_ptr<Operator> &parent, std::span<IU *> required,
               std::span<std::string> names, Builder &builder) override {
    IUSet requiredAsSet(std::vector<IU *>{required.begin(), required.end()});
    parent->produce(
        requiredAsSet, builder,
        [&](Builder &builder) {
          builder.createPrints({required.data(), required.size()});
        },
        [&](Builder &builder) {});
    builder.finishPipeline();
  }
};

void produce(std::unique_ptr<Operator> op, std::vector<IU *> outputs,
             std::vector<std::string> names, std::unique_ptr<Sink> sink);
}; // namespace p2cllvm
