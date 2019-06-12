#include <memory>
#include <gtest/gtest.h>
#include "monte_carlo/include/criteria_metropolis.h"
#include "monte_carlo/include/trial.h"
#include "system/test/system_test.h"

namespace feasst {

TEST(Criteria, current_energy) {
  System sys = default_system();
  const double pe_expected = 4*(pow(1.25, -12) - pow(1.25, -6));
  EXPECT_NEAR(sys.energy(), pe_expected, NEAR_ZERO);
  auto trans = MakeTrialTranslate({{"tunable_param", "0.1"}});
  CriteriaMetropolis crit;
  crit.set_current_energy(sys.energy());
  try {
    auto sys2 = sys;
    auto crit2 = crit;
    trans->attempt(&crit2, &sys2);
    CATCH_PHRASE("beta must be initialized");
  }
  crit.set_beta(1.);
  trans->attempt(&crit, &sys);
  EXPECT_NEAR(sys.energy(), crit.current_energy(), NEAR_ZERO);
}

}  // namespace feasst
