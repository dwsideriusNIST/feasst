#include <gtest/gtest.h>
#include <limits.h>
#include "accumulator_vec.h"

using namespace feasst;

TEST(Accumulator, constructor) {
  Accumulator a;
  a.setBlock(5);
  for (int i = 0; i < 20; ++i) {
    a.accumulate(i);
  }
  EXPECT_EQ(20, a.nValues());
  EXPECT_NEAR(19/2., a.average(), 1e-15);
  EXPECT_NEAR(5.916079783, a.stdev(), 1e-10);
  EXPECT_NEAR(3.2274861218, a.blockStdev(), 1e-10);

  // copy a using copy constructor and constructor and test
  Accumulator b(a.nValues(), a.sum(), a.sumSq());
  Accumulator c = a;
  EXPECT_EQ(b.nValues(), c.nValues());
  EXPECT_EQ(b.sum(), c.sum());
  EXPECT_EQ(b.sumSq(), c.sumSq());
  EXPECT_EQ(b.average(), c.average());
  EXPECT_EQ(b.stdev(), c.stdev());

  // reset a and test
  a.reset();
  EXPECT_NEAR(0., a.average(), 1e-15);
  EXPECT_EQ(0, a.nValues());

}

TEST(Accumulator, vec) {
  AccumulatorVec stat;
  EXPECT_EQ(0, stat.size());
  stat.accumulate(50, 3);
  EXPECT_EQ(51, stat.size());
  EXPECT_EQ(3, stat.vec()[50].average());
  stat.accumulate(100, 1);
  stat.accumulate(100, 2);
  stat.accumulate(100, 3);
  EXPECT_EQ(101, stat.size());
  EXPECT_EQ(2, stat.vec()[100].average());
  vector<double> p = stat.hist();
  EXPECT_NEAR(3./4., p[100], 1e-17);
  EXPECT_NEAR(1./4., p[50], 1e-17);
}
