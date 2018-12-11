
#include <gtest/gtest.h>
#include "core/include/particles.h"
#include "core/include/debug.h"
#include "core/include/file_lmp.h"

TEST(Particles, size_check) {
  try {
    feasst::Particle particle;
    particle.default_particle();
    feasst::Particles particles;
    particles.add(particle);
    feasst::Position position;
    position.set_vector({0, 0});
    particle.set_position(position);
    particles.add(particle);
    particles.check_size();
    CATCH_PHRASE("size error");
  }
}

TEST(Particles, site_types) {
  feasst::Particles particles;
  particles.add(feasst::FileLMP().read("../forcefield/data.atom"));
  particles.add(feasst::FileLMP().read("../forcefield/data.spce"));
  EXPECT_EQ(2, particles.num_site_types());
  EXPECT_EQ(0, particles.particle(0).site(0).type());
  EXPECT_EQ(0, particles.particle(1).site(0).type());
  EXPECT_EQ(1, particles.particle(1).site(1).type());
  EXPECT_EQ(1, particles.particle(1).site(2).type());

  try {
    particles.add("../forcefield/data.atom");
    CATCH_PHRASE("particles by file");
  }
}

TEST(Particles, unique_particles) {
  feasst::Particles particles;
  particles.unique_particles();
  particles.add("../forcefield/data.atom");
  particles.add("../forcefield/data.spce");
  EXPECT_EQ(2, particles.num_particle_types());
  EXPECT_EQ(3, particles.num_site_types());
  EXPECT_EQ(0, particles.particle(0).type());
  EXPECT_EQ(0, particles.particle(0).site(0).type());
  EXPECT_EQ(1, particles.particle(1).type());
  EXPECT_EQ(1, particles.particle(1).site(0).type());
  EXPECT_EQ(2, particles.particle(1).site(1).type());
  EXPECT_EQ(2, particles.particle(1).site(2).type());
}

