//https://stackoverflow.com/questions/2486093/millions-of-3d-points-how-to-find-the-10-of-them-closest-to-a-given-point

// $ g++ nearest.cc && (time ./a.out < million_3D_points.txt )
#include <algorithm>
#include <iostream>
#include <vector>

#include <boost/lambda/lambda.hpp>  // _1
#include <boost/lambda/bind.hpp>    // bind()
#include <boost/tuple/tuple_io.hpp>

namespace {
typedef double coord_t;
typedef boost::tuple<coord_t, coord_t, coord_t> point_t;

coord_t distance_sq(const point_t& a, const point_t& b) { // or boost::geometry::distance
  coord_t x = a.get<0>() - b.get<0>();
  coord_t y = a.get<1>() - b.get<1>();
  coord_t z = a.get<2>() - b.get<2>();
  return x * x + y * y + z * z;
}
}

int main() {
  using namespace std;
  using namespace boost::lambda; // _1, _2, bind()

  // read array from stdin
  vector<point_t> points;
  cin.exceptions(ios::badbit); // throw exception on bad input
  while (cin) {
    coord_t x, y, z;
    cin >> x >> y >> z;
    points.push_back(boost::make_tuple(x, y, z));
  }

  // use point value from previous examples
  point_t point(69.06310224, 2.23409409, 50.41979143);
  cout << "point: " << point << endl;  // 1.14s

  // find 10 nearest points using partial_sort()
  // Complexity: O(N)*log(m) comparisons (O(N)*log(N) worst case for the implementation)
  const size_t m = 10;
  partial_sort(points.begin(), points.begin() + m, points.end(),
               bind(less<coord_t>(), // compare by distance to the point
                    bind(distance_sq, _1, point),
                    bind(distance_sq, _2, point)));
  for_each(points.begin(), points.begin() + m, cout << _1 << "\n"); // 1.16s
}