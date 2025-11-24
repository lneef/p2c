#include "internal/basetypes.h"
#include "runtime/Murmur.h"
#include "runtime/Runtime.h"
#include "runtime/ThreadLocalContext.h"
#include "runtime/hashtables.h"
#include "runtime/hyperloglog.h"
#include "runtime/tuplebuffer.h"
#include "stdarg.h"
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <nmmintrin.h>
#include <string_view>


uint64_t hash(char *x, size_t len) { return murmurHash(x, len); }

char *hashtable_insert(HashTable *ht, HashTableEntry *entry, uint64_t hash) {
  entry->hash = hash;
  return ht->insert(entry);
}

char *hashtable_insert_tagged(HashTable *ht, HashTableEntry *entry,
                              uint64_t hash) {
  entry->hash = hash;
  return ht->insertWithTag(entry, hash);
}

HashTableEntry *hashtable_lookup(HashTable *ht, uint64_t hash) {
  return ht->lookup(hash);
}

void hashtable_alloc(HashTable *ht, uint64_t size) { *ht = HashTable(size); }

void hll_add(Sketch *sketch, uint64_t hash) { sketch->add(hash); }

uint64_t hll_estimate(Sketch *sketch) { return sketch->estimate(); }

char *tb_insert(TupleBuffer *tb, size_t elem_size) {
  return reinterpret_cast<char *>(tb->alloc(elem_size));
}

Buffer *getBuffers(TupleBuffer *tb) { return tb->getBuffers(); }

uint64_t getNumBuffers(TupleBuffer *tb) { return tb->getNumBuffers(); }

bool like_prefix(StringView *sv, char *prefix, size_t len) {
  if (len > sv->length) {
    return false;
  };
  return strncmp(sv->data, prefix, len) == 0;
}

bool like_suffix(StringView *sv, char *prefix, size_t len) {
  return std::string_view(sv->data, sv->length)
      .ends_with(std::string_view(prefix, len));
}

bool like(StringView *sv, char *pattern, size_t len) {
  return std::string_view(sv->data, sv->length).find(pattern, 0, len - 1) !=
         std::string_view::npos;
}

bool string_eq(const StringView *s1, const StringView *s2) {
  if (s1->length != s2->length) {
    return false;
  }
  return strncmp(s1->data, s2->data, s1->length) == 0;
}

bool string_lt(StringView *s1, StringView *s2) {
  return std::string_view(s1->data, s1->length) <
         std::string_view(s2->data, s2->length);
}

bool string_gt(StringView *s1, StringView *s2) {
  return std::string_view(s1->data, s2->length) >
         std::string_view(s2->data, s2->length);
}

void load_from_slotted_page(uint64_t idx, ColumnMapping<StringView> *data,
                            StringView *sv) {
  auto &slot = data->data->slot[idx];
  sv->data = reinterpret_cast<char *>(data->data) + slot.offset;
  sv->length = slot.length;
}

int32_t load_from_page(uint64_t idx, ColumnMapping<int32_t> *data) {
  return data->data[idx];
}

#define PRINTER(fmt, x) printf(fmt, x)

void printChar(char x) { PRINTER("%c  ", x); }

void printBool(bool x) { PRINTER("%d  ", x); }

void printDate(int32_t x) {
  unsigned year, month, day;
  Date::fromInt(std::bit_cast<unsigned>(x), year, month, day);
  printf("%u - %u - %u  ", year, month, day);
}

void printDouble(double x) { PRINTER("%.4f  ", x); }

void printStringView(StringView *sv) {
  for (auto i = 0; i < sv->length; ++i) {
    printf("%c", sv->data[i]);
  }
  printf("%s", "  ");
}

void printBigInt(int64_t x) { PRINTER("%ld  ", x); }

void printInteger(int32_t x) { PRINTER("%d  ", x); }

void printNewline() { printf("\n"); }

#undef PRINTER

void printDebug(void *ptr) { printf("%p\n", ptr); }

/// Join
char *insertJoinEntry(ThreadJoinContext *ctx, uint64_t hash, size_t elem_size) {
  ctx->sketch.add(hash);
  return ctx->tupleBuffer.alloc(elem_size);
}

