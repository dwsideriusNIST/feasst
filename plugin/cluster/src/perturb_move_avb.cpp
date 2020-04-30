#include "utils/include/serialize.h"
#include "math/include/random.h"
#include "cluster/include/perturb_move_avb.h"

namespace feasst {

PerturbMoveAVB::PerturbMoveAVB(
    std::shared_ptr<NeighborCriteria> neighbor_criteria,
    const argtype& args) : PerturbMove(args) {
  class_name_ = "PerturbMoveAVB";
  neighbor_criteria_ = neighbor_criteria;
  rotate_.set_tunable(180.);
  rotate_.disable_tunable_();
  disable_tunable_();
  inside_ = args_.key("inside").dflt("true").boolean();
}

class MapPerturbMoveAVB {
 public:
  MapPerturbMoveAVB() {
    auto obj = MakePerturbMoveAVB(MakeNeighborCriteria());
    obj->deserialize_map()["PerturbMoveAVB"] = obj;
  }
};

static MapPerturbMoveAVB mapper_ = MapPerturbMoveAVB();

std::shared_ptr<Perturb> PerturbMoveAVB::create(std::istream& istr) const {
  return std::make_shared<PerturbMoveAVB>(istr);
}

void PerturbMoveAVB::move(
    System * system,
    TrialSelect * select,
    Random * random) {
  DEBUG(select->mobile().str());
  const Configuration& config = system->configuration();

  // give particle random orientation
  if (select->mobile().num_sites() > 1) {
    rotate_.move(system, select, random);
  }


  if (displace_.dimension() == 0) {
    displace_.set_to_origin(config.dimension());
  }
  const Position& anchor_pos = config.select_particle(
    select->anchor().particle_index(0)).site(
    select->anchor().site_index(0, 0)).position();

  DEBUG("inside " << inside_);
  if (inside_) {
    // translate mobile particle to AV of anchor
    random->position_in_spherical_shell(
      neighbor_criteria_->minimum_distance(),
      neighbor_criteria_->maximum_distance(),
      &displace_);
  } else {
    // pick a random position in the domain but outside the AV, considering PBC
    const double volume_av = neighbor_criteria_->volume(config.dimension());
    ASSERT(config.domain()->volume() > volume_av, "AV: " << volume_av
      << " too large for domain");
    int attempts = 0;
    bool found = false;
    while (!found) {
      config.domain()->random_position(&displace_, random);
      displace_.subtract(anchor_pos);
      found = !neighbor_criteria_->is_position_accepted(displace_,
                                                        config.domain());
      ++attempts;
      ASSERT(attempts < 1e5, "max attempts");
    }
  }
  DEBUG("displace: " << displace_.str());
  // move from anchor_pos reference to mobile reference
  displace_.add(anchor_pos);
  DEBUG("mobile: " << select->mobile().str());
  const Position& mobile_pos = config.select_particle(
    select->mobile().particle_index(0)).site(
    select->mobile().site_index(0, 0)).position();
  DEBUG("mobile_pos " << mobile_pos.str());
  displace_.subtract(mobile_pos);
//  displace_.subtract(config.select_particle(select->mobile().particle_index(0)).site(
//    select->mobile().site_index(0, 0)).position());
  DEBUG("anchor_pos " << anchor_pos.str());
  DEBUG("displace " << displace_.str());
  translate_.move(displace_, system, select);
}

PerturbMoveAVB::PerturbMoveAVB(std::istream& istr)
  : PerturbMove(istr) {
  ASSERT(class_name_ == "PerturbMoveAVB", "name: " << class_name_);
  const int version = feasst_deserialize_version(istr);
  ASSERT(5937 == version, "mismatch version: " << version);
  // HWH for unknown reasons, this function template does not work
  // feasst_deserialize(neighbor_criteria_, istr);
  { int existing;
    istr >> existing;
    if (existing != 0) {
      neighbor_criteria_ = std::make_shared<NeighborCriteria>(istr);
    }
  }
  feasst_deserialize_fstobj(&rotate_, istr);
  feasst_deserialize(&inside_, istr);
}

void PerturbMoveAVB::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  serialize_perturb_(ostr);
  feasst_serialize_version(5937, ostr);
  feasst_serialize(neighbor_criteria_, ostr);
  feasst_serialize_fstobj(rotate_, ostr);
  feasst_serialize(inside_, ostr);
}

}  // namespace feasst
