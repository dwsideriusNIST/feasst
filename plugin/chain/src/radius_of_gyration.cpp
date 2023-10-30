#include "utils/include/serialize.h"
#include "chain/include/radius_of_gyration.h"

namespace feasst {

class MapRadiusOfGyration {
 public:
  MapRadiusOfGyration() {
    auto obj = MakeRadiusOfGyration();
    obj->deserialize_map()["RadiusOfGyration"] = obj;
  }
};

static MapRadiusOfGyration mapper_ = MapRadiusOfGyration();

RadiusOfGyration::RadiusOfGyration(argtype * args) : Analyze(args) {
  group_index_ = integer("group_index", args, 0);
}
RadiusOfGyration::RadiusOfGyration(argtype args) : RadiusOfGyration(&args) {
  FEASST_CHECK_ALL_USED(args);
}

void RadiusOfGyration::initialize(Criteria * criteria,
    System * system,
    TrialFactory * trial_factory) {
  printer(header(*criteria, *system, *trial_factory),
          output_file(*criteria));
}

std::string RadiusOfGyration::header(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory) const {
  std::stringstream ss;
  ss << accumulator_.status_header() << ",rgu,rguu" << std::endl;
  return ss.str();
}

void RadiusOfGyration::update(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory) {
  const Select& selection = system.configuration().group_selects()[group_index_];
  for (int select_index = 0;
       select_index < selection.num_particles();
       ++select_index) {
    const int part_index = selection.particle_index(select_index);
    const Particle& part = system.configuration().select_particle(part_index);
    Position r_cm(system.configuration().dimension());
    for (int site_index : selection.site_indices(select_index)) {
      const Site& site = part.site(site_index);
      r_cm.add(site.position());
    }
    r_cm.divide(selection.num_sites());
    double rg = 0.;
    for (int site_index : selection.site_indices(select_index)) {
      const Site& site = part.site(site_index);
      rg += site.position().squared_distance(r_cm);
    }
    const double rgn = rg/selection.num_sites();
    accumulator_.accumulate(rgn);
    const double en = criteria.current_energy();
    rg_e_.accumulate(rgn*en);
    rg_e2_.accumulate(rgn*en*en);
  }
}

std::string RadiusOfGyration::write(const Criteria& criteria,
    const System& system,
    const TrialFactory& trial_factory) {
  std::stringstream ss;
  if (rewrite_header()) {
    ss << header(criteria, system, trial_factory);
  }
  ss << accumulator_.status() << "," << rg_e_.average();
  ss << "," << rg_e2_.average();
  ss << std::endl;
  DEBUG(ss.str());
  return ss.str();
}

void RadiusOfGyration::serialize(std::ostream& ostr) const {
  Stepper::serialize(ostr);
  feasst_serialize_version(7685, ostr);
  feasst_serialize(group_index_, ostr);
  feasst_serialize_fstobj(rg_e_, ostr);
  feasst_serialize_fstobj(rg_e2_, ostr);
}

RadiusOfGyration::RadiusOfGyration(std::istream& istr) : Analyze(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 7685, "mismatch version:" << version);
  feasst_deserialize(&group_index_, istr);
  feasst_deserialize_fstobj(&rg_e_, istr);
  feasst_deserialize_fstobj(&rg_e2_, istr);
}

RadiusOfGyration::RadiusOfGyration(const Analyze& energy) {
  std::stringstream ss;
  energy.serialize(ss);
  *this = RadiusOfGyration(ss);
}

}  // namespace feasst
