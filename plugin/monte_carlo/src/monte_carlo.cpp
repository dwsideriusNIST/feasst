#include "utils/include/serialize.h"
#include "utils/include/checkpoint.h"
#include "math/include/random_mt19937.h"
#include "monte_carlo/include/monte_carlo.h"
#include "monte_carlo/include/trial_add.h"

namespace feasst {

MonteCarlo::MonteCarlo() {
  set(std::make_shared<RandomMT19937>());
//    timer_other_ = timer_.add("other");
//    timer_trial_ = timer_.add("trial");
//    timer_analyze_ = timer_.add("analyze");
//    timer_modify_ = timer_.add("modify");
//    timer_checkpoint_ = timer_.add("checkpoint");
}

void MonteCarlo::seed_random(const int seed) {
  random_->seed(seed);
}

void MonteCarlo::add(const Configuration& config) {
  system_.add(config);
  config_set_ = true;
  if (potential_set_) system_set_ = true;
  ASSERT(!criteria_set_, "add config before criteria");
}

void MonteCarlo::add(const Potential& potential) {
  ASSERT(!criteria_set_, "add potential before criteria");
  ASSERT(config_set_ || system_set_, "config:" << config_set_ <<
    " or system:" << system_set_ << " must be set before adding a potential");
  system_.add(potential);
  system_.precompute();
  potential_set_ = true;
  if (config_set_) system_set_ = true;
}

void MonteCarlo::set(const int index, const Potential& potential) {
  // ASSERT(!criteria_set_, "add potential before criteria");
  ASSERT(potential_set_ || system_set_, "add potential before setting one");
  system_.set_unoptimized(index, potential);
  system_.precompute();
}

void MonteCarlo::set(const System& system) {
  system_set_ = true;
  system_ = system;
  system_.precompute();
  // ASSERT(!criteria_set_, "add system before criteria");
  // HWH used in clones.cpp to transfer configurations
}

void MonteCarlo::set(std::shared_ptr<Criteria> criteria) {
  ASSERT(system_set_, "set System before Criteria.");
  criteria_ = criteria;
  criteria_set_ = true;
  initialize_energy();
  // criteria->set_current_energy(system_.unoptimized_energy());
  DEBUG("current energy: " << criteria->current_energy());
}

void MonteCarlo::add(std::shared_ptr<Trial> trial) {
  ASSERT(criteria_set_, "set Criteria before Trials.");

  // Require the use of reference potentials for multi-stage trials with Ewald.
  if (trial->num_stages() > 1) {
    if (trial->stage(0).reference() == -1) {
      for (const Potential& pot : system_.potentials().potentials()) {
        if (pot.visit_model().class_name() == "Ewald") {
          ERROR(trial->class_name() << " should use a reference potential "
            << "without Ewald due to multiple stages which complicate revert");
        }
      }
    }
  }

  trial->precompute(criteria_.get(), &system_);
  trial_factory_.add(trial);

  // HWH depreciate?
  // If later, perhaps after some initialization, more trials are added,
  // then Analyze and Modify classes may need to be re-initialized.
  // analyze_factory_.initialize(criteria_, system_, trial_factory_);
  // modify_factory_.initialize(criteria_, &system_, &trial_factory_);
}

void MonteCarlo::add(std::shared_ptr<Analyze> analyze) {
  ASSERT(criteria_set_, "set Criteria before Analyze");
  DEBUG("class name? " << analyze->class_name());
  if (analyze->is_multistate() and analyze->class_name() != "AnalyzeFactory") {
    auto multi = MakeAnalyzeFactory({{"multistate", "true"}});
    DEBUG("making multi " << multi->is_multistate());
    for (int state = 0; state < criteria_->num_states(); ++state) {
      DEBUG("state: " << state);
      std::shared_ptr<Analyze> an = deep_copy_derived(analyze);
      { std::stringstream ss;
        an->serialize(ss);
        DEBUG(ss.str());
      }
      an->set_state(state);
      // an->initialize(criteria_, system_, trial_factory_);
      multi->add(an);
    }
    analyze = multi;
  }
  analyze->initialize(criteria_.get(), &system_, &trial_factory_);
  DEBUG("mults " << analyze->is_multistate() << " class name? " << analyze->class_name());
  analyze_factory_.add(analyze);
}

void MonteCarlo::add(const std::shared_ptr<Modify> modify) {
  ASSERT(criteria_set_, "set Criteria before Modify");
  modify->initialize(criteria_.get(), &system_, &trial_factory_);
  modify_factory_.add(modify);
}

//void MonteCarlo::seek_num_particles(const int num) {
//  ASSERT(system_.configuration().num_particles() <= num,
//    "assumes you only want to add particles, not delete them");
//  auto add = MakeTrialAdd({{"particle_type", "0"}});
//  add->precompute(criteria_.get(), &system_);
//  while (system_.configuration().num_particles() < num) {
//    attempt();
//    add->attempt(criteria_.get(), &system_, random_.get());
//  }
//  reset_trial_stats();
//}

void MonteCarlo::set(const std::shared_ptr<Checkpoint> checkpoint) {
  checkpoint_ = checkpoint;
}

void MonteCarlo::after_trial_modify_() {
  modify_factory_.trial(criteria_.get(), &system_, &trial_factory_);
  if (checkpoint_) {
    checkpoint_->check(*this);
  }
}

void MonteCarlo::serialize(std::ostream& ostr) const {
  feasst_serialize_version(529, ostr);
  feasst_serialize_fstobj(system_, ostr);
  feasst_serialize_fstdr(criteria_, ostr);
  feasst_serialize_fstobj(trial_factory_, ostr);
  feasst_serialize_fstobj(analyze_factory_, ostr);
  feasst_serialize_fstobj(modify_factory_, ostr);
  feasst_serialize(checkpoint_, ostr);
  feasst_serialize_fstdr(random_, ostr);
  feasst_serialize(config_set_, ostr);
  feasst_serialize(potential_set_, ostr);
  feasst_serialize(system_set_, ostr);
  feasst_serialize(criteria_set_, ostr);
}

MonteCarlo::MonteCarlo(std::istream& istr) {
  //INFO(istr.rdbuf());
  //int tmp;
  //istr >> tmp;
  //INFO("tmp " << tmp);
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 529, "version: " << version);
  feasst_deserialize_fstobj(&system_, istr);
  // feasst_deserialize_fstdr(criteria_, istr);
  { // HWH for unknown reasons the above template function does not work
    int existing;
    istr >> existing;
    if (existing != 0) {
      criteria_ = criteria_->deserialize(istr);
    }
  }
  feasst_deserialize_fstobj(&trial_factory_, istr);
  feasst_deserialize_fstobj(&analyze_factory_, istr);
  feasst_deserialize_fstobj(&modify_factory_, istr);
  // HWH for unknown reasons, this function template does not work.
  //feasst_deserialize(checkpoint_, istr);
  { int existing;
    istr >> existing;
    if (existing != 0) {
      checkpoint_ = std::make_shared<Checkpoint>(istr);
    }
  }
  // HWH for unknown reasons, this function template does not work.
  //feasst_deserialize_fstdr(random_, istr);
  { int existing;
    istr >> existing;
    if (existing != 0) {
      random_ = random_->deserialize(istr);
    }
  }
  feasst_deserialize(&config_set_, istr);
  feasst_deserialize(&potential_set_, istr);
  feasst_deserialize(&system_set_, istr);
  feasst_deserialize(&criteria_set_, istr);
}

