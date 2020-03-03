#include "system/include/energy_map.h"

namespace feasst {

std::map<std::string, std::shared_ptr<EnergyMap> >& EnergyMap::deserialize_map() {
  static std::map<std::string, std::shared_ptr<EnergyMap> >* ans =
     new std::map<std::string, std::shared_ptr<EnergyMap> >();
  return *ans;
}

void EnergyMap::clear(
    const int part1_index,
    const int site1_index,
    const int part2_index,
    const int site2_index) {
  resize_(part1_index, site1_index, part2_index, site2_index);
  *smap_new_(part1_index, site1_index, part2_index, site2_index) =
    std::vector<double>(2 + dimen_, default_value());
  *smap_new_(part2_index, site2_index, part1_index, site1_index) =
    std::vector<double>(2 + dimen_, default_value());
}

double EnergyMap::update(
    const double energy,
    const int part1_index,
    const int site1_index,
    const int part2_index,
    const int site2_index,
    const double squared_distance,
    const Position * pbc) {
  resize_(part1_index, site1_index, part2_index, site2_index);
  std::vector<double> * smap1 =
    smap_new_(part1_index, site1_index, part2_index, site2_index);
  (*smap1)[0] = energy;
  (*smap1)[1] = squared_distance;
  if (pbc->dimension() > 0) {
    for (int dim = 0; dim < dimen_; ++dim) {
      (*smap1)[2 + dim] = pbc->coord(dim);
    }
  }
  std::vector<double> * smap2 =
    smap_new_(part2_index, site2_index, part1_index, site1_index);
  (*smap2)[0] = energy;
  (*smap2)[1] = squared_distance;
  if (pbc->dimension() > 0) {
    for (int dim = 0; dim < dimen_; ++dim) {
      (*smap2)[2 + dim] = -1.*pbc->coord(dim);
    }
  }
  return energy;
}

double EnergyMap::query(
    const int part1_index,
    const int site1_index,
    const int part2_index,
    const int site2_index) {
  return (*smap_(part1_index, site1_index, part2_index, site2_index))[0];
}

void EnergyMap::precompute(Configuration * config) {
  site_max_ = config->max_sites_in_any_particle();
  DEBUG("site_max_ " << site_max_);
  dimen_ = config->dimension();
}

double EnergyMap::total_energy() const {
  double en = 0;
  //DEBUG(map().size() << " " << map()[0].size() << " " << map()[0][0].size() << " " << map()[0][0][0].size() << " " << map()[0][0][0][0].size());
  for (const std::vector<std::vector<std::vector<std::vector<double> > > >& vec4 : map()) {
    for (const std::vector<std::vector<std::vector<double> > >& vec3 : vec4) {
      for (const std::vector<std::vector<double> >& vec2 : vec3) {
        for (const std::vector<double>& vec1 : vec2) {
          //DEBUG(vec1.size());
          en += vec1[0];
        }
      }
    }
  }
  return 0.5*en;
}

}  // namespace feasst