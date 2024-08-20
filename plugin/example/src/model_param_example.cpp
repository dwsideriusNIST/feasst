#include <cmath>
#include "utils/include/serialize.h"
#include "example/include/model_param_example.h"

namespace feasst {

class MapGamma {
 public:
  MapGamma() {
    auto obj = std::make_shared<Gamma>();
    obj->deserialize_map()["gamma"] = obj;
  }
};

static MapGamma mapper_gamma_ = MapGamma();

double Gamma::mix_(const double value1, const double value2) {
  return std::sqrt(value1*value2);
}

void Gamma::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  serialize_model_param_(ostr);
  feasst_serialize_version(9826, ostr);
}

Gamma::Gamma(std::istream& istr) : ModelParam(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 9826, "mismatch version: " << version);
}

}  // namespace feasst