void MonteCarlo::load_cache_(const bool load) {
  random_->set_cache_to_load(load);
  system_.load_cache(load);
}

void MonteCarlo::unload_cache_(const MonteCarlo& mc) {
  random_->set_cache_to_unload((*mc.random_));
  system_.unload_cache(mc.system());
}

void MonteCarlo::before_attempts_() {
  ASSERT(system_set_, "system must be set before attempting trials.");
  ASSERT(criteria_set_, "criteria must be set before attempting trials.");
}

void MonteCarlo::revert_(const int trial_index,
    const bool accepted,
    const double ln_prob) {
  trial_factory_.revert(trial_index, accepted, &system_);
  DEBUG("reverting " << criteria_->current_energy());
  criteria_->revert_(accepted, ln_prob);
}

void MonteCarlo::attempt_(int num_trials,
    TrialFactory * trial_factory,
    Random * random) {
  before_attempts_();
  for (int trial = 0; trial < num_trials; ++trial) {
    DEBUG("mc trial: " << trial);
    trial_factory->attempt(criteria_.get(), &system_, random);
    after_trial_analyze_();
    after_trial_modify_();
  }
}

bool MonteCarlo::attempt_trial(const int index) {
  return trial_factory_.attempt(criteria_.get(), &system_,
                                index, random_.get());
}

void MonteCarlo::imitate_trial_rejection_(const int trial_index,
    const double ln_prob,
    const int state_old,
    const int state_new) {
  trial_factory_.imitate_trial_rejection_(trial_index);
  criteria_->imitate_trial_rejection_(ln_prob, state_old, state_new);
}

void MonteCarlo::initialize_energy() {
  criteria_->set_current_energy(system_.unoptimized_energy());
  system_.energy();
  for (int ref = 0; ref < system_.num_references(); ++ref) {
    system_.reference_energy(ref);
  }
}

}  // namespace feasst
