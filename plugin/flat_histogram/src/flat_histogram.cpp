#include <cmath>
#include "utils/include/serialize.h"
#include "math/include/constants.h"
#include "math/include/random.h"
#include "flat_histogram/include/flat_histogram.h"
#include "flat_histogram/include/wang_landau.h"
#include "flat_histogram/include/transition_matrix.h"
#include "flat_histogram/include/wltm.h"
#include "flat_histogram/include/macrostate_energy.h"
#include "flat_histogram/include/wang_landau.h"

namespace feasst {

void FlatHistogram::init_(std::shared_ptr<Macrostate> macrostate,
    std::shared_ptr<Bias> bias) {
  macrostate_ = macrostate;
  bias_ = bias;
  bias_->resize(macrostate_->histogram());
}

FlatHistogram::FlatHistogram() : Criteria() {
  class_name_ = "FlatHistogram";
}
FlatHistogram::FlatHistogram(std::shared_ptr<Macrostate> macrostate,
    std::shared_ptr<Bias> bias)
  : FlatHistogram() {
  init_(macrostate, bias);
}
FlatHistogram::FlatHistogram(argtype * args) {
  ASSERT(!used("num_iterations_to_complete", *args),
    "FlatHistogram does not use the argument num_iterations_to_complete");
  init_(MacrostateEnergy().factory(str("Macrostate", args), args),
        MakeWangLandau({{"min_flatness", "1"}})->factory(str("Bias", args), args));
}
FlatHistogram::FlatHistogram(argtype args) : FlatHistogram(&args) {
  check_all_used(args);
}

FlatHistogram::FlatHistogram(std::shared_ptr<Macrostate> macrostate,
    std::shared_ptr<Bias> bias,
    std::shared_ptr<Constraint> constraint)
  : FlatHistogram(macrostate, bias) {
  add(constraint);
}

bool FlatHistogram::is_accepted(
    const System& system,
    Acceptance * acceptance,
    Random * random) {
  ASSERT(bias_ != NULL, "bias must be initialized before trials");
  bool is_accepted;
  double ln_metropolis_prob = acceptance->ln_metropolis_prob();
  DEBUG("macroshift " << acceptance->macrostate_shift());
//  const int shift = acceptance->macrostate_shift()*num_trial_states();
  bool is_allowed_ = true;
  if (acceptance->reject() ||
      !is_allowed(system, *acceptance) ) {
    is_accepted = false;
    ln_metropolis_prob = -NEAR_INFINITY;
    macrostate_new_ = macrostate_old_;
    DEBUG("forced rejection");
  } else {
    // the shift factory multiplied by number of states assumes only one
    // particle is added during an entire growth expanded cycle
    //macrostate_new_ = macrostate_->bin(system, this) + shift;
    macrostate_new_ = macrostate_->bin(system, *this, *acceptance);
    DEBUG("old " << macrostate_old_ << " new " << macrostate_new_);
    DEBUG("bias " << bias_->ln_bias(macrostate_new_, macrostate_old_));
    DEBUG("ln new " << bias_->ln_prob().value(macrostate_new_));
    DEBUG("ln old " << bias_->ln_prob().value(macrostate_old_));
    DEBUG("ln met " << ln_metropolis_prob);
    DEBUG("ln tot " << ln_metropolis_prob + bias_->ln_bias(macrostate_new_, macrostate_old_));
    is_allowed_ = macrostate_->is_allowed(system, *this, *acceptance);
    if (is_allowed_ &&
        random->uniform() < exp(ln_metropolis_prob +
                                bias_->ln_bias(macrostate_new_,
                                               macrostate_old_))) {
      is_accepted = true;
      DEBUG("accept");
    } else {
      is_accepted = false;
      DEBUG("reject");
    }
    DEBUG("macro old new " << macrostate_old_ << " " << macrostate_new_);
  }
  bias_->update(macrostate_old_,
                macrostate_new_,
                ln_metropolis_prob,
                is_accepted,
                is_allowed_);
  if (is_accepted) {
    set_current_energy(acceptance->energy_new());
    DEBUG("current energy: " << current_energy());
    macrostate_current_ = macrostate_new_;
  } else {
    // return the macrostate to the current value, as used by Analyze, etc.
    macrostate_current_ = macrostate_old_;
  }
  was_accepted_ = is_accepted;
  acceptance->set_allowed(is_allowed_);
  return is_accepted;
}

std::string FlatHistogram::write() const {
  std::stringstream ss;
  ss << Criteria::write();
  ss << bias_->write();
  ss << "state,"
     << bias_->write_per_bin_header()
     << std::endl;
  const Histogram& hist = macrostate_->histogram();
  for (int bin = 0; bin < hist.size(); ++bin) {
    ss << hist.center_of_bin(bin) << ","
       << bias_->write_per_bin(bin)
       << std::endl;
  }
  return ss.str();
}

void FlatHistogram::revert_(const bool accepted, const bool allowed, const double ln_prob) {
  Criteria::revert_(accepted, allowed, ln_prob);
  bias_->update_or_revert(macrostate_old_,
                          macrostate_new_,
                          ln_prob,
                          accepted,
                          allowed,
                          true);
  macrostate_new_ = macrostate_old_;
}

class MapFlatHistogram {
 public:
  MapFlatHistogram() {
    FlatHistogram().deserialize_map()["FlatHistogram"] =
      MakeFlatHistogram();
  }
};

static MapFlatHistogram mapper_ = MapFlatHistogram();

FlatHistogram::FlatHistogram(std::istream& istr)
  : Criteria(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 937, "version mismatch: " << version);
  // feasst_deserialize_fstdr(bias_, istr);
  { // HWH for unknown reasons the above template function does not work
    int existing;
    istr >> existing;
    if (existing != 0) {
      bias_ = bias_->deserialize(istr);
    }
  }
  // feasst_deserialize_fstdr(macrostate_, istr);
  { // HWH for unknown reasons the above template function does not work
    int existing;
    istr >> existing;
    if (existing != 0) {
      macrostate_ = macrostate_->deserialize(istr);
    }
  }
  feasst_deserialize(&macrostate_old_, istr);
  feasst_deserialize(&macrostate_new_, istr);
  feasst_deserialize(&macrostate_current_, istr);
  feasst_deserialize(&is_macrostate_set_, istr);
}

