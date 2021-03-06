#include <cmath>
#include "utils/include/serialize.h"
#include "configuration/include/physical_constants.h"
#include "math/include/constants.h"

namespace feasst {

std::map<std::string, std::shared_ptr<PhysicalConstants> >& PhysicalConstants::deserialize_map() {
  static std::map<std::string, std::shared_ptr<PhysicalConstants> >* ans =
     new std::map<std::string, std::shared_ptr<PhysicalConstants> >();
  return *ans;
}

std::shared_ptr<PhysicalConstants> PhysicalConstants::deserialize(std::istream& istr) {
  return template_deserialize(deserialize_map(), istr,
    // true argument denotes rewinding to reread class name
    // this allows derived class constructor to read class name.
    true);
}

const double PhysicalConstants::charge_conversion() const {
  return std::pow(elementary_charge(), 2)/
  (4*PI*permitivity_vacuum()*1e3/1e10/avogadro_constant());
}

class MapCODATA2018 {
 public:
  MapCODATA2018() {
    auto obj = MakeCODATA2018();
    obj->deserialize_map()["CODATA2018"] = obj;
  }
};

static MapCODATA2018 mapper_codata2018_ = MapCODATA2018();

class MapCODATA2014 {
 public:
  MapCODATA2014() {
    auto obj = MakeCODATA2014();
    obj->deserialize_map()["CODATA2014"] = obj;
  }
};

static MapCODATA2014 mapper_codata2014_ = MapCODATA2014();

class MapCODATA2010 {
 public:
  MapCODATA2010() {
    auto obj = MakeCODATA2010();
    obj->deserialize_map()["CODATA2010"] = obj;
  }
};

static MapCODATA2010 mapper_codata2010_ = MapCODATA2010();

class MapPhysicalConstantsCustom {
 public:
  MapPhysicalConstantsCustom() {
    auto obj = MakePhysicalConstantsCustom({
      {"boltzmann_constant", "0"},
      {"avogadro_constant", "0"},
      {"permitivity_vacuum", "0"},
      {"elementary_charge", "0"}});
    obj->deserialize_map()["PhysicalConstantsCustom"] = obj;
  }
};

static MapPhysicalConstantsCustom mapper_physical_constants_custom_ = MapPhysicalConstantsCustom();

PhysicalConstantsCustom::PhysicalConstantsCustom(const argtype& args)
  : PhysicalConstants() {
  class_name_ = "PhysicalConstantsCustom";
  Arguments args_(args);
  boltzmann_constant_ = args_.key("boltzmann_constant").dble();
  avogadro_constant_ = args_.key("avogadro_constant").dble();
  permitivity_vacuum_ = args_.key("permitivity_vacuum").dble();
  elementary_charge_ = args_.key("elementary_charge").dble();
}

void PhysicalConstantsCustom::serialize(std::ostream& ostr) const {
  ostr << class_name() << " ";
  feasst_serialize(boltzmann_constant_, ostr);
  feasst_serialize(avogadro_constant_, ostr);
  feasst_serialize(permitivity_vacuum_, ostr);
  feasst_serialize(elementary_charge_, ostr);
}

PhysicalConstantsCustom::PhysicalConstantsCustom(std::istream& istr)
  : PhysicalConstants(istr) {
  feasst_deserialize(&boltzmann_constant_, istr);
  feasst_deserialize(&avogadro_constant_, istr);
  feasst_deserialize(&permitivity_vacuum_, istr);
  feasst_deserialize(&elementary_charge_, istr);
}

} // namespace feasst
