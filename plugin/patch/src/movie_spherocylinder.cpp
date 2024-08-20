#include "utils/include/serialize.h"
#include "utils/include/arguments.h"
#include "system/include/system.h"
#include "monte_carlo/include/criteria.h"
#include "patch/include/movie_spherocylinder.h"

namespace feasst {

class MapMovieSpherocylinder {
 public:
  MapMovieSpherocylinder() {
    auto obj = MakeMovieSpherocylinder({{"output_file", "place_holder"}});
    obj->deserialize_map()["MovieSpherocylinder"] = obj;
  }
};

static MapMovieSpherocylinder mapper_ = MapMovieSpherocylinder();

MovieSpherocylinder::MovieSpherocylinder(argtype * args) : AnalyzeWriteOnly(args) {
  set_append();
  ASSERT(!output_file().empty(), "file name is required");
  args->insert({"append", "true"}); // always append
  xyz_ = FileXYZSpherocylinder(args);
}
MovieSpherocylinder::MovieSpherocylinder(argtype args) : MovieSpherocylinder(&args) { feasst_check_all_used(args); }

void MovieSpherocylinder::initialize(Criteria * criteria,
    System * system,
    TrialFactory * trial_factory) {
  const std::string name = output_file(*criteria);
  ASSERT(!name.empty(), "file name required. Did you forget to " <<
    "Analyze::set_output_file()?");

  // write xyz
  if (state() == criteria->state()) {
    xyz_.write(name, system->configuration());
  }

  // write vmd
  std::stringstream ss;
  ss << name << ".vmd";
  vmd_.write(ss.str(), system->configuration(), name);
}

std::string MovieSpherocylinder::write(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory) {
  // ensure the following order matches the header from initialization.
  xyz_.write(output_file(criteria), system.configuration());
  return std::string("");
}

void MovieSpherocylinder::serialize(std::ostream& ostr) const {
  Stepper::serialize(ostr);
  feasst_serialize_version(2364, ostr);
  feasst_serialize_fstobj(xyz_, ostr);
  feasst_serialize_fstobj(vmd_, ostr);
}

MovieSpherocylinder::MovieSpherocylinder(std::istream& istr) : AnalyzeWriteOnly(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 2364, "version mismatch:" << version);
  feasst_deserialize_fstobj(&xyz_, istr);
  feasst_deserialize_fstobj(&vmd_, istr);
}

}  // namespace feasst
