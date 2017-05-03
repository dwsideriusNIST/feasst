#include "./trial_beta.h"

namespace feasst {

TrialBeta::TrialBeta()
  : Trial() {
  defaultConstruction();
}
TrialBeta::TrialBeta(Space *space,
  Pair *pair,
  Criteria *criteria)
  : Trial(space, pair, criteria) {
  defaultConstruction();
}
TrialBeta::TrialBeta(const char* fileName,
  Space *space,
  Pair *pair,
  Criteria *criteria)
  : Trial(space, pair, criteria, fileName) {
  defaultConstruction();
}

/**
 * default construction
 */
void TrialBeta::defaultConstruction() {
  className_.assign("TrialBeta");
  trialType_.assign("move");
  verbose_ = 0;
}

}  // namespace feasst

