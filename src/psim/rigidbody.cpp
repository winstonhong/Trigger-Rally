
// rigidbody.cpp [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#include "psim.h"




PRigidBody::PRigidBody(PSim &sim_parent) : PReferenceFrame(), sim(sim_parent)
{
  linvel = vec3f::zero();
  angvel = vec3f::zero();

  mass = 1.0;
  mass_inv = 1.0;
  angmass = vec3f(1.0,1.0,1.0);
  angmass_inv = vec3f(1.0,1.0,1.0);

  accum_force = vec3f::zero();
  accum_torque = vec3f::zero();
}

PRigidBody::~PRigidBody()
{
}


void PRigidBody::setMassCuboid(float _mass, const vec3f &rad)
{
  if (mass <= 0.0 ||
    rad.x <= 0.0 ||
    rad.y <= 0.0 ||
    rad.z <= 0.0) return;

  mass = _mass;
  mass_inv = 1.0 / mass;

#if 1
  angmass = vec3f(rad.y*rad.z, rad.z*rad.x, rad.x*rad.y) * (mass * 0.4);
#else
  angmass = vec3f(1.0, 1.0, 1.0) * (mass * 4.0);
#endif

  angmass_inv.x = 1.0 / angmass.x;
  angmass_inv.y = 1.0 / angmass.y;
  angmass_inv.z = 1.0 / angmass.z;
}


void PRigidBody::addForce(const vec3f &frc)
{
  accum_force += frc;
}

void PRigidBody::addLocForce(const vec3f &frc)
{
  addForce(getLocToWorldVector(frc));
}

void PRigidBody::addForceAtPoint(const vec3f &frc, const vec3f &pt)
{
  accum_force += frc;

  vec3f wdiff = pt - pos;

  accum_torque += frc ^ wdiff;
  //accum_torque -= wdiff ^ frc;
}

void PRigidBody::addLocForceAtPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(getLocToWorldVector(frc), pt);
}

void PRigidBody::addForceAtLocPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(frc, getLocToWorldPoint(pt));
}

void PRigidBody::addLocForceAtLocPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(getLocToWorldVector(frc), getLocToWorldPoint(pt));
}

void PRigidBody::addTorque(const vec3f &trq)
{
  accum_torque += trq;
}

void PRigidBody::addLocTorque(const vec3f &trq)
{
  addTorque(getLocToWorldVector(trq));
}

vec3f PRigidBody::getLinearVelAtPoint(const vec3f &pt)
{
  vec3f usept = pt - pos;
  return (linvel + (usept ^ angvel));
}

vec3f PRigidBody::getLinearVelAtLocPoint(const vec3f &pt)
{
  return getLinearVelAtPoint(getLocToWorldPoint(pt));
}


//#define CLAMPVEL

void PRigidBody::tick(float delta)
{
  linvel += (accum_force * mass_inv + sim.gravity) * delta;

#ifdef CLAMPVEL
  CLAMP(linvel.x, -20.0, 20.0);
  CLAMP(linvel.y, -20.0, 20.0);
  CLAMP(linvel.z, -20.0, 20.0);
#endif

  pos += linvel * delta;

#if 0
  mat44f ori_mat2;
  ori_mat2.assemble(
    vec3f(ori_mat.row[0][0]*0.5+0.5, ori_mat.row[0][1]*0.5+0.5, ori_mat.row[0][2]*0.5+0.5),
    vec3f(ori_mat.row[1][0]*0.5+0.5, ori_mat.row[1][1]*0.5+0.5, ori_mat.row[1][2]*0.5+0.5),
    vec3f(ori_mat.row[2][0]*0.5+0.5, ori_mat.row[2][1]*0.5+0.5, ori_mat.row[2][2]*0.5+0.5));

  vec3f angmass_inv_world = ori_mat2.transform2(angmass_inv);
#else
  vec3f angmass_inv_world = angmass_inv;
#endif

  vec3f ang_accel = vec3f(
    accum_torque.x * angmass_inv_world.x,
    accum_torque.y * angmass_inv_world.y,
    accum_torque.z * angmass_inv_world.z);

  angvel += ang_accel * delta;

#ifdef CLAMPVEL
  CLAMP(angvel.x, -20.0, 20.0);
  CLAMP(angvel.y, -20.0, 20.0);
  CLAMP(angvel.z, -20.0, 20.0);
#endif

  quatf angdelta;
  angdelta.fromThreeAxisAngle(angvel * delta);

  ori = ori * angdelta;
  //ori = angdelta * ori;

  //PULLTOWARD(linvel, vec3f::zero(), delta * 0.1);
  //PULLTOWARD(angvel, vec3f::zero(), delta * 0.1);

  accum_force = vec3f::zero();
  accum_torque = vec3f::zero();

  PReferenceFrame::updateMatrices();
}




