# PicoTree

[![build-and-test](https://github.com/Jaybro/pico_tree/workflows/build-and-test/badge.svg)](https://github.com/Jaybro/pico_tree/actions?query=workflow%3Abuild-and-test) [![pip](https://github.com/Jaybro/pico_tree/workflows/pip/badge.svg)](https://github.com/Jaybro/pico_tree/actions?query=workflow%3Apip)

PicoTree is a C++ header only library with [Python bindings](https://github.com/pybind/pybind11) for nearest neighbor searches and range searches using a KdTree.

See the table below to get an impression of the performance provided by the [KdTree](https://en.wikipedia.org/wiki/K-d_tree) of this library versus several other implementations:

|                                     | Build C++ | Build Python  | Knn C++    | Knn Python  |
| ----------------------------------- | --------- | ------------- | ---------- | ----------- |
| [nanoflann][nano] v1.3.2            | 1.5s      | ...           | 3.2s       | ...         |
| [SciPy KDTree][spkd] v1.6.3         | ...       | 5.0s          | ...        | 547.2s      |
| [Scikit-learn KDTree][skkd] 0.22.2  | ...       | 12.2s         | ...        | 44.5s       |
| [OpenCV FLANN][cvfn] 4.5.2          | 1.9s      | ...           | 4.7s       | ...         |
| PicoTree KdTree v0.7.3              | 0.9s      | 1.1s          | 2.8s       | 3.3s        |

It compares the performance of the build and query algorithms using two [LiDAR](./docs/benchmark.md) based point clouds of sizes 7733372 and 7200863. The first is used to compare build times and both are used to compare query times. All benchmarks were generated with the following parameters: `max_leaf_size=10`, `knn=1` and `OMP_NUM_THREADS=1`. Note that a different C++ back-end was used for each of the `Knn C++` and `Knn Python` benchmarks. This means that they shouldn't be compared directly. See the [Python comparison](./examples/python/kd_tree.py) for more details. A more detailed [C++ comparison](./docs/benchmark.md) of PicoTree is available with respect to [nanoflann][nano].

[nano]: https://github.com/jlblancoc/nanoflann
[spkd]: https://docs.scipy.org/doc/scipy/reference/generated/scipy.spatial.KDTree.html
[skkd]: https://scikit-learn.org/stable/modules/generated/sklearn.neighbors.KDTree.html
[cvfn]: https://github.com/opencv/opencv

Available under the [MIT](https://en.wikipedia.org/wiki/MIT_License) license.

# Capabilities

* KdTree
  * Nearest neighbors, approximate nearest neighbors, radius, and box searches.
  * Customizable nearest neighbor searches, [metrics](https://en.wikipedia.org/wiki/Metric_(mathematics)) and tree splitting techniques.
  * Support for topological spaces with identifications. E.g., points on the circle `[-pi, pi]`.
  * Compile time and run time known dimensions.
  * Static tree builds.
  * Thread safe queries.
* PicoTree can interface with different types of points or point sets through a traits class. This can be a custom implementation or one of the traits classes provided by this library:
  * `pico_tree::StdTraits<>` supports interfacing with any `std::vector<PointType>`. It requires a specialization of `pico_tree::StdPointTraits<>` for each `PointType`. There are default `pico_tree::StdPointTraits<>` available for Eigen and OpenCV point types.
  * `pico_tree::EigenTraits<>` supports interfacing with Eigen matrices.
  * `pico_tree::CvTraits<>` supports interfacing with OpenCV matrices.

The examples show how PicoTree can be used:

* Creating [custom](./examples/pico_toolshed/pico_toolshed/point.hpp) `pico_tree::StdPointTraits<>`.
* `pico_tree::StdTraits<>` can be used as an example to create a [custom](./src/pico_tree/pico_tree/std_traits.hpp) traits class.
* Using the [KdTree](./examples/kd_tree/kd_tree.cpp) and creating a custom search visitor.
* Support for [Eigen](./examples/eigen/eigen.cpp) and [OpenCV](./examples/opencv/opencv.cpp) data types.
* How to use the [KdTree with Python](./examples/python/kd_tree.py).

# Requirements

Minimum:

* A compiler that is compliant with the C++11 standard or higher.
* [CMake](https://cmake.org/). It is also possible to just copy and paste the [pico_tree](./src/pico_tree/) directory into an include directory.

Optional:

* [Doxygen](https://www.doxygen.nl). Needed for generating documentation.
* [Google Test](https://github.com/google/googletest). Used for running unit tests.
* [Eigen](http://eigen.tuxfamily.org). To run the example that shows how Eigen data types can be used in combination with PicoTree.
* [OpenCV](https://opencv.org/). To run the OpenCV example that shows how to work with OpenCV data types.
* [Google Benchmark](https://github.com/google/benchmark) and a compiler that is compliant with the C++17 standard are needed to run any of the benchmarks. The [nanoflann](https://github.com/jlblancoc/nanoflann) and [OpenCV](https://opencv.org/) benchmarks also require their respective libraries to be installed.

Python bindings:
* [Python](https://www.python.org/). Version 3.5 or higher.
* [pybind11](https://github.com/pybind/pybind11). Used to ease the creation of Python bindings. Available under the [BSD](https://github.com/pybind/pybind11/blob/master/LICENSE) license and copyright.
* [OpenMP](https://www.openmp.org/). For parallelization of queries.
* [numpy](https://numpy.org/). Points and search results are represented by ndarrays.
* [scikit-build](https://scikit-build.readthedocs.io/). Glue between CMake and setuptools.

# Build

Build using [CMake](https://cmake.org/):

```console
$ mkdir build && cd build
$ cmake ../
$ make
$ make install
$ make pico_tree_doc
```

Similarly with [MSYS2](https://github.com/msys2/) and [MinGW64](http://mingw-w64.org/):

```console
$ ...
$ cmake.exe ../ -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:/msys64/mingw64/local
$ mingw32-make.exe
$ ...
```

```cmake
find_package(PicoTree REQUIRED)

add_executable(myexe main.cpp)
target_link_libraries(myexe PUBLIC PicoTree::PicoTree)
```

Install with pip:

```console
$ pip install ./pico_tree
```

Set a generator for use with MinGW:

```console
$ pip install ./pico_tree --install-option="-GMinGW Makefiles"
```

# References

* [Computational Geometry - Algorithms and Applications.](https://www.springer.com/gp/book/9783540779735) Mark de Berg, Otfried Cheong, Marc van Kreveld, and Mark Overmars, Springer-Verlag, third edition, 2008.
* S. Maneewongvatana and D. M. Mount. [It's okay to be skinny, if your friends are fat.](http://www.cs.umd.edu/~mount/Papers/cgc99-smpack.pdf) 4th Annual CGC Workshop on Computational Geometry, 1999.
* S. Arya and H. Y. Fu. [Expected-case complexity of approximate nearest neighbor searching.](https://www.cse.ust.hk/faculty/arya/pub/exp.pdf) InProceedings of the 11th ACM-SIAM Symposium on Discrete Algorithms, 2000.
* S. Arya and D. M. Mount. [Algorithms for fast vector quantization.](https://www.cs.umd.edu/~mount/Papers/DCC.pdf) In IEEE Data Compression Conference, pages 381–390, March 1993.
* N. Sample, M. Haines, M. Arnold and T. Purcell. [Optimizing Search Strategies in k-d Trees.](http://infolab.stanford.edu/~nsample/pubs/samplehaines.pdf) In: 5th WSES/IEEE World Multiconference on Circuits, Systems, Communications & Computers (CSCC 2001), July 2001.
* A. Yershova and S. M. LaValle, [Improving Motion-Planning Algorithms by Efficient Nearest-Neighbor Searching.](http://msl.cs.uiuc.edu/~lavalle/papers/YerLav06.pdf) In IEEE Transactions on Robotics, vol. 23, no. 1, pp. 151-157, Feb. 2007.
