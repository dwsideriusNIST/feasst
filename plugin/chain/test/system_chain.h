#include "configuration/include/domain.h"
#include "system/include/long_range_corrections.h"
#include "system/include/visit_model_intra.h"
#include "system/include/visit_model_cell.h"
#include "system/include/lennard_jones.h"

namespace feasst {

inline Configuration config() {
  Configuration config(MakeDomain({{"cubic_box_length", "12"},
                                   {"init_cells", "1."}}),
    {{"particle_type0", "../forcefield/data.chain10"}});
  config.add_particle_of_type(0);
  return config;
}

inline Potential lj_dual_cut(const Configuration config) {
  Potential lj_dual_cut(MakeLennardJones(), MakeVisitModelCell());
  lj_dual_cut.set_model_params(config);
  lj_dual_cut.set_model_param("cutoff", 0, 1);
  return lj_dual_cut;
}

inline Potential lj_intra_dual_cut(const Configuration config) {
  Potential lj_intra_dual_cut(MakeLennardJones(),
                              MakeVisitModelIntra({{"cutoff", "1"}}));
  lj_intra_dual_cut.set_model_params(config);
  lj_intra_dual_cut.set_model_param("cutoff", 0, 1);
  return lj_intra_dual_cut;
}

inline System chain_system() {
  System system;
  system.add(config());
  system.add_to_unoptimized(Potential(MakeLennardJones()));
  system.add_to_reference(lj_dual_cut(system.configuration()));
  system.add_to_unoptimized(Potential(MakeLennardJones(),
                                      MakeVisitModelIntra({{"cutoff", "1"}})));
  system.add_to_reference(lj_intra_dual_cut(system.configuration()));
  system.add_to_unoptimized(Potential(MakeLongRangeCorrections()));
  return system;
}

}  // namespace feasst
