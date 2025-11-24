#pragma once
#include "IR/Pipeline.h"
#include "internal/Compiler.h"
#include "internal/tpch.h"
#include <cstdint>
#include <llvm/Support/Error.h>
#include <tuple>
#include <vector>

namespace p2cllvm {
template <typename I> class QueryScheduler {
public:
  virtual ~QueryScheduler() = default;
  QueryScheduler(TPCH &db) : db(db) {}

  void execPipeline(Pipeline &pipeline, QueryCompiler &qc) {
    switch (pipeline.type) {
    case PipelineType::Default:
      static_cast<I &>(*this).execPipelineImpl(pipeline, qc);
      break;
    case PipelineType::Scan:
      static_cast<I &>(*this).execScanPipelineImpl(
          static_cast<ScanPipeline &>(pipeline), qc);
      break;
    case PipelineType::Continuation:
      static_cast<I &>(*this).execContinuationPipelineImpl(pipeline, qc);
      break;
    }
  }

protected:
  TPCH &db;
};

class SimpleQueryScheduler : public QueryScheduler<SimpleQueryScheduler> {
public:
  SimpleQueryScheduler(TPCH &db) : QueryScheduler(db) {};
  ~SimpleQueryScheduler() = default;

  void execPipelineImpl(Pipeline &pipeline, QueryCompiler &qc) {
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto *fptr = (*fn).toPtr<void (*)(void **)>();
    fptr(pipeline.args.data());
  }

  void execScanPipelineImpl(ScanPipeline &pipeline, QueryCompiler &qc) {
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto tableIdx = pipeline.tableIndex;
    auto [ptr, size] = db.getTable(tableIdx);
    auto *fptr =
        (*fn).toPtr<void (*)(void *, uint64_t, uint64_t, uint64_t, void **)>();
    fptr(ptr, 0, size, 0, pipeline.args.data());
  }

  void execContinuationPipelineImpl(Pipeline &pipeline, QueryCompiler &qc) {
    return execPipelineImpl(pipeline, qc);
  }
};

class MultiThreadedScheduler : public QueryScheduler<MultiThreadedScheduler> {
public:
  MultiThreadedScheduler(size_t chunkSize, TPCH &db,
                         size_t nthreads = std::thread::hardware_concurrency())
      : QueryScheduler(db), nthreads(nthreads), chunkSize(chunkSize) {}
  ~MultiThreadedScheduler() = default;

  void execPipelineImpl(Pipeline &pipeline, QueryCompiler &qc) {
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto *fptr = (*fn).toPtr<void (*)(void **)>();
    fptr(pipeline.args.data());
  }
  void execScanPipelineImpl(ScanPipeline &pipeline, QueryCompiler &qc) {
    std::vector<std::thread> threads(nthreads);
    auto tableIdx = pipeline.tableIndex;
    auto [ptr, size] = db.getTable(tableIdx);
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto *fptr =
        (*fn).toPtr<void (*)(void *, uint64_t, uint64_t, uint64_t, void **)>();
    std::atomic<size_t> chunk = 0;
    for (auto &t : threads) {
      t = std::thread([&]() {
        size_t start;
        while ((start = chunk.fetch_add(chunkSize)) < size) {
          fptr(ptr, start, std::min(start + chunkSize, size), nthreads,
               pipeline.args.data());
        }
      });
    }
    for (auto &t : threads)
      t.join();
  }

  void execContinuationPipelineImpl(Pipeline& pipeline, QueryCompiler& qc){
    std::vector<std::thread> threads(nthreads);
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto *fptr =
        (*fn).toPtr<void (*)(void **)>();
    for(auto& t: threads)
        t = std::thread(fptr, pipeline.args.data());
    
    for(auto& t: threads)
        t.join();
  }
private:
  size_t nthreads;
  size_t chunkSize;
};

class CompilationTimeScheduler : public QueryScheduler<CompilationTimeScheduler>{
public:
 CompilationTimeScheduler(TPCH &db) : QueryScheduler(db) {};
  ~CompilationTimeScheduler() = default;

  void execPipelineImpl(Pipeline &pipeline, QueryCompiler &qc) {
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    std::ignore = (*fn).toPtr<void (*)(void **)>();
  }

  void execScanPipelineImpl(ScanPipeline &pipeline, QueryCompiler &qc) {
    auto fn = qc.getPipelineFunction(pipeline.name);
    if (!fn)
      llvm::report_fatal_error(fn.takeError());
    auto tableIdx = pipeline.tableIndex;
    std::ignore = db.getTable(tableIdx);
    std::ignore =
        (*fn).toPtr<void (*)(void *, uint64_t, uint64_t, uint64_t, void **)>();
  }

  void execContinuationPipelineImpl(Pipeline &pipeline, QueryCompiler &qc) {
    return execPipelineImpl(pipeline, qc);
  }

};
} // namespace p2cllvm
