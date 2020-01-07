
// HWH consider making a feasst omp plugin that could interface other parallelization methods
#ifdef _OPENMP
  #include <omp.h>
#endif // _OPENMP
#include "prefetch/include/prefetch.h"

// // use this to turn prefetch serial to simply debugging
// #define DEBUG_SERIAL_MODE_5324634

namespace feasst {

MonteCarlo * Prefetch::clone_(const int ithread) {
  if (ithread == 0) {
    return this;
  }
  return &pool_[ithread].mc;
}

void Prefetch::attempt_(
    const int num_trials,
    TrialFactory * trial_factory,
    Random * random) {
  if (!is_activated_) {
    MonteCarlo::attempt_(num_trials, trial_factory, random);
    return;
  }

  #ifndef _OPENMP
    ERROR("requires openmp");
  #endif // _OPENMP

  // initialize MC clones for each processor
  // and pool size.
  // alternatively, input the clones by using a functional
  // approach to creating mc objects
  int proc_id;
  int first_thread_accepted;
  if (pool_.size() == 0) {
    #ifdef _OPENMP
      #pragma omp parallel private(proc_id)
      {
        proc_id = omp_get_thread_num();
        DEBUG("hello from " << proc_id);
        if (proc_id == 0) {
          num_threads_ = static_cast<int>(omp_get_num_threads());
        }
      }
    #else // _OPENMP
      num_threads_ = 1;
    #endif // _OPENMP
//    num_threads_ = 2;
    pool_.resize(num_threads_);

    // set all trials for delayed finalization
    delay_finalize();

    //clones_.resize(num_threads_);
    for (int thread = 1; thread < num_threads_; ++thread) {
      std::stringstream clone_ss;
      MonteCarlo::serialize(clone_ss);
      pool_[thread].mc = MonteCarlo(clone_ss);
    }

    // offset random number chains so that clones are not equal
    for (int i = 0; i < num_threads_ - 1; ++i) {
      for (int j = i + 1; j < num_threads_; ++j) {
        clone_(i)->offset_random();
      }
    }

    // run some checks before attempting trials
    for (int thread = 0; thread < num_threads_; ++thread) {
      clone_(thread)->before_attempts_();
    }
  }

  #ifdef _OPENMP
  #pragma omp parallel private(proc_id)
  {
    proc_id = omp_get_thread_num();
  #endif // _OPENMP

    for (int itrial = 0; itrial < num_trials; ++itrial) {
      #ifndef _OPENMP
      for (proc_id = 0; proc_id < num_threads_; ++proc_id) {
      #endif // _OPENMP

      if (proc_id == 0) {
        DEBUG("************************");
        DEBUG("* Begin Prefetch cycle *");
        DEBUG("************************");
      }

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      // ordered list of randomly generated moves identified by index of trial.
      if (proc_id == 0) {
        for (int ithread = 0; ithread < num_threads_; ++ithread) {
          pool_[ithread].set_index(trial_factory->random_index(random));
          DEBUG("num attempts " << pool_[ithread].mc.trials().num_attempts() << " "
                                << pool_[ithread].mc.trials().num_success() << " " << &pool_[ithread].mc);
        }
        DEBUG("num attempts master " << trials().num_attempts() << " "
                                     << trials().num_success() << " " << this);
      }

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      // set random number generators to store (zero storage for selecting trial?).
      Pool * pool = &pool_[proc_id];
      MonteCarlo * mc = clone_(proc_id);
      mc->load_cache(true);

      #ifdef DEBUG_SERIAL_MODE_5324634
      #pragma omp critical
      {
      #endif

      // Each processor attempts their trial in parallel,
      // without analyze modify or checkpoint.
      // Store new macrostate and acceptance prob
      pool->set_accepted(mc->attempt_trial(pool->index()));
      pool->set_ln_prob(mc->trial(pool->index())->accept().ln_metropolis_prob());
      DEBUG("critical proc_id " << proc_id << " " << pool->str());
      DEBUG("nump " << mc->system().configuration().num_particles());

      #ifdef DEBUG_SERIAL_MODE_5324634
      }
      #endif

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      // Determine first trial accepted (if any).
      if (proc_id == 0) {
        first_thread_accepted = num_threads_;
        for (int ithread = 0; ithread < num_threads_; ++ithread) {
          if (pool_[ithread].accepted() &&
              first_thread_accepted == num_threads_) {
            first_thread_accepted = ithread;
          }
        }
        DEBUG("first thread " << first_thread_accepted);
      }

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      #ifdef DEBUG_SERIAL_MODE_5324634
      #pragma omp critical
      {
      #endif

      // revert trials after accepted trial.
      if (first_thread_accepted != num_threads_) {
        if (proc_id > first_thread_accepted) {
          DEBUG("reverting trial " << proc_id);
          mc->revert(pool->index(), pool->accepted());
        }
      }

      #ifdef DEBUG_SERIAL_MODE_5324634
      }
      #endif

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      if (proc_id == 0) {
        DEBUG("for each thread up to the first accepted, "
           << "update other threads (incl. main) regarding failed attempt by "
           << "thread. update steppers");
        for (int ithread = 0; ithread < first_thread_accepted; ++ithread) {
          DEBUG("mimic failed " << ithread);
          // loop through other threads to update
          for (int jthread = 0; jthread < num_threads_; ++jthread) {
            if (ithread != jthread) {
              clone_(jthread)->mimic_trial_rejection(
                pool_[ithread].index(),
                pool_[ithread].ln_prob());
            }
          }
          DEBUG("num attempts " << pool->mc.trials().num_attempts() << " "
                                << pool->mc.trials().num_success() << " " << &pool->mc);
        }
        DEBUG("num attempts master " << trials().num_attempts() << " "
                                     << trials().num_success() << " " << this);
      }

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      #ifdef DEBUG_SERIAL_MODE_5324634
      #pragma omp critical
      {
      #endif

      if (first_thread_accepted < num_threads_) {
        DEBUG("Replicate first accepted trial in all other threads");
        Pool * accepted_pool = &pool_[first_thread_accepted];
        if (proc_id == first_thread_accepted) {
          mc->finalize(accepted_pool->index());
        } else {
          // load/unload system energies and random numbers
          mc->unload_cache(*clone_(first_thread_accepted));
          mc->attempt_trial(accepted_pool->index());
          mc->finalize(accepted_pool->index());
        }
      }

      #ifdef DEBUG_SERIAL_MODE_5324634
      }
      #endif

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      #ifdef DEBUG_SERIAL_MODE_5324634
      #pragma omp critical
      {
      #endif

      // disable cache
      mc->load_cache(false);

      // perform after trial on all clones/master after multiple trials performed
      mc->after_trial_();

      #ifdef DEBUG_SERIAL_MODE_5324634
      }
      #endif

      #ifdef _OPENMP
      #pragma omp barrier
      #endif // _OPENMP

      #ifdef DEBUG_SERIAL_MODE_5324634
      #pragma omp critical
      {
      #endif

      // periodically check that all threads are equal
      if (itrial % steps_per_check_ == 0 &&
          proc_id > 0) {
        const double energy = criteria()->current_energy();
        DEBUG("check that the current energy of all threads and master are the same: " << energy);
        const double tolerance = 1e-8;
        const MonteCarlo& mcc = *clone_(proc_id);
        const double diff = mcc.criteria()->current_energy() - energy;
        ASSERT(fabs(diff) <= tolerance, "diff: " << diff);
        ASSERT(system().configuration().is_equal(mcc.system().configuration()), "configs not equal thread" << proc_id);
        ASSERT(trials().is_equal(mcc.trials()), "trials not equal thread" << proc_id);
      }

      #ifdef DEBUG_SERIAL_MODE_5324634
      }
      #endif

      // periodically equate all clones to main to prevent drift (energy checks?)
    }
  }
}

}  // namespace feasst
