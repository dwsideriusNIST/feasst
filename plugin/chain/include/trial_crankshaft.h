
#ifndef FEASST_CHAIN_TRIAL_CRANKSHAFT_H_
#define FEASST_CHAIN_TRIAL_CRANKSHAFT_H_

#include <string>
#include <memory>
#include "monte_carlo/include/trial_move.h"

namespace feasst {

class TrialCrankshaft : public TrialMove {
 public:
  TrialCrankshaft(
    /// These arguments are sent to both PerturbCrankshaft and TrialStage.
    const argtype& args = argtype());

  std::shared_ptr<Trial> create(std::istream& istr) const override;
  void serialize(std::ostream& ostr) const override;
  explicit TrialCrankshaft(std::istream& istr);
  virtual ~TrialCrankshaft() {}
};

inline std::shared_ptr<TrialCrankshaft> MakeTrialCrankshaft(
    const argtype &args = argtype()) {
  return std::make_shared<TrialCrankshaft>(args);
}

}  // namespace feasst

#endif  // FEASST_CHAIN_TRIAL_CRANKSHAFT_H_
