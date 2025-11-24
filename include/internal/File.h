#pragma once
#include <cstdint>
#include <fcntl.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

#include "IR/Defs.h"
#include "IR/Types.h"
#include "basetypes.h"

namespace p2cllvm {
template<typename T> struct ColumnBase{
    static constexpr bool fixed_size = !std::is_same_v<T, StringView>;
};
template <typename T> struct ColumnVec : ColumnBase<T> {
  using vec = T *;
};

template <> struct ColumnVec<StringView> : ColumnBase<StringView> {
  using vec = String *;
};

template <typename T> struct ColumnMapping {

  using mapping_type = typename ColumnVec<T>::vec;

  ColumnMapping(const std::string_view dirPath, const std::string_view colName){
      std::tie(data, size) = load_columns(dirPath, colName);
  }

  static std::pair<mapping_type, size_t> load_columns(std::string_view dirPath,
                            std::string_view colName) {
    std::string filePath = std::string(dirPath) + "/" + std::string(colName) + ".bin";
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1)
      throw std::runtime_error("Failed to open file: " + filePath);

    /// Find the size of the file
    lseek(fd, 0, SEEK_END);
    size_t size = lseek(fd, 0, SEEK_CUR);
    auto *data = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    if(data == MAP_FAILED){
        auto err = errno;
        close(fd);
        throw std::logic_error("Could not map file: " +
                                std::string(strerror(err)));
    }
    close(fd);
    if (size > 1024 * 1024) {
      madvise(data, size, MADV_HUGEPAGE);
    }

    /// If type is fixed size is the number of elements
    if constexpr (ColumnVec<T>::fixed_size) {
        size /= sizeof(T);
    }
    return {reinterpret_cast<mapping_type>(data), size};
  }

  static TypeRef<llvm::StructType> createType(llvm::LLVMContext &context, llvm::Type *ptr) {
    auto name = "ColumnMapping";
    return getOrCreateType(context, name, [&]() {
      return llvm::StructType::create(
          context,
          {PointerTy::createType(context), BigIntTy::createType(context)},
          name);
    });
  }

  ~ColumnMapping() {
    if (data) {
      if constexpr (ColumnVec<T>::fixed_size) {
        ::munmap(data, size * sizeof(T));
      } else {
        ::munmap(data, size);
      }
    }
  }

  mapping_type data;
  uint64_t size;
};
}; // namespace p2cllvm