void FlatHistogram::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  serialize_criteria_(ostr);
  feasst_serialize_version(937, ostr);
  feasst_serialize_fstdr(bias_, ostr);
  feasst_serialize_fstdr(macrostate_, ostr);
  feasst_serialize(macrostate_old_, ostr);
  feasst_serialize(macrostate_new_, ostr);
  feasst_serialize(macrostate_current_, ostr);
  feasst_serialize(is_macrostate_set_, ostr);
}

void FlatHistogram::before_attempt(const System& system) {
  macrostate_old_ = macrostate_->bin(system, *this, empty_);
  DEBUG("macro old " << macrostate_old_);
  ASSERT(macrostate_old_ >= macrostate_->soft_min() &&
         macrostate_old_ <= macrostate_->soft_max(),
    "macrostate: " << macrostate_old_ << " is not in range from " <<
    macrostate_->soft_min() << " to " << macrostate_->soft_max());
}

bool FlatHistogram::is_fh_equal(const FlatHistogram& flat_histogram,
    const double tolerance) const {
  if (!Criteria::is_equal(flat_histogram, tolerance)) {
      return false;
  }
  if (macrostate_old_ != flat_histogram.macrostate_old_) return false;
  if (macrostate_new_ != flat_histogram.macrostate_new_) return false;
  if (macrostate_current_ != flat_histogram.macrostate_current_) return false;
  return true;
}

FlatHistogram::FlatHistogram(const Criteria& criteria) {
  std::stringstream ss;
  criteria.serialize(ss);
  *this = FlatHistogram(ss);
}

int FlatHistogram::set_soft_max(const int index, const System& sys) {
  return macrostate_->set_soft_max(index, sys, *this);
}

int FlatHistogram::set_soft_min(const int index, const System& sys) {
  return macrostate_->set_soft_min(index, sys, *this);
}

void FlatHistogram::set_cm(const bool inc_max, const int macro, const Criteria& crit) {
  if (inc_max) {
    macrostate_->add_to_soft_max(1);
  } else {
    macrostate_->remove_from_soft_min(1);
  }
  bias_->set_cm(macro, crit.bias());
}

void FlatHistogram::adjust_bounds(const bool left_most, const bool right_most,
  const int min_size, const System& system, const System& upper_sys,
  Criteria * criteria) {
  DEBUG("left_most " << left_most);
  DEBUG("right_most " << right_most);
  const int lower_max = macrostate().soft_max();
  const int upper_min = criteria->macrostate().soft_min();
  if (num_iterations() > criteria->num_iterations()) {
    if (criteria->macrostate().soft_max() - upper_min > min_size) {
      DEBUG("move macrostate from upper to lower");
      if (criteria->set_soft_min(upper_min + 1, upper_sys) > 0) {
        set_cm(true, upper_min, *criteria);
      }
    }
  } else if (num_iterations() < criteria->num_iterations()) {
    if (lower_max - macrostate().soft_min() > min_size) {
      DEBUG("move macrostate from lower to upper");
      if (set_soft_max(lower_max - 1, system) > 0) {
        criteria->set_cm(false, lower_max, *this);
      }
    }
  }

  // HWH this only works if using new sweeps definition in TM
  if (left_most) {
    // consider increasing the soft_min if the existing visits are
    // enough for completion
    if (bias().visits(macrostate().soft_min()) >= num_iterations_to_complete()) {
      set_soft_min(macrostate().soft_min() + 1, system);
    }
  }
  if (right_most) {
    // consider decreasing the soft_max if the existing visits are
    // enough for completion
    if (criteria->bias().visits(criteria->macrostate().soft_max()) >=
        criteria->num_iterations_to_complete()) {
      set_soft_max(criteria->macrostate().soft_max() - 1, upper_sys);
    }
  }
  INFO(macrostate().soft_min() << " " << criteria->macrostate().soft_max());
}

}  // namespace feasst
