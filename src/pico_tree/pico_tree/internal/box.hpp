#pragma once

#include "sequence.hpp"

namespace pico_tree {

namespace internal {

template <typename Box_>
struct BoxTraits;

template <typename Derived>
class BoxBase {
 public:
  using ScalarType = typename BoxTraits<Derived>::ScalarType;
  static constexpr int Dim = BoxTraits<Derived>::Dim;

  //! \brief Checks if \p x is contained. A point on the edge considered inside
  //! the box.
  inline bool Contains(ScalarType const* x) const {
    // We use derived().size() which includes the constexpr part. Otherwise a
    // small trait needs to be written.
    for (std::size_t i = 0; i < derived().size(); ++i) {
      if (min(i) > x[i] || max(i) < x[i]) {
        return false;
      }
    }
    return true;
  }

  template <typename OtherDerived>
  inline bool Contains(BoxBase<OtherDerived> const& x) const {
    return Contains(x.min()) && Contains(x.max());
  }

  inline void FillInverseMax() {
    for (std::size_t i = 0; i < derived().size(); ++i) {
      min(i) = std::numeric_limits<ScalarType>::max();
      max(i) = std::numeric_limits<ScalarType>::lowest();
    }
  }

  //! \brief See which axis of the box is the longest.
  inline void LongestAxis(int* p_max_index, ScalarType* p_max_value) const {
    *p_max_value = std::numeric_limits<ScalarType>::lowest();

    for (int i = 0; i < static_cast<int>(derived().size()); ++i) {
      ScalarType const delta = max(i) - min(i);
      if (delta > *p_max_value) {
        *p_max_index = i;
        *p_max_value = delta;
      }
    }
  }

  inline void Update(ScalarType const* x) {
    for (std::size_t i = 0; i < derived().size(); ++i) {
      if (x[i] < min(i)) {
        min(i) = x[i];
      }
      if (x[i] > max(i)) {
        max(i) = x[i];
      }
    }
  }

  template <typename OtherDerived>
  inline void Update(BoxBase<OtherDerived> const& x) {
    for (std::size_t i = 0; i < derived().size(); ++i) {
      if (x.min(i) < min(i)) {
        min(i) = x.min(i);
      }

      if (x.max(i) > max(i)) {
        max(i) = x.max(i);
      }
    }
  }

  //! Returns a const reference to the derived class.
  inline Derived const& derived() const {
    return *static_cast<Derived const*>(this);
  }
  //! Returns a reference to the derived class.
  inline Derived& derived() { return *static_cast<Derived*>(this); }

  inline ScalarType const* min() const noexcept { return derived().min(); }
  inline ScalarType* min() noexcept { return derived().min(); }
  inline ScalarType const* max() const noexcept { return derived().max(); }
  inline ScalarType* max() noexcept { return derived().max(); }
  inline ScalarType const& min(std::size_t i) const noexcept {
    return derived().min(i);
  }
  inline ScalarType& min(std::size_t i) noexcept { return derived().min(i); }
  inline ScalarType const& max(std::size_t i) const noexcept {
    return derived().max(i);
  }
  inline ScalarType& max(std::size_t i) noexcept { return derived().max(i); }
  inline std::size_t size() const noexcept { return derived().size(); }
};

//! \brief An axis aligned box represented by a min and max coordinate.
template <typename Scalar_, int Dim_>
class Box final : public BoxBase<Box<Scalar_, Dim_>> {
 public:
  using ScalarType = Scalar_;
  static int constexpr Dim = Dim_;

  inline explicit Box(std::size_t) {}

  inline ScalarType const* min() const noexcept { return min_.data(); }
  inline ScalarType* min() noexcept { return min_.data(); }
  inline ScalarType const* max() const noexcept { return max_.data(); }
  inline ScalarType* max() noexcept { return max_.data(); }
  inline ScalarType const& min(std::size_t i) const { return min_[i]; }
  inline ScalarType& min(std::size_t i) { return min_[i]; }
  inline ScalarType const& max(std::size_t i) const { return max_[i]; }
  inline ScalarType& max(std::size_t i) { return max_[i]; }
  inline std::size_t constexpr size() const noexcept { return min_.size(); }

