#include <gtest/gtest.h>
#include "core/include/long_range_corrections.h"
#include "core/include/model_empty.h"
#include "core/test/configuration_test.h"

namespace feasst {

TEST(LongRangeCorrections, LRC) {
  Configuration config = default_configuration();
  ModelEmpty empty;
  LongRangeCorrections lrc;
  empty.compute(&config, &lrc);
  const double pe_lrc = (8./3.)*PI*pow(config.num_particles(), 2)/config.domain().volume()
    *((1./3.)*pow(3, -9) - pow(3, -3));
  EXPECT_NEAR(pe_lrc, lrc.energy(), NEAR_ZERO);

  // serialize
  std::stringstream ss;
  lrc.serialize(ss);
  auto visit = lrc.deserialize(ss);
  empty.compute(&config, visit.get());
  EXPECT_NEAR(visit->energy(), lrc.energy(), NEAR_ZERO);
}

}  // namespace feasst