template <typename F>
static inline void insert(ThreadJoinContext &ctx, HashTable *ht,
                          size_t elem_size, F &&fn) {
  size_t num = ctx.tupleBuffer.getNumBuffers();
  auto *buffers = ctx.tupleBuffer.getBuffers();
  for (auto i = 0ull; i < num; ++i) {
    auto &[ptr, size, mem] = buffers[i];
    for (auto j = 0ull; j < ptr; j += elem_size) {
      auto *htEntry = reinterpret_cast<HashTableEntry *>(mem + j);
      fn(htEntry, htEntry->hash);
    }
  }
}

void insertAll(ThreadLocalStorage<ThreadJoinContext> *ctx, HashTable *ht,
               size_t elem_size) {
  auto [ref, size] = ctx->getElems();
  for (auto i = 0u; i < size; ++i) {
    insert(static_cast<ThreadJoinContext &>(ref[i]), ht, elem_size,
           [&](HashTableEntry *htEntry, uint64_t hash) {
             ht->insertWithTag(htEntry, hash);
           });
  }
}

void insertMultithreaded(ThreadJoinContext *ctx, HashTable *ht, size_t elem_size) {
  size_t num = ctx->tupleBuffer.getNumBuffers();
  auto *buffers = ctx->tupleBuffer.getBuffers();
  for (auto i = 0ull; i < num; ++i) {
    auto &[ptr, size, mem] = buffers[i];
    for (auto j = 0ull; j < ptr; j += elem_size) {
      auto *htEntry = reinterpret_cast<HashTableEntry *>(mem + j);
      ht->insertWithTagThreaded(htEntry, htEntry->hash);
    }
  }    
}

/// Aggregation
void insertAggEntry(ThreadAggregationContext *ctx, uint64_t hash,
                    HashTableEntry *entry) {
  assert(ctx->ht.getThreshold() ==
         (ThreadAggregationContext::localHtSize * 10) / 7);
  ctx->sketch.add(hash);
  ctx->ht.insert(entry, hash);
}

HashTable *getLocalHashTable(ThreadAggregationContext *ctx) { return &ctx->ht; }

/// Sort
void allocSortBuffer(SortBuffer *sb, size_t size) {
  assert(size > 0);
  *sb = SortBuffer(size);
}

size_t combineSizes(ThreadLocalStorage<ThreadSortContext> *ctx, size_t tsize) {
  auto [elems, len] = ctx->getElems();
  size_t combined = 0;
  for (size_t i = 0; i < len; ++i)
    combined += elems[i].elems;
  return combined * tsize;
}

char *insertSortEntry(ThreadSortContext *lctx, size_t size) {
  lctx->elems++;
  return lctx->tb.alloc(size);
}

void insertSortBuffer(ThreadLocalStorage<ThreadSortContext> *ctx,
                      SortBuffer *sb, size_t size) {
  auto [elems, n] = ctx->getElems();
  for (size_t i = 0; i < n; ++i) {
    auto &tb = elems[i].tb;
    auto numBuffers = tb.getNumBuffers();
    auto *buffers = tb.getBuffers();
    for (size_t j = 0; j < numBuffers; ++j) {
      auto &[ptr, _, mem] = buffers[j];
      for (size_t k = 0; k < ptr; k += size) {
        char *elem = sb->insertUnchecked(size);
        std::memcpy(elem, mem + k, size);
      }
    }
  }
}

void sortBuffer(SortBuffer *buf, size_t size,
                int (*cmp)(const void *, const void *)) {
  std::qsort(buf->mem, buf->ptr / size, size, cmp);
}

char *getSorted(SortBuffer *sb) { return sb->mem; }

void *signExtend(void *ptr) {
  /// adapted from https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend  
  constexpr unsigned signOff = 48;
  auto intptr = reinterpret_cast<intptr_t>(ptr);
  const intptr_t m = 1ull << (signOff - 1);
  intptr = intptr & ((1ull << signOff) - 1);
  intptr = (intptr ^ m) - m;
  return reinterpret_cast<void *>(intptr);
}

bool cmpTag(void *ptr, uint64_t hash) {
  auto tag = reinterpret_cast<intptr_t>(ptr) >> 48;
  return (tag & (hash >> 48)) != (hash >> 48);
}
