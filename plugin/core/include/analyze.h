
#ifndef FEASST_CORE_ANALYZE_H_
#define FEASST_CORE_ANALYZE_H_

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include "core/include/trial_factory.h"
#include "core/include/trial_transfer.h"
#include "core/include/bond_visitor.h"
#include "core/include/file_xyz.h"
#include "core/include/stepper.h"
#include "core/include/arguments.h"

namespace feasst {

class Analyze : public Stepper {
 public:
  Analyze(const argtype &args = argtype()) : Stepper(args) {}

  virtual void initialize(const std::shared_ptr<Criteria> criteria,
      const System& system,
      const TrialFactory& trial_factory) {
    // do nothing by default
  }

  virtual void trial(const std::shared_ptr<Criteria> criteria,
      const System& system,
      const TrialFactory& trial_factory);

  virtual void update(const std::shared_ptr<Criteria> criteria,
      const System& system,
      const TrialFactory& trial_factory);

  virtual std::string write(const std::shared_ptr<Criteria> criteria,
      const System& system,
      const TrialFactory& trial_factory);

  virtual void serialize(std::ostream& ostr) const;
  virtual std::shared_ptr<Analyze> create(std::istream& istr) const;
  std::map<std::string, std::shared_ptr<Analyze> >& deserialize_map();
  std::shared_ptr<Analyze> deserialize(std::istream& istr);
  virtual ~Analyze() {}
};

class AnalyzeWriteOnly : public Analyze {
 public:
  AnalyzeWriteOnly(
    /**
      steps_per : write every this many steps
     */
    const argtype &args = argtype()) : Analyze(args) {
    // disable update
    Analyze::set_steps_per_update(-1);

    // parse
    if (!args_.key("steps_per").empty()) {
      set_steps_per(args_.integer());
    }
  }

  void set_steps_per_update(const int steps) override {
    ERROR("This analyze is write only."); }

  void set_steps_per(const int steps) { set_steps_per_write(steps); }

  std::shared_ptr<Analyze> create(std::istream& istr) const override;
  void serialize(std::ostream& ostr) const override;

 private:
  const std::string class_name_ = "AnalyzeWriteOnly";
};

class AnalyzeUpdateOnly : public Analyze {
 public:
  AnalyzeUpdateOnly(
    /**
      steps_per : update every this many steps
     */
    const argtype &args = argtype()) : Analyze(args) {
    // disable update
    Analyze::set_steps_per_write(-1);

    // parse
    if (!args_.key("steps_per").empty()) {
      set_steps_per(args_.integer());
    }
  }

  void set_steps_per_write(const int steps) override {
    ERROR("This analyze is update only."); }

  void set_steps_per(const int steps) { set_steps_per_update(steps); }

  std::shared_ptr<Analyze> create(std::istream& istr) const override;
  void serialize(std::ostream& ostr) const override;

 private:
  const std::string class_name_ = "AnalyzeUpdateOnly";
};

}  // namespace feasst

#endif  // FEASST_CORE_ANALYZE_H_
