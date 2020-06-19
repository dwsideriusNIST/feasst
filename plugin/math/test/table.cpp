#include <cmath>
#include "utils/test/utils.h"
#include "math/include/table.h"
#include "math/include/constants.h"

namespace feasst {

TEST(Table3D, interpolate) {
  auto table = MakeTable3D(
    {{"num0", "2"}, {"num1", "2"}, {"num2", "2"}, {"default_value", "0."}});
  EXPECT_NEAR(table->linear_interpolation(0.5, 0.5, 0.5), 0, NEAR_ZERO);
  table->set_data(0, 0, 0, 1.);
  auto table2 = std::make_shared<Table3D>(test_serialize(*table));
  EXPECT_NEAR(table->linear_interpolation(0.5, 0.5, 0.5), 1./8., NEAR_ZERO);
  EXPECT_EQ(table->bin_to_value(0, 1), 1);
  EXPECT_EQ(table->value_to_nearest_bin(0, 0.45), 0);
  EXPECT_EQ(table->value_to_nearest_bin(0, 0.501), 1);
}

}  // namespace feasst
