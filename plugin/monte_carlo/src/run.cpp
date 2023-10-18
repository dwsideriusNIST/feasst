
#include <fstream>
#include <sstream>
#include "utils/include/serialize.h"
#include "utils/include/debug.h"
#include "utils/include/timer.h"
#include "math/include/utils_math.h"
#include "system/include/visit_model_cell.h"
#include "monte_carlo/include/run.h"
#include "monte_carlo/include/monte_carlo.h"

namespace feasst {

Run::Run(argtype * args) {
  num_trials_ = integer("num_trials", args, -1);
  until_num_particles_ = integer("until_num_particles", args, -1);
  configuration_index_ = integer("configuration_index", args, 0);
  particle_type_ = integer("particle_type", args, -1);
  for_hours_ = dble("for_hours", args, -1);
  until_criteria_complete_ = boolean("until_criteria_complete", args, false);
  class_name_ = "Run";
}
Run::Run(argtype args) : Run(&args) {
  FEASST_CHECK_ALL_USED(args);
}

class MapRun {
 public:
  MapRun() {
    auto obj = MakeRun();
    obj->deserialize_map()["Run"] = obj;
  }
};

static MapRun mapper_Run = MapRun();

Run::Run(std::istream& istr) : Action(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version >= 3854 && version <= 3855, "mismatch version: " << version);
  feasst_deserialize(&num_trials_, istr);
  feasst_deserialize(&until_num_particles_, istr);
  if (version >= 3855) {
    feasst_deserialize(&configuration_index_, istr);
  }
  feasst_deserialize(&particle_type_, istr);
  feasst_deserialize(&for_hours_, istr);
  feasst_deserialize(&until_criteria_complete_, istr);
}

void Run::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  serialize_action_(ostr);
  feasst_serialize_version(3855, ostr);
  feasst_serialize(num_trials_, ostr);
  feasst_serialize(until_num_particles_, ostr);
  feasst_serialize(configuration_index_, ostr);
  feasst_serialize(particle_type_, ostr);
  feasst_serialize(for_hours_, ostr);
  feasst_serialize(until_criteria_complete_, ostr);
}

void Run::run(MonteCarlo * mc) {
  while (num_trials_ > 0) {
    mc->attempt(1);
    --num_trials_;
    DEBUG("num_trials " << num_trials_);
  }
  const Configuration& conf = mc->configuration(configuration_index_);
  while ((until_num_particles_ > 0) &&
         ((particle_type_ == -1 && (conf.num_particles() != until_num_particles_)) ||
          (particle_type_ != -1 && (conf.num_particles_of_type(particle_type_) != until_num_particles_)))) {
    mc->attempt(1);
    DEBUG("num_particles " << conf.num_particles());
  }
  if (for_hours_ > 0) {
    const double begin = cpu_hours();
    while (for_hours_ > cpu_hours() - begin) {
      mc->attempt(10);
    }
  }
  if (until_criteria_complete_) {
    DEBUG("num iterations " << mc->criteria().num_iterations());
    DEBUG("num iterations to complete " << mc->criteria().num_iterations_to_complete());
    DEBUG("mc->criteria().is_complete() " << mc->criteria().is_complete());
    while (!mc->criteria().is_complete()) {
      DEBUG("mc->criteria().is_complete() " << mc->criteria().is_complete());
      mc->attempt(1);
    }
  }
}

}  // namespace feasst
