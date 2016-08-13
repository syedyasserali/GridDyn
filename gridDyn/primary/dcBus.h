/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef DCBUS_H_
#define DCBUS_H_

// headers
#include "acBus.h"

// forward classes

/** @brief class implements a DC powered bus
 the DC has only one state (voltage) and can only attach to links that are DC capable
*/
class dcBus : public acBus
{
public:
protected:
public:
  dcBus (const std::string &objName = "dcBus_$");
  ~dcBus ();

  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // add components
  using acBus::add;
  virtual int add (gridLink *lnk) override;  //this add function checks for DC capable links

  // initializeB

  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;
protected:
  virtual  void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual  void pFlowObjectInitializeB () override;
public:
  virtual  change_code powerFlowAdjust (unsigned long flags, check_level_t level) override;      //only applicable in pFlow
  //ual  void powerAdjust(double adjustment);
  virtual  void pFlowCheck (std::vector<violation> &Violation_vector) override;
  //initializeB dynamics
protected:
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeB (IOdata &outputSet) override;
public:
  // parameter set functions
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;


  virtual void jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void converge (double ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode mode = converge_mode::local_iteration,double tol = 0.01) override;

  virtual double timestep (double ttime, const solverMode &sMode) override;

  double getAngle (const stateData *, const solverMode &) const final;  //no angle so no reason to inherit beyond this
  virtual bool useVoltage (const solverMode &sMode) override;
  virtual int getMode (const solverMode &sMode) override;
  virtual int propogatePower (bool makeSlack = false) override;
protected:
};


#endif
