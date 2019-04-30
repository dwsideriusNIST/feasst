#include <gtest/gtest.h>
#include "core/include/cells.h"
#include "core/include/debug.h"

namespace feasst {

TEST(Cells, cells) {
  Cells cells;
  EXPECT_EQ(0, cells.num_total());
  try {
    cells.create(3, {14});
    CATCH_PHRASE("unrecognized dim");
  }
  try {
    cells.create(3, {14, 14, 14, 14});
    CATCH_PHRASE("unrecognized dim");
  }
  cells.create(3, {12, 12, 13});
  EXPECT_EQ(4*4*4, cells.num_total());
  EXPECT_EQ(4, cells.num()[0]);
  EXPECT_EQ(cells.neighbor().size(), 4*4*4);
  for (const std::vector<int>& neigh : cells.neighbor()) {
    EXPECT_EQ(neigh.size(), 3*3*3);
  }
  EXPECT_EQ(0, cells.id({-0.49, -0.49, -0.49}));
  try {
    EXPECT_EQ(0, cells.id({-0.501, -0.49, -0.49}));
    CATCH_PHRASE("not scaled");
  }

  /// 2D
  cells.create(3, {12, 13});
  EXPECT_EQ(4*4, cells.num_total());
  EXPECT_EQ(4, cells.num()[0]);
  EXPECT_EQ(cells.neighbor().size(), 4*4);
  for (const std::vector<int>& neigh : cells.neighbor()) {
    EXPECT_EQ(neigh.size(), 3*3);
  }

  /// test id
  cells.create(1, {6, 6});
  EXPECT_EQ(6*6, cells.num_total());
  EXPECT_EQ(6, cells.num()[0]);
  try {
    EXPECT_EQ(0, cells.id({-0.49, -0.49, -0.49}));
    CATCH_PHRASE("size error");
  }
  EXPECT_EQ(0, cells.id({-0.49, -0.49}));
  EXPECT_EQ(14, cells.id({-0.001, -0.001}));
  EXPECT_EQ(21, cells.id({0.001, 0.001}));
  EXPECT_EQ(20, cells.id({-0.001, 0.001}));
  EXPECT_EQ(15, cells.id({0.001, -0.001}));
  EXPECT_EQ(35, cells.id({0.49, 0.49}));
  EXPECT_EQ(28, cells.neighbor()[35][0]);
  EXPECT_EQ(0, cells.neighbor()[35][8]);
  for (int cell = 0; cell < cells.num_total(); ++cell) {
    EXPECT_EQ(cell, cells.neighbor()[cell][4]);
  }

  // serialize
  std::stringstream ss;
  cells.serialize(ss);
  Cells cells2(ss);
  EXPECT_EQ(cells.neighbor(), cells2.neighbor());
}

}  // namespace feasst
