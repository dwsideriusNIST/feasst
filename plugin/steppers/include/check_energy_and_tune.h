
#ifndef FEASST_STEPPERS_CHECK_ENERGY_AND_TUNE_H_
#define FEASST_STEPPERS_CHECK_ENERGY_AND_TUNE_H_

#include <memory>
#include "utils/include/arguments.h"
#include "monte_carlo/include/modify_factory.h"

namespace feasst {

/// Combine CheckEnergy and Tuner.
class CheckEnergyAndTune : public ModifyFactory {
 public:
  explicit CheckEnergyAndTune(const argtype& args = argtype());
};

inline std::shared_ptr<CheckEnergyAndTune> MakeCheckEnergyAndTune(
    const argtype &args = argtype()) {
  return std::make_shared<CheckEnergyAndTune>(args);
}

}  // namespace feasst

#endif  // FEASST_STEPPERS_CHECK_ENERGY_AND_TUNE_H_
