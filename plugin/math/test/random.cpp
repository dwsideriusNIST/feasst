#include <cmath>
#include <vector>
#include "utils/test/utils.h"
#include "math/include/random_modulo.h"
#include "math/include/random_mt19937.h"
#include "math/include/histogram.h"
#include "math/include/accumulator.h"

namespace feasst {

std::vector<std::shared_ptr<Random> > gens = {MakeRandomMT19937(), MakeRandomModulo()};
//std::vector<std::shared_ptr<Random> > gens = {MakeRandomModulo()};

TEST(Random, uniform) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    double sum = 0;
    const int num = 1e5;
    for (int i = 0; i < num; ++i) {
      const double uni = random->uniform();
      //INFO("uni " << uni);
      sum += uni;
    }
    EXPECT_NEAR(sum, double(num)/2., sqrt(double(num)));
  }
}

TEST(Random, uniform_int) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    double sum = 0;
    const int num = 1e5;
    for (int i = 0; i < num; ++i) {
      sum += random->uniform(0, 10);
    }
    EXPECT_NEAR(sum, 5*double(num), 15*sqrt(double(num)));
  }
}

TEST(Random, alpha_numeric) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    const int size = 10;
    std::string unique = random->alpha_numeric(size);
    INFO("unique alpha numeric: " << unique);
    EXPECT_EQ(unique.size(), size);
    EXPECT_NE(unique, random->alpha_numeric(size));
  }
}

TEST(Random, element) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    std::vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    double sum = 0;
    const int num = 1e5;
    for (int i = 0; i < num; ++i) {
      sum += random->const_element(vec);
    }
    EXPECT_NEAR(sum, 5*double(num), 15*sqrt(double(num)));
  }
}

TEST(Random, unit_sphere) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    Position position;
    position.set_vector({0., 0.});
    random->unit_sphere_surface(&position);
    EXPECT_NEAR(position.distance(), 1., NEAR_ZERO);
    position.set_vector({0., 0., 0.});
    random->unit_sphere_surface(&position);
    EXPECT_NEAR(position.distance(), 1., NEAR_ZERO);

  //  // visualize
  //  for (int point = 0; point < 1e3; ++point) {
  //    random.unit_sphere_surface(&position);
  //    std::cout << position.str() << std::endl;
  //  }
  }
}

TEST(Random, spherical_shell) {
  const double upper = 3, lower = 1.;
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    Position position;

    // 3D
    position.set_vector({0., 0., 0.});
    for (int point = 0; point < 1e3; ++point) {
      random->position_in_spherical_shell(lower, upper, &position);
      EXPECT_LE(lower, position.distance());
      EXPECT_GE(upper, position.distance());
      // visualize
      // std::cout << position.str() << std::endl;
    }

    // 2D
    position.set_vector({0., 0.});
    for (int point = 0; point < 1e3; ++point) {
      random->position_in_spherical_shell(lower, upper, &position);
      EXPECT_LE(lower, position.distance());
      EXPECT_GE(upper, position.distance());
      // visualize
      // std::cout << position.str() << std::endl;
    }
  }
}

TEST(Random, index_from_cumulative_probability) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    std::vector<double> cpdf;
    const int ncpdf = 10, num = 100;
    for (int i = 0; i < ncpdf; ++i) cpdf.push_back((i+1)/double(ncpdf));
    std::vector<double> cpdfran(ncpdf);
    for (int i = 0; i < num; ++i) {
      const int j = random->index_from_cumulative_probability(cpdf);
      ++cpdfran[j];
    }
    for (int i = 0; i < ncpdf; ++i) EXPECT_NEAR(cpdfran[i]/double(num), ncpdf/double(num), 0.2);
  }
}

TEST(RandomModulo, compiler_independent) {
  auto random = MakeRandomModulo({{"seed", "1346867550"}});
//  EXPECT_NEAR(random.uniform(), 0, NEAR_ZERO);
//  EXPECT_NEAR(random.uniform(), 0, NEAR_ZERO);
//  EXPECT_NEAR(random.uniform(), 0, NEAR_ZERO);
  EXPECT_EQ(random->uniform(), 0.082789837886947132);
  EXPECT_EQ(random->uniform(), 0.44880536592044185);
  EXPECT_EQ(random->uniform(), 0.071785024866361652);
}

