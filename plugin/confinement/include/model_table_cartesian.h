
#ifndef FEASST_CONFINEMENT_MODEL_TABLE_CARTESIAN_H_
#define FEASST_CONFINEMENT_MODEL_TABLE_CARTESIAN_H_

#include <vector>
#include "utils/include/arguments.h"
#include "math/include/table.h"
#include "system/include/model_one_body.h"

namespace feasst {

class Shape;
class System;

// HWH have domain return scaled coordinates?
/**
  A tabular potential based on cartesian coordinates.
  Assumes symmetry along the x plane and that the Domain has no tilt.
 */
class ModelTableCart1DHard : public ModelOneBody {
 public:
  ModelTableCart1DHard(std::shared_ptr<Table1D> table) { table_ = table; }
  double energy(
    const Position& wrapped_site,
    const Site& site,
    const Configuration& config,
    const ModelParams& model_params) const override;

  const Table1D& table() const;

  /**
    Generate the table by finding where the point is inside the shape and the
    nearest distance to the surface is half of the diameter.
    The initial bounds are [0, L/2] inclusive, assuming
    a plane (or line) of symmetry at origin perpendicular to y axis.

    args:
    - diameter: diameter of the sphere (default: 1)
   */
  void compute_table(
    Shape * shape,
    Domain * domain,
    Random * random,
    const argtype& args = argtype());

  void serialize(std::ostream& ostr) const override;
  std::shared_ptr<Model> create(std::istream& istr) const override {
    return std::make_shared<ModelTableCart1DHard>(istr); }
  explicit ModelTableCart1DHard(std::istream& istr);
  virtual ~ModelTableCart1DHard() {}

 private:
  const std::string class_name_ = "ModelTableCart1DHard";
  std::shared_ptr<Table1D> table_;
};

inline std::shared_ptr<ModelTableCart1DHard> MakeModelTableCart1DHard(
    std::shared_ptr<Table1D> table) {
  return std::make_shared<ModelTableCart1DHard>(table);
}

/**
  A tabular potential based on cartesian coordinates.
  Assumes symmetry along the x, y planes and that the Domain has no tilt.
  Integration of material does not take periodicity into account.
  E.g., the shapes extend forever and are not periodic in the domain.
 */
class ModelTableCart2DIntegr : public ModelOneBody {
 public:
  ModelTableCart2DIntegr(std::shared_ptr<Table2D> table) { table_ = table; }
  double energy(
    const Position& wrapped_site,
    const Site& site,
    const Configuration& config,
    const ModelParams& model_params) const override;

  const Table2D& table() const;

  /// Generate the table by integration of the shape of the confinement over
  /// the entire and domain.
  void compute_table(
    Shape * shape,
    Domain * domain,
    Random * random,
    /// See Shape for documentation of integration_args.
    const argtype& integration_args);

  /// Same as above, but parallelize the task with OMP.
  void compute_table_omp(
    Shape * shape,
    Domain * domain,
    Random * random,
    const argtype& integration_args,
    /// See Thread for documentation of these two arguments.
    const int node = 0,
    const int num_node = 1);

  void serialize(std::ostream& ostr) const override;
  std::shared_ptr<Model> create(std::istream& istr) const override {
    return std::make_shared<ModelTableCart2DIntegr>(istr); }
  explicit ModelTableCart2DIntegr(std::istream& istr);
  virtual ~ModelTableCart2DIntegr() {}

 private:
  const std::string class_name_ = "ModelTableCart2DIntegr";
  std::shared_ptr<Table2D> table_;
};

inline std::shared_ptr<ModelTableCart2DIntegr> MakeModelTableCart2DIntegr(
    std::shared_ptr<Table2D> table) {
  return std::make_shared<ModelTableCart2DIntegr>(table);
}

/**
  A tabular potential based on cartesian coordinates.
  Assumes symmetry along the x, y and z planes and that the Domain has no tilt.
  Integration of material does not take periodicity into account.
  E.g., the shapes extend forever and are not periodic in the domain.
 */
class ModelTableCart3DIntegr : public ModelOneBody {
 public:
  ModelTableCart3DIntegr(std::shared_ptr<Table3D> table) { table_ = table; }
  double energy(
    const Position& wrapped_site,
    const Site& site,
    const Configuration& config,
    const ModelParams& model_params) const override;

  const Table3D& table() const;

  /**
    Generate the table by integration of a shape, which represents a continuous
    medium, over the entire domain.
   */
  void compute_table(
    Shape * shape,
    Domain * domain,
    Random * random,
    /// See Shape for documentation of integration_args.
    const argtype& integration_args);

  /// Same as above, but parallelize the task with OMP.
  void compute_table_omp(
    Shape * shape,
    Domain * domain,
    Random * random,
    const argtype& integration_args,
    /// See Thread for documentation of these two arguments.
    const int node = 0,
    const int num_nodes = 1);

  /**
    Generate the table by computing the energy of interaction of the select
    with the rest of the system.
    The select is assumed to be a single site, so that tables can be generated
    for each site type.
   */
  void compute_table(
    System * system,
    Select * select);

  /// Same as above, but parallelize the task with OMP
  void compute_table_omp(
    System * system,
    Select * select,
    /// See Thread for documentation of these two arguments.
    const int node = 0,
    const int num_node = 1);

  void serialize(std::ostream& ostr) const override;
  std::shared_ptr<Model> create(std::istream& istr) const override {
    return std::make_shared<ModelTableCart3DIntegr>(istr); }
  explicit ModelTableCart3DIntegr(std::istream& istr);
  virtual ~ModelTableCart3DIntegr() {}

 private:
  const std::string class_name_ = "ModelTableCart3DIntegr";
  std::shared_ptr<Table3D> table_;
};

inline std::shared_ptr<ModelTableCart3DIntegr> MakeModelTableCart3DIntegr(
    std::shared_ptr<Table3D> table) {
  return std::make_shared<ModelTableCart3DIntegr>(table);
}

}  // namespace feasst

#endif  // FEASST_CONFINEMENT_MODEL_TABLE_CARTESIAN_H_
