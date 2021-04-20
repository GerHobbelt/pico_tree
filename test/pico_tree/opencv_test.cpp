#include <gtest/gtest.h>

#include <pico_tree/kd_tree.hpp>
#include <pico_tree/opencv.hpp>

#include "common.hpp"

TEST(OpenCvTest, Interface) {
  using Scalar = float;
  int constexpr Dim = 3;
  using Traits = pico_tree::CvTraits<Scalar, Dim>;

  cv::Mat matrix(8, 3, cv::DataType<Scalar>::type);
  CheckTraits<Traits, Dim, int>(matrix, matrix.cols, matrix.rows);
}

TEST(OpenCvTest, TreeCompatibility) {
  using Scalar = float;
  cv::Mat random(8, 4, cv::DataType<Scalar>::type);
  cv::randu(random, -Scalar(1.0), Scalar(1.0));

  pico_tree::KdTree<pico_tree::CvTraits<Scalar, 4>> tree(random, 10);

  TestKnn(tree, 2);
}
