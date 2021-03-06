#ifdef _OPENMP
  #include <omp.h>
#endif // _OPENMP
#ifdef _WIN32
  #include <Windows.h>  // sleep
#else
  #include <unistd.h>  // sleep
#endif
#include "utils/include/debug.h"
#include "utils/include/serialize.h"
#include "math/include/constants.h"
#include "math/include/utils_math.h"
#include "flat_histogram/include/flat_histogram.h"
#include "flat_histogram/include/clones.h"

namespace feasst {

//void Clones::create(const MonteCarlo& mc,
//                    const Window& window) {
//  std::stringstream ss_fh, ss_macro;
//  ASSERT(mc.criteria().class_name() == "FlatHistogram",
//    "assumes flat histogram criteria: " << mc.criteria().class_name());
//  mc.criteria().serialize(ss_fh);
//  FlatHistogram fh(ss_fh);
//  fh.macrostate().serialize(ss_macro);
//  auto macrostate = std::make_shared<Macrostate>(ss_macro);
//  for (std::vector<int> bounds : window.boundaries()) {
//    auto clone = std::make_shared<MonteCarlo>(deep_copy(mc));
//    auto macro2 = deep_copy_derived(macrostate);
//    macro2->set(Histogram({{"width"
//    auto macrostate =
//    clone->set(MakeFlatHistogram(
//  }
//}

const MonteCarlo& Clones::clone(const int index) const {
  ASSERT(index < num(), "index: " << index << " >= num: " << num());
  return const_cast<MonteCarlo&>(*clones_[index]);
}

MonteCarlo * Clones::get_clone(const int index) {
  ASSERT(index < num(), "index: " << index << " >= num: " << num());
  return clones_[index].get();
}

void Clones::initialize(const int upper_index,
    const argtype& args) {
  Arguments args_(args);
  const int attempt_batch = args_.key("attempt_batch").dflt("1").integer();
  const int max_batch = args_.key("max_batch").dflt("-1").integer();
  Acceptance empty;
  MonteCarlo * lower = clones_[upper_index - 1].get();
  MonteCarlo * upper = clones_[upper_index].get();
  FlatHistogram fh_lower = flat_histogram(upper_index - 1);
  FlatHistogram fh_upper = flat_histogram(upper_index);
  const double macro_upper_min = fh_upper.macrostate().value(0);
  const double macro_lower_max =
    fh_lower.macrostate().histogram().center_of_last_bin();

  DEBUG("find configuration in lower which overlaps with upper");
  bool overlap = false;
  int batch = 0;
  while (!overlap) {
    lower->attempt(attempt_batch);

    const double macro_lower = fh_lower.macrostate().value(
      lower->system(), lower->criteria(), empty);
    DEBUG("macro_lower: " << macro_lower);
    if (is_in_interval(macro_lower, macro_upper_min, macro_lower_max)) {
      overlap = true;
    }

    ++batch;
    ASSERT(max_batch == -1 || batch < max_batch,
      "reached maximum batch: " << batch);
  }

  DEBUG("send configuration to upper");
  //upper->set(deep_copy(lower->system()));
  upper->get_system()->get_configuration()->copy_particles(
    lower->configuration(),
    true);
  DEBUG("num " << upper->configuration().num_particles());
  upper->initialize_energy();
}

void Clones::initialize(const argtype& args) {
  for (int upper_index = 1; upper_index < num(); ++upper_index) {
    initialize(upper_index, args);
  }
}

void Clones::run_until_complete(const argtype& args) {
#ifdef _OPENMP
  run_until_complete_omp_(args);
#else // _OPENMP
  run_until_complete_serial_();
#endif // _OPENMP
}

void Clones::run_until_complete_serial_() {
  DEBUG("run_until_complete_serial_");
  for (auto clone : clones_) {
    clone->run_until_complete();
  }
}

bool are_all_complete(const std::vector<bool>& is_complete) {
  bool complete = true;
  for (const bool comp : is_complete) {
    if (!comp) complete = false;
  }
  return complete;
}

void Clones::run_until_complete_omp_(const argtype& run_args,
                                     const bool init,
                                     const argtype& init_args) {
  DEBUG("run_until_complete_omp_");
#ifdef _OPENMP
  Arguments args_(run_args);
  const int omp_batch = args_.key("omp_batch").dflt("1").integer();
  std::vector<bool> is_complete(num(), false);
  std::vector<bool> is_initialized(num(), false);
  is_initialized[0] = true;
  #pragma omp parallel
  {
    const int thread = omp_get_thread_num();
    const int num_thread = static_cast<int>(omp_get_num_threads());
    DEBUG("thread " << thread << " of " << num_thread);

    ASSERT(num() <= num_thread, "more clones: " << num() << " than OMP threads: "
      << num_thread << ". Use \"export OMP_NUM_THREADS=\" to set OMP threads.");
    if (thread < num()) {
      if (init) {
        // wait until thread is initialized
        while (!is_initialized[thread]) sleep(0.01);
        INFO("thread " << thread << " is initialized");

        // initialize next thread, if applicable
        if (thread < num() - 1) initialize(thread + 1, init_args);
        is_initialized[thread + 1] = true;
      }

      // run in parallel until complete
      clones_[thread]->run_until_complete();
      is_complete[thread] = true;

      // continue running while waiting for other threads to complete
      while (!are_all_complete(is_complete)) {
        clones_[thread]->attempt(omp_batch);
      }
    }

    #pragma omp barrier
  }

#else // _OPENMP
FATAL("Not complied with OMP");
#endif // _OPENMP
}

void Clones::initialize_and_run_until_complete(const argtype& init_args,
                                               const argtype& run_args) {
#ifdef _OPENMP
  run_until_complete_omp_(run_args, true, init_args);
#else // _OPENMP
  initialize(init_args);
  run_until_complete_serial_();
#endif // _OPENMP
}

FlatHistogram Clones::flat_histogram(const int index) const {
  std::stringstream ss;
  clone(index).criteria().serialize(ss);
  return FlatHistogram(ss);
}

LnProbability Clones::ln_prob(const argtype& args) const {
  std::vector<double> ln_prob;
  double shift = 0.;
  int starting_lower_bin = 0;
  for (int fh_index = 0; fh_index < num() - 1; ++fh_index) {
    FlatHistogram fh_lower = flat_histogram(fh_index);
    FlatHistogram fh_upper = flat_histogram(fh_index + 1);

    // determine if fh are TM and, if so, ignore the lowest macrostate of upper
    // and highest macrostate of lower
    int index_upper_min = 0;
    if (fh_upper.bias().class_name() == "TransitionMatrix") ++index_upper_min;
    const double macro_upper_min = fh_upper.macrostate().value(index_upper_min);

    // and same for lower, by simply introducing a truncation for the following
    // bin loop
    int truncate = 0;
    if (fh_lower.bias().class_name() == "TransitionMatrix") ++truncate;

    int upper_index = index_upper_min;
    std::vector<double> overlap_upper;
    std::vector<double> overlap_lower;
    for (int bin = starting_lower_bin;
         bin < fh_lower.bias().ln_prob().size() - truncate;
         ++bin) {
      const double macro_lower = fh_lower.macrostate().value(bin);
      const double ln_prob_lower = fh_lower.bias().ln_prob().value(bin);
      if (std::abs(macro_lower - macro_upper_min) > NEAR_ZERO &&
          upper_index == index_upper_min) {
        ln_prob.push_back(ln_prob_lower + shift);
      } else {
        DEBUG("upper index " << upper_index);
        DEBUG("stitch " << bin);
        overlap_upper.push_back(fh_upper.bias().ln_prob().value(upper_index));
        overlap_lower.push_back(ln_prob_lower);
        ++upper_index;
      }
    }
    ASSERT(upper_index > index_upper_min,
      "No overlap. If TransitionMatrix, did you add at least two extra " <<
      "overlap to acount for the spurious ln_prob at the ends?");

    // average the ln_probs of the overlapping region
    Accumulator ln_prob_shift;
    for (int ovl = 0; ovl < static_cast<int>(overlap_upper.size()); ++ovl) {
      ln_prob_shift.accumulate(overlap_lower[ovl] - overlap_upper[ovl]);
    }
    shift += ln_prob_shift.average();
    for (int ovl = 0; ovl < static_cast<int>(overlap_upper.size()); ++ovl) {
      DEBUG(shift);
      ln_prob.push_back(0.5*(overlap_lower[ovl] + overlap_upper[ovl] + shift));
    }
    starting_lower_bin = static_cast<int>(overlap_lower.size() + truncate);
  }

  // now add the non-overlapping part of the last clone
  FlatHistogram fh = flat_histogram(num() - 1);
  for (int bin = starting_lower_bin; bin < fh.bias().ln_prob().size(); ++bin) {
    ln_prob.push_back(fh.bias().ln_prob().value(bin) + shift);
  }

  LnProbability lnpi(ln_prob);
  lnpi.normalize();
  return lnpi;
}

void Clones::serialize(std::ostream& ostr) const {
  feasst_serialize_version(2845, ostr);
  feasst_serialize(clones_, ostr);
}

Clones::Clones(std::istream& istr) {
  const int version = feasst_deserialize_version(istr);
  ASSERT(version == 2845, "version: " << version);
  // HWH for unknown reasons, this does not work
  //feasst_deserialize(&clones_, istr);
  int dim1;
  istr >> dim1;
  clones_.resize(dim1);
  for (int index = 0; index < dim1; ++index) {
    int existing;
    istr >> existing;
    if (existing != 0) {
      clones_[index] = std::make_shared<MonteCarlo>(istr);
    }
  }
}

}  // namespace feasst
