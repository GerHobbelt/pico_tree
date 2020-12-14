#include <Eigen/Dense>
// This example compiles with C++11.
// Using C++11 and higher don't need the StdVector include (as mentioned inside
// the include itself).
//#include <Eigen/StdVector>
// If we use C++17 there is no need to take care of memory alignment:
// https://eigen.tuxfamily.org/dox-devel/group__TopicUnalignedArrayAssert.html
#include <pico_tree/eigen.hpp>
#include <pico_tree/kd_tree.hpp>
#include <random>
#include <scoped_timer.hpp>

// Important! This is not a performance benchmark. So don't take the "elapsed
// time" numbers too seriously.

using Index = int;
// Certain fixed size matrices require us to use aligned memory.
// https://eigen.tuxfamily.org/dox-devel/group__TopicFixedSizeVectorizable.html
using PointCm = Eigen::Vector3f;
using PointRm = Eigen::RowVector3f;
// Eigen::Vector4f requires aligned memory.
using Point4f = Eigen::Vector4f;

template <typename Point>
using PointsMapCm = Eigen::Map<
    Eigen::Matrix<
        typename Point::Scalar,
        Point::RowsAtCompileTime,
        Eigen::Dynamic>,
    Eigen::AlignedMax>;
// The alignment used by Eigen equals Eigen::AlignedMax. Note that Eigen can
// look at the data pointer to know if it is properly aligned.

template <typename Point>
using PointsMapRm = Eigen::Map<
    Eigen::Matrix<
        typename Point::Scalar,
        Eigen::Dynamic,
        Point::ColsAtCompileTime,
        Eigen::RowMajor>,
    Eigen::AlignedMax>;

template <typename Point>
using EigenAdaptorCm = pico_tree::EigenAdaptor<Index, PointsMapCm<Point>>;

template <typename Point>
using EigenAdaptorRm = pico_tree::EigenAdaptor<Index, PointsMapRm<Point>>;

std::size_t const kRunCount = 1024 * 1024;
int const kNumPoints = 1024 * 1024 * 2;
double const kArea = 1000.0;
Index const kMaxLeafCount = 16;

template <typename Point>
std::vector<Point, Eigen::aligned_allocator<Point>> GenerateRandomEigenN(
    int n, typename Point::Scalar size) {
  std::vector<Point, Eigen::aligned_allocator<Point>> random(n);
  for (auto& p : random) {
    p = Point::Random() * size / typename Point::Scalar(2.0);
  }

  return random;
}

void ColMajor() {
  using Point = PointCm;
  using Scalar = typename Point::Scalar;
  constexpr int Dim = Point::RowsAtCompileTime;
  using PointsMap = PointsMapCm<Point>;
  using Adaptor = EigenAdaptorCm<Point>;

  auto points = GenerateRandomEigenN<Point>(kNumPoints, kArea);
  Point p = Point::Random() * kArea / Scalar(2.0);

  std::cout << "Eigen RowMajor: " << Adaptor::RowMajor << std::endl;

  {
    pico_tree::KdTree<Index, Scalar, Dim, Adaptor> tree(
        Adaptor(PointsMap(points.data()->data(), Dim, points.size())),
        kMaxLeafCount);

    std::vector<std::pair<Index, Scalar>> knn;
    ScopedTimer t("tree nn_ pico_tree deflt l2", kRunCount);
    for (std::size_t i = 0; i < kRunCount; ++i) {
      tree.SearchKnn(p, 1, &knn);
    }
  }
}

void RowMajor() {
  using Point = PointRm;
  using Scalar = typename Point::Scalar;
  constexpr int Dim = Point::ColsAtCompileTime;
  using PointsMap = PointsMapRm<Point>;
  using Adaptor = EigenAdaptorRm<Point>;

  auto points = GenerateRandomEigenN<Point>(kNumPoints, kArea);
  Point p = Point::Random() * kArea / Scalar(2.0);

  std::cout << "Eigen RowMajor: " << Adaptor::RowMajor << std::endl;

  {
    pico_tree::KdTree<Index, Scalar, Dim, Adaptor> tree(
        Adaptor(PointsMap(points.data()->data(), points.size(), Dim)),
        kMaxLeafCount);

    std::vector<std::pair<Index, Scalar>> knn;
    ScopedTimer t("tree nn_ pico_tree deflt l2", kRunCount);
    for (std::size_t i = 0; i < kRunCount; ++i) {
      tree.SearchKnn(p, 1, &knn);
    }
  }
}

// The Metrics demo shows how it can be beneficial to use the metrics supplied
// by the <pico_tree/eigen.hpp> header.
//
// Suppose we want to use a KdTree with a spatial dimension of 3 using floats as
// a scalar. In this case we can use Eigen::Vector3f as the data type. However,
// Eigen::Vector3f doesn't benefit from vectorization. With Eigen::Vector4f we
// can, but in this case we have one dimension too many!
//
// Luckily we can use a different dimension for the points and the KdTree, but
// some care needs to be taken:
// * The default Metrics don't make explicit use of vectorization but the Eigen
// based Metrics may do so.
// * The extra coordinate of Eigen::Vector4f must be set 0 so it doesn't
// influence any distance calculations. E.g., the squared distance uses a dot
// product.
//
// See also:
// http://eigen.tuxfamily.org/index.php?title=UsingVector4fForVector3fOperations
void Metrics() {
  using Point = Point4f;
  using Scalar = typename Point::Scalar;
  // Tell the KdTree we use a spatial dimension of 3 instead of 4.
  constexpr int Dim = Point::RowsAtCompileTime - 1;
  using PointsMap = PointsMapCm<Point>;
  using Adaptor = EigenAdaptorCm<Point>;

  auto points = GenerateRandomEigenN<Point>(kNumPoints, kArea);
  // The Eigen::Map uses the dimension of 4.
  PointsMap map(points.data()->data(), Point::RowsAtCompileTime, points.size());
  // Set the last row (4th coordinate) to 0.
  map.bottomRows<1>().setZero();

  Adaptor adaptor(map);
  Point p = Point::Random() * kArea / Scalar(2.0);
  // Again, set the last row (4th coordinate) to 0.
  p.w() = Scalar(0.0);

  std::cout << "Eigen Metrics: " << std::endl;

  {
    pico_tree::KdTree<
        Index,
        Point::Scalar,
        Dim,
        Adaptor,
        pico_tree::EigenMetricL2<Scalar>>
        tree(adaptor, kMaxLeafCount);

    std::vector<std::pair<Index, Scalar>> knn;
    ScopedTimer t("tree nn_ pico_tree eigen l2", kRunCount);
    for (std::size_t i = 0; i < kRunCount; ++i) {
      tree.SearchKnn(p, 1, &knn);
    }
  }

  {
    pico_tree::KdTree<
        Index,
        Point::Scalar,
        Dim,
        Adaptor,
        pico_tree::EigenMetricL1<Scalar>>
        tree(adaptor, kMaxLeafCount);

    std::vector<std::pair<Index, Scalar>> knn;
    ScopedTimer t("tree nn_ pico_tree eigen l1", kRunCount);
    for (std::size_t i = 0; i < kRunCount; ++i) {
      tree.SearchKnn(p, 1, &knn);
    }
  }
}

int main() {
  RowMajor();
  ColMajor();
  Metrics();
  return 0;
}
