
#include "utils/test/utils.h"
#include "configuration/include/configuration.h"
#include "system/include/lennard_jones.h"
#include "system/include/potential.h"
#include "system/include/visit_model.h"
#include "system/include/system.h"
#include "monte_carlo/include/monte_carlo.h"
#include "monte_carlo/include/run.h"
#include "monte_carlo/include/convert_to_ref_potential.h"

namespace feasst {

TEST(ConvertToRefPotential, lj) {
  MonteCarlo mc;
  mc.add(MakeConfiguration({{"cubic_side_length", "8"}, {"particle_type0", "../particle/lj.fstprt"}}));
  mc.add(MakePotential(MakeLennardJones()));
  mc.run(MakeConvertToRefPotential({{"cutoff", "1"}, {"use_cell", "true"}}));
  EXPECT_EQ(mc.system().num_references(), 1);
  EXPECT_EQ(mc.system().reference(0, 0).visit_model().class_name(), "VisitModelCell");
}

}  // namespace feasst
