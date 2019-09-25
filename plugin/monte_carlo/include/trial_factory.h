
#ifndef FEASST_MONTE_CARLO_TRIAL_FACTORY_H_
#define FEASST_MONTE_CARLO_TRIAL_FACTORY_H_

#include <sstream>
#include <memory>
#include "monte_carlo/include/trial.h"
//#include "utils/include/timer.h"

namespace feasst {

class TrialFactory : public Trial {
 public:
  TrialFactory() { class_name_ = "TrialFactory"; }

  /// Add a trial.
  void add(std::shared_ptr<Trial> trial) {
    trials_.push_back(trial);

    // update probability of selection
    std::vector<double> weights;
    for (std::shared_ptr<Trial> trial : trials_) {
      weights.push_back(trial->weight());
    }
    cumulative_probability_ = cumulative_probability(weights);
    //std::stringstream ss;
    //ss << trials_.back()->class_name()"trial" << num_trials() - 1;
    // timer_.add(trials_.back()->class_name());
  }

  /// Return the number of trials.
  int num_trials() const { return static_cast<int>(trials_.size()); }

  // HWH depreciate
  std::vector<std::shared_ptr<Trial> > trials() { return trials_; }

  /// Return a trial by index of the order trials were added.
  const Trial * trial(const int index) const { return trials_[index].get(); }

  /// Return the index of a trial selected with probability proportional to its
  /// weight.
  int random_index(Random * random) {
    ASSERT(num_trials() > 0, "no trials to select");
    return random->index_from_cumulative_probability(cumulative_probability_);
  }

  /// Attempt one of the trials. Return true if accepted.
  bool attempt(
      Criteria* criteria,
      System * system,
      /// attempt trial_index. If -1, choose randomly with probabilty
      /// determined from the weight.
      const int trial_index,
      Random * random) {
    // timer_.start(0);
    if (num_trials() == 0) return false;
    if (trial_index != -1) {
      return attempt_(criteria, system, trial_index, random);
    }
    return attempt_(criteria, system, random_index(random), random);
  }

  /// Attempt one of the trials with selection probability proportional to
  /// the weight.
  bool attempt(Criteria* criteria, System * system, Random * random) override {
    return attempt(criteria, system, -1, random); }

  /// Revert changes to system by trial index.
  void revert(const int index, const bool accepted, System * system) {
    trials_[index]->revert(index, accepted, system); }

  void mimic_trial_rejection(const int index) {
    INFO("index " << index << " " << trials_.size());
    trials_[index]->increment_num_attempts();
  }

  /// Return the header description for the statuses of the trials (e.g., acceptance, etc).
  std::string status_header() const override {
    std::stringstream ss;
    ss << "attempt ";
    for (const std::shared_ptr<Trial> trial : trials_) {
      ss << trial->status_header() << " ";
    }
    return ss.str();
  }

  /// Return the statuses of the trials (e.g., acceptance, etc).
  std::string status() const override {
    std::stringstream ss;
    ss << num_attempts() << " ";
    for (const std::shared_ptr<Trial> trial : trials_) {
      ss << trial->status() << " ";
    }
    return ss.str();
  }

  void reset_stats() override {
    Trial::reset_stats();
    for (std::shared_ptr<Trial> trial : trials_) {
      trial->reset_stats();
    }
  }

  void tune() override {
    for (std::shared_ptr<Trial> trial : trials_) {
      trial->tune();
    }
  }

  void precompute(Criteria * criteria, System * system) override {
    for (std::shared_ptr<Trial> trial : trials_) {
      trial->precompute(criteria, system);
    }
  }

  int64_t num_attempts() const override {
    int64_t num = 0;
    for (std::shared_ptr<Trial> trial : trials_) {
      num += trial->num_attempts();
    }
    return num;
  }

//  std::string class_name() const override { return std::string("TrialFactory"); }

//  const Timer& timer() const { return timer_; }

 private:
  std::vector<std::shared_ptr<Trial> > trials_;
  std::vector<double> cumulative_probability_;
//  Timer timer_;

  bool attempt_(
      Criteria* criteria,
      System * system,
      const int index,
      Random * random) {
    //timer_.start(index + 1);  // +1 for "other"
    return trials_[index]->attempt(criteria, system, random);
    //timer_.end();
  }
};

}  // namespace feasst

#endif  // FEASST_MONTE_CARLO_TRIAL_FACTORY_H_