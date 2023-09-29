#include "utils/test/utils.h"
#include "aniso/include/visit_model_inner_table.h"

namespace feasst {

TEST(VisitModelInnerTable, b2) {
  //const std::string table_file = "../plugin/aniso/tutorial/dat_sqw_10rel_2z.txt";
  //const std::string table_file = "../plugin/aniso/tutorial/dat_sqw_15rel_2z.txt";
  //const std::string table_file = "../plugin/aniso/tutorial/dat.txt";
  //const std::string table_file = "../plugin/aniso/tutorial/table15.txt";
  //const std::string table_file = "../plugin/aniso/tutorial/table3.txt";
  const std::string table_file = "../plugin/aniso/test/data/dat_sqw_3rel_2z.txt";
  //const std::string table_file = "../plugin/aniso/test/data/dat_sqw_6rel_2z.txt";
  auto vis = MakeVisitModelInnerTable({{"table_file", table_file}});
  //EXPECT_NEAR((2*PI/3)*(1-(std::pow(1.5, 3)-1)*(std::exp(1)-1)), vis->second_virial_coefficient(), NEAR_ZERO);
  EXPECT_NEAR((2*PI/3)*(1-(std::pow(1.5, 3)-1)*(std::exp(1)-1)), vis->second_virial_coefficient({{"expand_t", "1"}}), 1);
  //EXPECT_NEAR(1.7225709266833233, vis->second_virial_coefficient(), NEAR_ZERO);
  //EXPECT_NEAR(2.024442976806, vis->second_virial_coefficient({{"expand_t", "2"}}), 1e-12);
//  EXPECT_NEAR(2*PI/3., vis->second_virial_coefficient({{"expand_t", "10"}}), NEAR_ZERO);
//  vis->write_surface({{"xyz_file", "tmp/surf.xyz"}, {"expand_t", "1"}});
}

TEST(VisitModelInnerTable, mab) {
  const std::string table_file = "../plugin/aniso/test/data/mab_p1z2.txt";
  auto vis = MakeVisitModelInnerTable({{"table_file", table_file}});
  EXPECT_NEAR(74.09830, vis->inner()[0][0].data()[0][0][0][0][0], 1e-5);
  EXPECT_NEAR(71.76155, vis->inner()[0][0].data()[0][0][0][0][1], 1e-5);
  EXPECT_NEAR(74.09830, vis->inner()[0][0].data()[0][0][0][0][2], 1e-5);
  EXPECT_NEAR(-6.835550, vis->energy()[0][0].data()[0][0][0][0][0][0], 1e-6);
  EXPECT_NEAR(-0.001538283, vis->energy()[0][0].data()[0][0][0][0][0][1], 1e-9);
  EXPECT_NEAR(-6.736353, vis->energy()[0][0].data()[0][0][0][0][1][0], 1e-6);
  EXPECT_NEAR(0.001778595, vis->energy()[0][0].data()[0][0][0][0][1][1], 1e-9);
  EXPECT_NEAR(-6.835550, vis->energy()[0][0].data()[0][0][0][0][2][0], 1e-6);
  EXPECT_NEAR(-0.001538283, vis->energy()[0][0].data()[0][0][0][0][2][1], 1e-9);
}

}  // namespace feasst