TEST(RandomModulo, serialize) {
  RandomModulo random;
  random.seed_by_time();
  random.set_cache_to_load(true);
  RandomModulo random2 = test_serialize(random);
  const double next = random.uniform();
  EXPECT_EQ(next, random2.uniform());
  RandomModulo random3;
  random3.set_cache_to_unload(random2);
  EXPECT_EQ(next, random3.uniform());
  random3.set_cache_to_load(false);
  EXPECT_NE(random.uniform(), random3.uniform());
  TRY(
    random3.set_cache_to_unload(random2);
    random3.uniform();
    random3.uniform();
    CATCH_PHRASE("can not unload if nothing stored");
  );
}

TEST(RandomMT19937, serialize) {
  RandomMT19937 random;
  random.seed_by_time();
  random.set_cache_to_load(true);
  RandomMT19937 random2 = test_serialize(random);
  const double next = random.uniform();
  EXPECT_EQ(next, random2.uniform());
  RandomMT19937 random3;
  random3.set_cache_to_unload(random2);
  EXPECT_EQ(next, random3.uniform());
  random3.set_cache_to_load(false);
  EXPECT_NE(random.uniform(), random3.uniform());
  TRY(
    random3.set_cache_to_unload(random2);
    random3.uniform();
    random3.uniform();
    CATCH_PHRASE("can not unload if nothing stored");
  );
}

TEST(Random, standard_normal) {
  for (std::shared_ptr<Random> random : gens) {
    random->seed_by_time();
    double sum = 0., sumsq = 0.;
    const int num = 1e5;
    const double num_dble = static_cast<double>(num);
    for (int i = 0; i < num; ++i) {
      const double ran = random->standard_normal();
      sum += ran;
      sumsq += ran*ran;
    }
    const double av = sum/num_dble;
    const double stdev = std::sqrt((sumsq/num_dble-av*av));
    EXPECT_NEAR(sum, 0., 4*std::sqrt(num_dble));
    EXPECT_NEAR(stdev, 1., 0.01);
  }
}

//TEST(Random, bond_length) {
//  for (std::shared_ptr<Random> random : gens) {
//    random->seed_by_time();
//    double spring_constant = 100.;
//    const double equilibrum_length = 3.;
//    double sum1 = 0., sumsq1 = 0., sum2 = 0., sumsq2 = 0.;
//    const int num = 5e3;
//    const double num_dble = static_cast<double>(num);
//  //  Histogram hist1, hist2;
//  //  hist1.set_width_center(0.01, equilibrum_length);
//  //  hist2.set_width_center(0.01, equilibrum_length);
//    //({{"width", "0.1"}, {"max", "6"}});;
//    for (int i = 0; i < num; ++i) {
//      const double ran1 = random->bond_length(equilibrum_length, 2*equilibrum_length, spring_constant, 2, 3);
//      const double ran2 = random->harmonic_bond_length(equilibrum_length, spring_constant, 3);
//  //    hist1.add(ran1);
//  //    hist2.add(ran2);
//      sum1 += ran1; sumsq1 += ran1*ran1; sum2 += ran2; sumsq2 += ran2*ran2;
//    }
//    const double av1 = sum1/num_dble;
//    const double av2 = sum2/num_dble;
//    const double stdev1 = std::sqrt((sumsq1/num_dble-av1*av1));
//    const double stdev2 = std::sqrt((sumsq2/num_dble-av2*av2));
//    const double stdev_exp = 1./std::sqrt(2*spring_constant);
//    EXPECT_NEAR(av1, equilibrum_length, 8e-2);
//    EXPECT_NEAR(av2, equilibrum_length, 8e-2);
//    EXPECT_NEAR(stdev1, stdev_exp, 5e-2);
//    EXPECT_NEAR(stdev2, stdev_exp, 5e-2);
//  //  for (int bin = 0; bin < hist.size(); ++bin) {
//  //    std::cout << hist.center_of_bin(bin) << " " << hist.histogram()[bin] << std::endl;
//  //  }
//  }
//}

