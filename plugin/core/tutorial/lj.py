import feasst

def lj():
  potential = feasst.Potential()
  potential.set_model(feasst.ModelLJShrPtr())
  return potential

def lj_cell():
  potential = lj()
  potential.set_visit_model(feasst.VisitModelCell())
  return potential

def lrc():
  potential = feasst.Potential()
  potential.set_visit_model(feasst.LongRangeCorrectionsShrPtr())
  return potential

def system(box_length):
  system = feasst.System()
  system.add(feasst.Configuration(feasst.args(
    {"cubic_box_length": str(box_length),
     "particle_type": "../../../forcefield/data.lj",
     "init_cells": "3"})))
  system.add(lj())
  system.add(lrc())
  if system.configuration().domain().is_cell_enabled():
    system.add_to_optimized(lj_cell())
    system.add_to_optimized(lrc())
  return system

def add_analysis(mc, steps_per):
  mc.add(feasst.MakeLog(feasst.args(
    {"steps_per" : str(steps_per)})))
  mc.add(feasst.MakeMovie(feasst.args(
    {"steps_per" : str(steps_per),
     "file_name" : "movie.xyz"})));
  mc.add(feasst.MakeEnergyCheck(feasst.args(
    {"steps_per" : str(steps_per),
     "tolerance" : "1e-10"})));
  mc.add(feasst.MakeTuner(feasst.args(
    {"steps_per" : str(steps_per)})));

