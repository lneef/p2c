#include "IR/Builder.h"
#include "IR/Pipeline.h"
#include "internal/Compiler.h"
#include "internal/QueryScheduler.h"
#include "internal/tpch.h"
#include "operators/Driver.h"
#include "operators/Iu.h"
#include "operators/Operator.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <memory>
#include <string>

namespace p2cllvm {

static void produce_impl(TPCH &db, std::unique_ptr<Operator> &op,
                         std::vector<IU *> &outputs,
                         std::vector<std::string> &names,
                         std::unique_ptr<Sink> &sink) {
  Query query(db);
  auto builder = Builder(query);
  sink->produce(op, outputs, names, builder);
  QueryCompiler compiler;
  compiler.createJIT();
  compiler.addQuery(std::move(builder.query), builder.query.context);
  compiler.addSymbols(builder.query.symbolManager);
  MultiThreadedScheduler scheduler{10000, db};
  for (const auto &pipeline : builder.query.pipelines) {
    scheduler.execPipeline(*pipeline, compiler);
#ifndef NDEBUG
    llvm::errs() << "executed: " << pipeline->name << "\n";
#endif
  }
}

void produce(std::unique_ptr<Operator> op, std::vector<IU *> outputs,
             std::vector<std::string> names, std::unique_ptr<Sink> sink) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  std::string path = std::getenv("tpchpath") ? std::getenv("tpchpath")
                                             : "../data-generator/output";
  TPCH db(path);
  uint32_t runs = std::getenv("runs") ? std::atoi(std::getenv("runs")) : 3;
   for (uint32_t run = 0; run < runs; ++run) {
    produce_impl(db, op, outputs, names, sink);
  }
}
} // namespace p2cllvm
