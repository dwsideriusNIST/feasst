
#ifndef FEASST_MONTE_CARLO_WRITE_STEPPER_H_
#define FEASST_MONTE_CARLO_WRITE_STEPPER_H_

#include <string>
#include <memory>
#include "utils/include/arguments.h"
#include "monte_carlo/include/action.h"

namespace feasst {

/**
  An Analyze/Modify of given name is forced to write to file.
  This is convenient for example printing the very last update to FlatHistogram.
 */
class WriteStepper : public Action {
 public:
  //@{
  /** @name Arguments
    - analyze_name: The Analyze::class_name() to print to file.
      If empty, do nothing (default: empty).
    - modify_name: The Modify::class_name() to print to file (default: empty).
      If empty, do nothing (default: empty).
   */
  explicit WriteStepper(argtype args = argtype());
  explicit WriteStepper(argtype * args);

  //@}
  /** @name Public Functions
   */
  //@{

  void run(MonteCarlo * mc) override;
  std::shared_ptr<Action> create(std::istream& istr) const override {
    return std::make_shared<WriteStepper>(istr); }
  std::shared_ptr<Action> create(argtype * args) const override {
    return std::make_shared<WriteStepper>(args); }
  void serialize(std::ostream& ostr) const override;
  explicit WriteStepper(std::istream& istr);
  virtual ~WriteStepper() {}

  //@}
 private:
  std::string analyze_name_;
  std::string modify_name_;
};

inline std::shared_ptr<WriteStepper> MakeWriteStepper(argtype args = argtype()) {
  return std::make_shared<WriteStepper>(args);
}

}  // namespace feasst

#endif  // FEASST_MONTE_CARLO_WRITE_STEPPER_H_
