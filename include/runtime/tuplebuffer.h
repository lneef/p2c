#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

namespace p2cllvm {
class Builder;
struct Buffer {
    uint64_t ptr;
    uint64_t size;
    char *mem;

    Buffer(): mem(nullptr) {}

    Buffer(uint64_t size)
        : ptr(0), size(size),
          mem(reinterpret_cast<char *>(
              ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0))) {
        if (mem == MAP_FAILED) {
            std::cerr << "mmap failed" << std::endl;
        }
          }
    ~Buffer() {
      if (mem != nullptr)
        ::munmap(mem, size);
    }

    Buffer(Buffer &&other) : ptr(other.ptr), size(other.size), mem(other.mem) {
      other.ptr = 0;
      other.size = 0;
      other.mem = nullptr;
    }

    Buffer &operator=(Buffer &&other) {
      ptr = other.ptr;
      size = other.size;
      mem = other.mem;
      other.ptr = other.size = 0;
      other.mem = nullptr;
      return *this;
    }

    char* insertUnchecked(size_t elemSize);

    static llvm::StructType *createType(llvm::LLVMContext &context);
  };

class TupleBuffer {
public:
  TupleBuffer(uint64_t size = 64, size_t page_size = sysconf(_SC_PAGESIZE))
      : base(size * page_size) {}

  char*alloc(size_t elem_size) {
    if (buffers.empty() || buffers.back().size - buffers.back().ptr < elem_size){
      buffers.emplace_back(base *= 2);
    }
    auto &[ptr, size, mem] = buffers.back();
    auto *elem = mem + ptr;
    ptr += elem_size;
    return elem;
  }

  Buffer *getBuffers() { 
      return buffers.data(); }
  uint64_t getNumBuffers() {
      return buffers.size(); }

private:
  std::vector<Buffer> buffers;
  uint64_t base;
};
} // namespace p2cllvm
