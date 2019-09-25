
#ifndef FEASST_CONFIGURATION_PARTICLE_FACTORY_H_
#define FEASST_CONFIGURATION_PARTICLE_FACTORY_H_

#include <vector>
#include "configuration/include/particle.h"
#include "configuration/include/model_params.h"
#include "configuration/include/group.h"

namespace feasst {

/**
  A container for a list of particles.
  There are effectively three different uses:

    1. A list of particles that exist in a simulation. In this case there can
       be multiple particles and sites of the same type.

    2. A list of particle types that may exist in a simulation. In this case
       there can not be multiple particles of the same type.
       This is enforced by ParticleFactory::unique_particles().

    3. A list of site and bond types (contained within particles) that may exist
       in a simulation.
       In this case, there can not be multiple sites or bonds of the same
       type. This contains site-based and bond-based properties.
       This is enforced by ParticleFactory::unique_types().
 */
class ParticleFactory {
 public:
  ParticleFactory() {}

  /// Adjust the site types of added particles to ensure uniqueness.
  /// Returns self for chain setting.
  ParticleFactory& unique_particles();

  /**
    Only add sites or bonds which are new as a holder for site- or bond- type
    properties.
    Note that ParticleFactory::unique_particles() is also applied.
    Returns self for chain setting.
   */
  ParticleFactory& unique_types();

  /**
    Check that no site or particle type is skipped.
    As particles and sites are added, a never before seen type must be only
    one higher than the previous maximum index.
    Types begin with 0.

    For example, a particle with sites of type {0, 2} is invalid because
    the second site of type "2" appears without a previous type "1" site.

    In another example, a particle with sites of type {1,2} is invalid because
    the site type doesn't begin with 0.

    Note: for partial configurations by groups, these checks may not apply.
   */
  void check_types(int * num_site_types, int * num_particle_types) const;

  /// Check that no site or particle type is skipped.
  void check_types() const;

  /// Check that no site type is skipped.
  /// \return number of site types.
  int check_site_types() const;

  /// Check that no particle type is skipped.
  /// \return number of particle types.
  int check_particle_types() const;

  /// Add particle by file.
  void add(const std::string file_name);

  /// Add a particle.
  void add(const Particle& particle);

  /// Remove particle by index.
  void remove(const int particle_index);

  /// Return particle by index.
  const Particle& particle(const int particle_index) const;

  /// Return particles.
  std::vector<Particle> particles() const { return particles_; }

  /// Return the number of particles.
  int num() const { return static_cast<int>(particles_.size()); }

  /// Replace position of the particle by index.
  void replace_position(const int particle_index, const Particle& replacement);

  /// Replace position of the site by index.
  void replace_position(const int particle_index,
                        const int site_index,
                        const Position& replacement);

  /// Replace position of particle but not site.
  void replace_position(const int particle_index,
                        const Position& replacement);

  /// Replace properties of the site by index.
  void replace_properties(const int particle_index,
                          const int site_index,
                          const Properties& replacement,
                          const std::vector<std::string>& exclude) {
    particles_[particle_index].replace_properties(site_index, replacement, exclude); }

  /// Check consistency of dimensionality of positions of particles and sites.
  /// By default, for dimension == -1, determine automatically.
  void check(const int dimension = -1) const;

  /// Return the number of site types.
  int num_site_types() const { return check_site_types(); }

  /// Return the number of sites.
  int num_sites() const;

  /// Return the number of particle types.
  int num_particle_types() const { return check_particle_types(); }

  /// Remove particles and sites based on the group.
  void remove(const Group group);

  /// Displace the particle with given index.
  void displace(const int particle_index, const Position& displacement) {
    particles_[particle_index].displace(displacement);
  }

  /// Return the model parameters.
  const ModelParams& model_params() const { return model_params_; }

  /// Add a custom type of model parameter.
  void add(const std::shared_ptr<ModelParam> param) {
    model_params_.add(param); }

  /// Modify model parameter of a given site type and name to value.
  void set_model_param(const std::string name,
                       const int site_type,
                       const double value) {
    model_params_.set(name, site_type, value);
  }

  /// Add model parameter of a given name to value.
  void add_model_param(const std::string name,
                       const double value) {
    model_params_.add_property(name, value);
  }

  /// Set site as physical/nonphysical.
  void set_site_physical(const int particle, const int site, const bool phys) {
    particles_[particle].set_site_physical(site, phys); }

  /// Add the property of sites in a particle.
  void add_site_property(const std::string name,
      const double value,
      const int particle_index,
      const int site_index) {
    particles_[particle_index].add_site_property(name, value, site_index);
  }

  /// Add or set the property of sites in a particle.
  void add_or_set_site_property(const std::string name,
      const double value,
      const int particle_index,
      const int site_index) {
    particles_[particle_index].add_or_set_site_property(name, value, site_index);
  }

  /// Set the property of sites in a particle.
  void set_site_property(const std::string name,
      const double value,
      const int particle_index,
      const int site_index) {
    particles_[particle_index].set_site_property(name, value, site_index);
  }
  void set_site_property(const int index,
      const double value,
      const int particle_index,
      const int site_index) {
    particles_[particle_index].set_site_property(index, value, site_index);
  }

  void serialize(std::ostream& ostr) const;
  ParticleFactory(std::istream& istr);

 private:
  std::vector<Particle> particles_;
  bool unique_particles_ = false;
  bool unique_types_ = false;
  ModelParams model_params_;
};

}  // namespace feasst

#endif  // FEASST_CONFIGURATION_PARTICLE_FACTORY_H_