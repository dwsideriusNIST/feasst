
#include "utils/test/utils.h"
#include "charge/include/utils.h"

namespace feasst {

TEST(Charge, utils) {
  System sys = spce({{"physical_constants", "CODATA2010"}});
  EXPECT_EQ(sys.configuration().model_params().physical_constants().class_name(), "CODATA2010");
}

}  // namespace feasst
