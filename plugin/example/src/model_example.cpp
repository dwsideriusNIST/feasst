#include "example/include/model_example.h"
#include "utils/include/serialize.h"

namespace feasst {

class MapModelExample {
 public:
  MapModelExample() {
    ModelExample().deserialize_map()["ModelExample"] = MakeModelExample();
  }
};

static MapModelExample mapper_ = MapModelExample();

ModelExample::ModelExample() {}

void ModelExample::serialize(std::ostream& ostr) const {
  ostr << class_name_ << " ";
  feasst_serialize_version(5023, ostr);
}

ModelExample::ModelExample(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 5023, "unrecognized version: " << version);
}

}  // namespace feasst