 private:
  //! \brief Minimum box coordinate.
  std::array<Scalar_, Dim_> min_;
  //! \brief Maximum box coordinate.
  std::array<Scalar_, Dim_> max_;
};

// TODO Not using a vector, but having custom memory management would allow us
// to store max and avoid applying offsets.

//! \brief An axis aligned box represented by a min and max coordinate.
template <typename Scalar_>
class Box<Scalar_, kDynamicDim> final
    : public BoxBase<Box<Scalar_, kDynamicDim>> {
 public:
  using ScalarType = Scalar_;
  static int constexpr Dim = kDynamicDim;

  inline explicit Box(std::size_t size) : min_(size * 2), size_(size) {}

  inline ScalarType const* min() const noexcept { return min_.data(); }
  inline ScalarType* min() noexcept { return min_.data(); }
  inline ScalarType const* max() const noexcept { return min_.data() + size_; }
  inline ScalarType* max() noexcept { return min_.data() + size_; }
  inline ScalarType const& min(std::size_t i) const { return min_[i]; }
  inline ScalarType& min(std::size_t i) { return min_[i]; }
  inline ScalarType const& max(std::size_t i) const { return min_[i + size_]; }
  inline ScalarType& max(std::size_t i) { return min_[i + size_]; }
  inline std::size_t size() const noexcept { return size_; }

 private:
  //! \brief Minimum and maximum box coordinates aligned in memory.
  std::vector<ScalarType> min_;
  std::size_t size_;
};

template <typename Scalar_, int Dim_>
class BoxMap final : public BoxBase<BoxMap<Scalar_, Dim_>> {
 public:
  using ScalarType = Scalar_;
  static int constexpr Dim = Dim_;

  inline BoxMap(ScalarType* min, ScalarType* max, std::size_t)
      : min_(min), max_(max) {}

  inline ScalarType const* min() const noexcept { return min_; }
  inline ScalarType* min() noexcept { return min_; }
  inline ScalarType const* max() const noexcept { return max_; }
  inline ScalarType* max() noexcept { return max_; }
  inline ScalarType const& min(std::size_t i) const { return min_[i]; }
  inline ScalarType& min(std::size_t i) { return min_[i]; }
  inline ScalarType const& max(std::size_t i) const { return max_[i]; }
  inline ScalarType& max(std::size_t i) { return max_[i]; }
  inline std::size_t constexpr size() const noexcept {
    return static_cast<std::size_t>(Dim_);
  }

 private:
  ScalarType* min_;
  ScalarType* max_;
};

template <typename Scalar_>
class BoxMap<Scalar_, kDynamicDim> final
    : public BoxBase<BoxMap<Scalar_, kDynamicDim>> {
 public:
  using ScalarType = Scalar_;
  static int constexpr Dim = kDynamicDim;

  inline BoxMap(ScalarType* min, ScalarType* max, std::size_t size)
      : min_(min), max_(max), size_(size) {}

  inline ScalarType const* min() const noexcept { return min_; }
  inline ScalarType* min() noexcept { return min_; }
  inline ScalarType const* max() const noexcept { return max_; }
  inline ScalarType* max() noexcept { return max_; }
  inline ScalarType const& min(std::size_t i) const { return min_[i]; }
  inline ScalarType& min(std::size_t i) { return min_[i]; }
  inline ScalarType const& max(std::size_t i) const { return max_[i]; }
  inline ScalarType& max(std::size_t i) { return max_[i]; }
  inline std::size_t size() const noexcept { return size_; }

 private:
  ScalarType* min_;
  ScalarType* max_;
  std::size_t size_;
};

template <typename Scalar_, int Dim_>
struct BoxTraits<Box<Scalar_, Dim_>> {
  using ScalarType = typename std::remove_const<Scalar_>::type;
  static int constexpr Dim = Dim_;
};

template <typename Scalar_, int Dim_>
struct BoxTraits<BoxMap<Scalar_, Dim_>> {
  using ScalarType = typename std::remove_const<Scalar_>::type;
  static int constexpr Dim = Dim_;
};

}  // namespace internal

}  // namespace pico_tree
