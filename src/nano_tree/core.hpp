#pragma once

//! \file core.hpp
//! \brief Contains various common utilities.

#include <cmath>
#include <vector>

namespace nano_tree {

//! Value used for any template Dims parameter to determine the dimensions
//! compile time.
constexpr int kRuntimeDims = -1;

namespace internal {

//! \brief Restores the heap property of the range defined by \p first and \p
//! last, assuming only the top element could have broken it.
//! \details Worst case performs O(2 log n) comparisons and O(log n) copies.
//! Performance will be better in practice because it is possible to "early
//! out" as soon as the first node is encountered that adheres to the heap
//! property.
//! <p/>
//! A function for replacing the top of the heap is not available using the
//! C++ stl. It is possible to use something like std::make_heap() or
//! range.push_back() followed by a std::pop_heap() to get the desired effect.
//! However, these solutions are either forced to traverse the entire range or
//! introduce overhead, which seem to make them quite a bit slower.
//! <p/>
//! It should be possible to use this function in combination with any of the
//! C++ stl heap related functions, as the documention of those suggest that the
//! heap is implemented as a binary heap.
template <typename RandomAccessIterator, typename Compare>
inline void ReplaceFrontHeap(
    RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
  auto const size = last - first;

  if (size < 2) {
    return;
  }

  typename std::iterator_traits<RandomAccessIterator>::difference_type parent =
      0;
  auto e = std::move(first[parent]);
  auto const last_parent = (size - 2) / 2;
  while (parent < last_parent) {
    auto child = 2 * parent + 1;
    if (comp(first[child], first[child + 1])) {
      ++child;
    }
    if (!comp(e, first[child])) {
      first[parent] = std::move(e);
      return;
    } else {
      first[parent] = std::move(first[child]);
      parent = child;
    }
  }
  // Everything below (but for replaceing the last child) is the same as
  // inside the loop. The only difference is that for even sized vectors we
  // can't compare with the second child of the last parent.
  //
  // Assuming not doing the check in the loop is better?
  auto child = 2 * parent + 1;
  if ((size & 1) == 1 && comp(first[child], first[child + 1])) {
    ++child;
  }
  if (comp(e, first[child])) {
    first[parent] = std::move(first[child]);
    parent = child;
  }
  // Last child gets replaced.
  first[parent] = std::move(e);
}

//! Compile time dimension count handling.
template <int Dims_>
struct Dimensions {
  //! Returns the dimension index of the dim dimension from the back.
  inline static constexpr int Back(int dim) { return Dims_ - dim; }
  inline static constexpr int Dims(int) { return Dims_; }
};

//! Run time dimension count handling.
template <>
struct Dimensions<kRuntimeDims> {
  inline static constexpr int Back(int) { return kRuntimeDims; }
  inline static int Dims(int dims) { return dims; }
};

//! Simple memory buffer making deletions of recursive elements a bit easier.
template <typename T>
class ItemBuffer {
 public:
  ItemBuffer(std::size_t const size) { buffer_.reserve(size); }

  template <typename... Args>
  inline T* MakeItem(Args&&... args) {
    buffer_.emplace_back(std::forward<Args>(args)...);
    return &buffer_.back();
  }

 private:
  std::vector<T> buffer_;
};

//! Returns the maximum amount of leaves given \p num_points and \p
//! max_leaf_size.
inline std::size_t MaxLeavesFromPoints(
    std::size_t const num_points, std::size_t const max_leaf_size) {
  // Each increase of the leaf size by a factor of two reduces the tree height
  // by 1. Each reduction in tree height halves the amount of leaves.
  // Rounding up the number of leaves means that the last one is not fully
  // occupied.
  return std::ceil(
      num_points /
      std::pow(2.0, std::floor(std::log2(static_cast<double>(max_leaf_size)))));
}

//! Returns the maximum amount of nodes given \p num_points and \p
//! max_leaf_size.
inline std::size_t MaxNodesFromPoints(
    std::size_t const num_points, std::size_t const max_leaf_size = 1) {
  return MaxLeavesFromPoints(num_points, max_leaf_size) * 2 - 1;
}

}  // namespace internal

}  // namespace nano_tree
