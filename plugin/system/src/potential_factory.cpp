#include <vector>
#include <string>
#include "system/include/potential_factory.h"
#include "utils/include/debug.h"
#include "utils/include/serialize.h"
#include "math/include/constants.h"
#include "math/include/utils_math.h"

namespace feasst {

void PotentialFactory::add(const Potential& potential) {
  potentials_.push_back(potential);
}

void PotentialFactory::set(const int index,
                           const Potential& potential) {
  potentials_[index] = potential;
}

void PotentialFactory::precompute(Configuration * config) {
  for (Potential& potential : potentials_) {
    potential.precompute(config);
  }
}

void PotentialFactory::precompute(const int index, Configuration * config) {
  potentials_[index].precompute(config);
}

double PotentialFactory::energy(Configuration * config) {
  double en = 0;
  int index = 0;
  while ((index < static_cast<int>(potentials_.size())) and
         (en < NEAR_INFINITY/10.)) {
    DEBUG("potential index: " << index);
    en += potentials_[index].energy(config);
    ++index;
  }
  DEBUG("en " << en);
  DEBUG(str());
  return en;
}

double PotentialFactory::energy(const Select& select, Configuration * config) {
  double en = 0;
  int index = 0;
  while ((index < static_cast<int>(potentials_.size())) and
         (en < NEAR_INFINITY)) {
    en += potentials_[index].energy(select, config);
    ++index;
  }
  DEBUG("en " << en);
  DEBUG(str());
  return en;
}

std::vector<double> PotentialFactory::stored_energy_profile() const {
  std::vector<double> en;
  for (const Potential& potential : potentials_) {
    en.push_back(potential.stored_energy());
  }
  return en;
}

double PotentialFactory::stored_energy() const {
  std::vector<double> en = stored_energy_profile();
  return std::accumulate(en.begin(), en.end(), 0.);
}

std::string PotentialFactory::str() const {
  std::stringstream ss;
  ss << "PotentialFactory: ";
  for (const Potential& potential : potentials_) {
    ss << potential.stored_energy() << " ";
  }
  return ss.str();
}

void PotentialFactory::revert(const Select& select) {
  for (Potential& potential : potentials_) {
    potential.revert(select);
  }
}

void PotentialFactory::finalize(const Select& select) {
  for (Potential& potential : potentials_) {
    potential.finalize(select);
  }
}

void PotentialFactory::serialize(std::ostream& sstr) const {
  feasst_serialize_version(8655, sstr);
  feasst_serialize_fstobj(potentials_, sstr);
}

PotentialFactory::PotentialFactory(std::istream& sstr) {
  const int version = feasst_deserialize_version(sstr);
  ASSERT(version == 8655, "unrecognized verison: " << version);
  feasst_deserialize_fstobj(&potentials_, sstr);
}

void PotentialFactory::load_cache(const bool load) {
  for (Potential& potential : potentials_) {
    potential.load_cache(load);
  }
}

void PotentialFactory::unload_cache(const PotentialFactory& factory) {
  ASSERT(num() == factory.num(), "size mismatch");
  for (int ip = 0; ip < num(); ++ip) {
    potentials_[ip].unload_cache(factory.potentials_[ip]);
  }
}

void PotentialFactory::check() const {
  for (const Potential& potential : potentials_) {
    potential.check();
  }
}

}  // namespace feasst
