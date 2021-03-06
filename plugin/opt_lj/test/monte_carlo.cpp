#include "utils/test/utils.h"
#include "math/include/random_mt19937.h"
#include "system/include/lennard_jones.h"
#include "system/include/utils.h"
#include "monte_carlo/include/monte_carlo.h"
#include "monte_carlo/include/trial_factory.h"
#include "monte_carlo/include/trial_transfer.h"
#include "monte_carlo/include/seek_num_particles.h"
#include "monte_carlo/include/metropolis.h"
#include "monte_carlo/include/trial_translate.h"
#include "opt_lj/include/visit_model_opt_lj.h"
//#include "system/include/energy_map_all.h"

namespace feasst {

TEST(MonteCarlo, NVT_opt_lj_benchmark) {
  MonteCarlo mc;
  mc.set(lennard_jones());
  mc.set(MakeMetropolis({{"beta", "1.2"}, {"chemical_potential", "1."}}));
  mc.add(MakeTrialTranslate({{"weight", "1."}, {"tunable_param", "1."}}));
  // HWH Perhaps implement as optimized potential instead of optimized VisitModel
  mc.add_to_optimized(Potential(MakeLennardJones(), //HWH: prevents ModelEmpty... how to remove?
                                MakeVisitModelOptLJ()));
 //                               MakeVisitModelOptLJ(MakeVisitModelInner(MakeEnergyMapAll()))));
  mc.set(MakeRandomMT19937({{"seed", "default"}}));
  SeekNumParticles(50).with_trial_add().run(&mc);
  // mc.seek_num_particles(250);
  // mc.attempt(1e6);  // 4.1 sec with 50 [5 sec 11/21/19, 4.3 on 1/6/19, 4.5 on 1/30/19 (4.18 VERBOSE_LEVEL=0)]
  // mc.seek_num_particles(450);
  // mc.attempt(1e5);  // 15 sec with 450 on slow computer
  mc.attempt(1e3);
  // DEBUG("\n" << mc.timer_str());
}

}
