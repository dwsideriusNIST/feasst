
#ifndef FEASST_UTILS_INCLUDE_CHECKPOINT_H_
#define FEASST_UTILS_INCLUDE_CHECKPOINT_H_

#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include "utils/include/arguments.h"
#include "utils/include/timer.h"

namespace feasst {

/**
  Checkpoint after user defined number of hours.
 */
class Checkpoint {
 public:
  /**
    args:
    - num_hours: Number of hours between printing of checkpoint file
      (default: 12).
    - file_name: The default is one space (e.g., " ") and no checkpointing.
   */
  explicit Checkpoint(const argtype &args = argtype());

  /// Return number of hours between writing file.
  double num_hours() const { return num_hours_; }

  /// Write the checkpoint to file.
  template <typename T>
  void write(const T& obj) {
    if (file_name_.empty() || file_name_ == " ") {
      return;
    }
    std::ofstream file(file_name_.c_str(),
      std::ofstream::out | std::ofstream::trunc);
    std::stringstream ss;
    obj.serialize(ss);
    file << ss.str();
    file.close();
  }

  /// Write object to file_name if num_hours has passed since previous.
  template <typename T>
  void check(const T& obj) {
    const double hours = cpu_hours();
    if (hours > previous_hours_ + num_hours_) {
      previous_hours_ = hours;
      write(obj);
    }
  }

  /// Initialize object by reading from file.
  template <typename T>
  void read(T * obj) {
    std::ifstream file(file_name_.c_str());
    std::string line;
    std::getline(file, line);
    std::stringstream ss(line);
    *obj = T(ss);
  }

  /// Serialize object.
  void serialize(std::ostream& ostr) const;

  /// Deserialize object.
  explicit Checkpoint(std::istream& istr);

 private:
  std::string file_name_;
  double previous_hours_ = 0.;
  double num_hours_ = 0;
  Arguments args_;
};

inline std::shared_ptr<Checkpoint> MakeCheckpoint(
    const argtype &args = argtype()) {
  return std::make_shared<Checkpoint>(args);
}

}  // namespace feasst

#endif  // FEASST_UTILS_INCLUDE_CHECKPOINT_H_
