#ifndef FEASST_UTILS_INCLUDE_CACHE_H_
#define FEASST_UTILS_INCLUDE_CACHE_H_

#include <sstream>
#include <deque>

namespace feasst {

class Cache {
 public:
  Cache();

  bool is_loading() const { return is_loading_; }
  bool is_unloading() const { return is_unloading_; }

  /// Store numbers if true (default: false). Either way, any previously stored
  /// values are reset, and unloading is disabled.
  void set_load(const bool store = false);

  /// Unload numbers if true (default: false). Either way, loading is disabled.
  void set_unload(const bool unload = false);

  /// Preset numbers to those stored by another Cache.
  void set_unload(const Cache& cache);

  /// Return true if unloading into value.
  bool is_unloading(double * value);

  /// Attempt to load value into cache
  void load(const double value);

  /// Serialize.
  void serialize(std::ostream& ostr) const;

  /// Deserialize.
  Cache(std::istream& istr);

 private:
  bool is_loading_;
  bool is_unloading_;
  std::deque<double> stored_;
};

}  // namespace feasst

#endif  // FEASST_UTILS_INCLUDE_CACHE_H_
