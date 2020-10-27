#include <cmath>
#include "utils/include/serialize.h"
#include "math/include/constants.h"
#include "configuration/include/model_params.h"
#include "ewald/include/coulomb.h"

namespace feasst {

class MapCoulomb {
 public:
  MapCoulomb() {
    Coulomb().deserialize_map()["Coulomb"] = MakeCoulomb();
  }
};

static MapCoulomb map_charge_screened_ = MapCoulomb();

void Coulomb::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  feasst_serialize_version(1634, ostr);
  feasst_serialize(conversion_factor_, ostr);
}

Coulomb::Coulomb(std::istream& istr) : ModelTwoBody(istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 1634, "unrecognized verison: " << version);
  feasst_deserialize(&conversion_factor_, istr);
}

double Coulomb::energy(
    const double squared_distance,
    const int type1,
    const int type2,
    const ModelParams& model_params) const {
  const double distance = std::sqrt(squared_distance);
  if (std::abs(distance) < NEAR_ZERO) return NEAR_INFINITY;
  const double mixed_charge = model_params.mixed_charge()[type1][type2];
  return mixed_charge*conversion_factor_/distance;
}

void Coulomb::precompute(const ModelParams& existing) {
  conversion_factor_ = existing.constants().charge_conversion();
}

}  // namespace feasst