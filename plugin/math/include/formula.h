
#ifndef FEASST_MATH_FORMULA_H_
#define FEASST_MATH_FORMULA_H_

#include <map>
#include <memory>
#include <string>
#include <sstream>
#include "utils/include/arguments.h"

namespace feasst {

/**
  Define a formula that depends on one or more variables, x or y.
 */
class Formula {
 public:
  /**
    args:
    - x0: central reference position (default: 0).
   */
  Formula(const argtype& args = argtype());
  void set_x0(const double x0) { x0_ = x0; }
  double x0() const { return x0_; }
  virtual double evaluate(const double x) const;
  virtual double evaluate(const double x, const double y) const;
  virtual double derivative(const double x) const;

  // serialization
  virtual std::string class_name() const { return class_name_; }
  virtual void serialize(std::ostream& ostr) const;
  virtual std::shared_ptr<Formula> create(std::istream& istr) const;
  std::map<std::string, std::shared_ptr<Formula> >& deserialize_map();
  std::shared_ptr<Formula> deserialize(std::istream& istr);
  explicit Formula(std::istream& istr);
  virtual ~Formula() {}

 protected:
  std::string class_name_ = "Formula";
  void serialize_formula_(std::ostream& ostr) const;
  Arguments args_;

 private:
  double x0_;
};

}  // namespace feasst

#endif  // FEASST_MATH_FORMULA_H_
