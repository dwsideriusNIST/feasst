
#ifndef FEASST_CONFINEMENT_SLAB_H_
#define FEASST_CONFINEMENT_SLAB_H_

#include "confinement/include/shape.h"
#include "utils/include/arguments.h"

namespace feasst {

/**
  A slab is the intersection of two half spaces.
 */
class Slab : public Shape {
 public:
  Slab(
    /**
      dimension : The slab surface is perpendicular to this dimensional axis.

      bound0 : Set a lower or upper value of the slab.

      bound1 : Set the second bound, upper or lower, respectively.
     */
    const argtype &args = argtype());

  double nearest_distance(const Position& point) const override {
    return slab_->nearest_distance(point); }

  void serialize(std::ostream& ostr) const override;
  std::shared_ptr<Shape> create(std::istream& istr) const override {
    return std::make_shared<Slab>(istr); }
  explicit Slab(std::istream& istr);
  virtual ~Slab() {}

 private:
  const std::string class_name_ = "Slab";
  std::shared_ptr<Shape> slab_;
  Arguments args_;
};

inline std::shared_ptr<Slab> MakeSlab(
    const argtype& args = argtype()) {
  return std::make_shared<Slab>(args);
}

}  // namespace feasst

#endif  // FEASST_CONFINEMENT_SLAB_H_
