
#ifndef FEASST_PREFETCH_PREFETCH_H_
#define FEASST_PREFETCH_PREFETCH_H_

#include <string>
#include <vector>
#include <memory>
#include "utils/include/arguments.h"
#include "monte_carlo/include/monte_carlo.h"

namespace feasst {

/*
  Store clones in Pool.
  Arbitrarily assign thread=0 to main/master.
  Thus, don't access mc of thread 0.
*/

class Pool {
 public:
  void set_index(const int index) {
    DEBUG("index " << index);
    index_ = index; }
  int index() const { return index_; }
  void set_ln_prob(const double ln_prob) { ln_prob_ = ln_prob; }
  double ln_prob() const { return ln_prob_; }
  void set_accepted(const bool accepted) { accepted_ = accepted; }
  bool accepted() const { return accepted_; }

  const std::string str() const {
    std::stringstream ss;
    ss << index_ << " " << ln_prob_ << " " << accepted_;
    return ss.str();
  }

  MonteCarlo mc;

 private:
  int index_;
  double ln_prob_;
  bool accepted_;
};

/**
  Farm a trial to each processor, then reconstruct the serial Markov chain.
 */
class Prefetch : public MonteCarlo {
 public:
  Prefetch(
    /**
      steps_per_check : number of steps between check (default: 1e5)
     */
    const argtype& args = argtype()) {
    activate_prefetch();
    Arguments args_(args);
    steps_per_check_ = args_.key("steps_per_check").dflt("100000").integer();
  }

  void activate_prefetch(const bool active = true) { is_activated_ = active; }
  void reset_trial_stats() override {
    MonteCarlo::reset_trial_stats();
    for (Pool& pool : pool_) {
      pool.mc.reset_trial_stats();
    }
  }

  void serialize(std::ostream& ostr) const override { ERROR("not implemented"); }
  Prefetch(std::istream& istr) : MonteCarlo(istr) {}

  virtual ~Prefetch() {}

 protected:
  void attempt_(const int num_trials, TrialFactory * trial_factory, Random * random) override;

 private:
  // void distribute_for_();

  // Distribute pool of trial(s) to each processor
  // use a omp parallel while constructed as described in
  // https://cvw.cac.cornell.edu/OpenMP/whileloop
//  void distribute_while_();

  int num_threads_;
  bool is_activated_;
  int steps_per_check_;

  std::vector<Pool> pool_;

  // Pick a clone based on ithread
  MonteCarlo * clone_(const int ithread);
};

inline std::shared_ptr<Prefetch> MakePrefetch(const argtype& args = argtype()) {
  return std::make_shared<Prefetch>(args);
}

}  // namespace feasst

#endif  // FEASST_PREFETCH_PREFETCH_H_