//TEST(Random, bond_angle) {
//  for (std::shared_ptr<Random> random : gens) {
//    random->seed_by_time();
//    double sum1 = 0., sumsq1 = 0., sum2 = 0., sumsq2 = 0.;
//    const int num = 2e3;
//    const double num_dble = static_cast<double>(num);
//    Position point(3);
//    for (int i = 0; i < num; ++i) {
//      const double ran1 = random->bond_angle(0, 0, 2, 3);
//      random->position_in_spherical_shell(0, 1., &point);
//      const double ran2 = point.spherical().coord(2);
//      sum1 += ran1; sumsq1 += ran1*ran1; sum2 += ran2; sumsq2 += ran2*ran2;
//    }
//    const double av1 = sum1/num_dble;
//    const double av2 = sum2/num_dble;
//    const double stdev1 = std::sqrt((sumsq1/num_dble-av1*av1));
//    const double stdev2 = std::sqrt((sumsq2/num_dble-av2*av2));
//    const double stdev_exp = 0.675;
//    EXPECT_NEAR(av1, PI/2, 8e-2);
//    EXPECT_NEAR(av2, PI/2, 8e-2);
//    EXPECT_NEAR(stdev1, stdev_exp, 5e-2);
//    EXPECT_NEAR(stdev2, stdev_exp, 5e-2);
//  }
//}

// Compute the average bond length for a potential U(L)=600(L-1)^2
TEST(Random, harmonic_bond_brute) {
  const double k = 600;
  const double l0 = 1;
  auto ran = MakeRandomMT19937();
  double old_dist = l0;
  Position xn({{"x", feasst::str(old_dist)}, {"y", "0"}, {"z", "0"}});
  Position old_x = xn;
  Accumulator length;
  double old_en = 0.;
  const double max_move = 0.1;
  int accepted = 0;
  const int num_attempts = 3e5;
  //const int num_attempts = 1e7;
  for (int i = 0; i < num_attempts; ++i) {
    //ran->position_in_spherical_shell(0.5, 1.5, &xn);
    for (int dim = 0; dim < xn.dimension(); ++dim) {
      xn.set_coord(dim, old_x.coord(dim) + max_move*(2*ran->uniform() - 1));
    }
    const double dist = xn.distance(), dx = dist - l0;
    const double en = k*dx*dx;
    const double delta_en = en - old_en;
    if (ran->uniform() < std::exp(-delta_en)) {
      old_dist = dist;
      old_en = en;
      old_x = xn;
      ++accepted;
    }
    length.accumulate(old_dist);
  }
  INFO("acceptance " << static_cast<double>(accepted)/num_attempts);
  INFO(length.str());
  EXPECT_NEAR(1.+1./k, length.average(), 5*length.block_stdev());
}

// Compute the average bond angle for a potential U(theta)=600(theta-PI/2)^2
TEST(Random, harmonic_angle_brute) {
  const double k = 600;
  const double theta0 = PI/2;
  double old_theta = theta0;
  auto ran = MakeRandomMT19937();
  Position xa1({{"x", "0"}, {"y", "1"}, {"z", "0"}});
  Position xa0({{"x", "0"}, {"y", "0"}, {"z", "0"}});
  Position xn({{"x", "1"}, {"y", "0"}, {"z", "0"}});
  Position old_x = xn;
  Accumulator angle;
  double old_en = 0.;
  const double max_move = 1;
  int accepted = 0;
  const int num_attempts = 3e5;
  //const int num_attempts = 1e7;
  for (int i = 0; i < num_attempts; ++i) {
    //ran->position_in_spherical_shell(0.5, 1.5, &xn);
    for (int dim = 0; dim < xn.dimension(); ++dim) {
      xn.set_coord(dim, old_x.coord(dim) + max_move*(2*ran->uniform() - 1));
    }
    const double theta = xa0.vertex_angle_radians(xn, xa1);
    const double dtheta = theta - theta0;
    const double en = k*dtheta*dtheta;
    const double delta_en = en - old_en;
    if (ran->uniform() < std::exp(-delta_en)) {
      old_theta = theta;
      old_en = en;
      old_x = xn;
      ++accepted;
    }
    angle.accumulate(old_theta);
  }
  INFO("acceptance " << static_cast<double>(accepted)/num_attempts);
  INFO(angle.str() << " PI/2 " << PI/2);
  EXPECT_NEAR(PI/2, angle.average(), 15*angle.block_stdev());
}

}  // namespace feasst
