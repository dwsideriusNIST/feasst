#include <gtest/gtest.h>
#include "core/include/domain.h"
#include "core/include/constants.h"

namespace feasst {

TEST(Domain, Volume) {
  Domain domain;
  domain.set_cubic(5);
  EXPECT_EQ(125, domain.volume());
}

TEST(Domain, min_side_length) {
  Domain domain;
  domain.set_cubic(10);
  EXPECT_NEAR(10, domain.min_side_length(), NEAR_ZERO);
  domain.set_side_length(1, 5.234);
  EXPECT_NEAR(5.234, domain.min_side_length(), NEAR_ZERO);
}

// compute the volume of a sphere
TEST(Domain, random_position) {
  Domain domain;
  Random random;
  domain.set_cubic(1);
  int inside = 0;
  const int trials = 1e3;
  for (int trial = 0; trial < trials; ++trial) {
    if (domain.random_position(&random).squared_distance() <= 1) {
      ++inside;
    }
  }
  EXPECT_NEAR(static_cast<double>(inside)/trials, PI/6, 0.06);
}

TEST(Domain, wrap) {
  auto domain = Domain().set_cubic(5);
  Position pos;
  pos.set_vector({5, 5, 5});
  Position shift = domain.shift(pos);
  EXPECT_FALSE(domain.is_tilted());
  EXPECT_NEAR(-5, shift.coord(0), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(1), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(2), NEAR_ZERO);

  domain.set_xy(1);
  shift = domain.shift(pos);
  EXPECT_TRUE(domain.is_tilted());
  EXPECT_NEAR(-6, shift.coord(0), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(1), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(2), NEAR_ZERO);

  domain.set_xz(1);
  shift = domain.shift(pos);
  EXPECT_TRUE(domain.is_tilted());
  EXPECT_NEAR(-7, shift.coord(0), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(1), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(2), NEAR_ZERO);

  domain.set_yz(1);
  shift = domain.shift(pos);
  EXPECT_TRUE(domain.is_tilted());
  EXPECT_NEAR(-7, shift.coord(0), NEAR_ZERO);
  EXPECT_NEAR(-6, shift.coord(1), NEAR_ZERO);
  EXPECT_NEAR(-5, shift.coord(2), NEAR_ZERO);
}

}  // namespace feasst