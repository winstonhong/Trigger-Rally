
// psim.h [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "../pengine/pengine.h"

/*
#include <tnl/tnl.h>
#include <tnl/tnlNetBase.h>
#include <tnl/tnlGhostConnection.h>
#include <tnl/tnlNetInterface.h>
#include <tnl/tnlNetObject.h>
*/


class PSim;
class   PRigidBody;

class PMotor;
class PGearbox;

class ClipSet;
class ClipNode;
class ClipMesh;

class PVehicleType;
class   PVehicleTypePart;
class PVehicle;



class PReferenceFrame {
public:
  vec3f pos;
  quatf ori;

  mat44f ori_mat, ori_mat_inv;

public:
  PReferenceFrame() : pos(vec3f::zero()), ori(quatf::identity()) {
    updateMatrices();
  }

  void updateMatrices() {
    ori.normalize();
    ori_mat = ori.getMatrix();
    ori_mat_inv = ori_mat.transpose();
  }

  void setPosition(const vec3f &_pos) { pos = _pos; }
  vec3f getPosition() { return pos; }

  void setOrientation(const quatf &_ori) { ori = _ori; }
  quatf getOrientation() { return ori; }
  mat44f getOrientationMatrix() { return ori_mat; }
  mat44f getInverseOrientationMatrix() { return ori_mat_inv; }

  vec3f getLocToWorldVector(const vec3f &pt) {
    return ori_mat.transform1(pt);
  }
  vec3f getWorldToLocVector(const vec3f &pt) {
    return ori_mat.transform2(pt);
  }
  vec3f getLocToWorldPoint(const vec3f &pt) {
    return pos + ori_mat.transform1(pt);
  }
  vec3f getWorldToLocPoint(const vec3f &pt) {
    return ori_mat.transform2(pt - pos);
  }
};


class PRigidBody : public PReferenceFrame {
private:

  PSim &sim;

  // config
  float mass, mass_inv;
  vec3f angmass, angmass_inv; // inertial tensor approximation

  // state
  vec3f linvel;
  vec3f angvel;

  vec3f accum_force;
  vec3f accum_torque;

  // TODO: intelligent friction calc

public:
  PRigidBody(PSim &sim_parent);
  ~PRigidBody();

public:
  void setMassCuboid(float _mass, const vec3f &dim);

  void setLinearVel(const vec3f &vel) { linvel = vel; }
  const vec3f &getLinearVel() { return linvel; }

  void setAngularVel(const vec3f &vel) { angvel = vel; }
  const vec3f &getAngularVel() { return angvel; }

  void addForce(const vec3f &frc);
  void addLocForce(const vec3f &frc);
  void addForceAtPoint(const vec3f &frc, const vec3f &pt);
  void addLocForceAtPoint(const vec3f &frc, const vec3f &pt);
  void addForceAtLocPoint(const vec3f &frc, const vec3f &pt);
  void addLocForceAtLocPoint(const vec3f &frc, const vec3f &pt);

  void addTorque(const vec3f &trq);
  void addLocTorque(const vec3f &trq);

  vec3f getLinearVelAtPoint(const vec3f &pt);
  vec3f getLinearVelAtLocPoint(const vec3f &pt);

  void tick(float delta);

  friend class PSim;
};




#include "vehicle.h"



class PSim {
private:
  
  PTerrain *terrain;
  
  PResourceList<PVehicleType> vtypelist;
  
  std::vector<PRigidBody *> body;
  
  std::vector<PVehicle *> vehicle;
  
  vec3f gravity;
  
public:
  PSim();
  ~PSim();
  
public:
  void setTerrain(PTerrain *_terrain) { terrain = _terrain; }
  
  void setGravity(const vec3f &_gravity) { gravity = _gravity; }
  
  PVehicleType *loadVehicleType(const std::string &filename, PSSModel &ssModel);
  
  PRigidBody *createRigidBody();
  
  PVehicle *createVehicle(TiXmlElement *element, const std::string &filepath, PSSModel &ssModel);
  PVehicle *createVehicle(const std::string &type, const vec3f &pos, const quatf &ori, const std::string &filepath, PSSModel &ssModel);
  PVehicle *createVehicle(PVehicleType *type, const vec3f &pos, const quatf &ori, PSSModel &ssModel);
  
  // Remove all bodies and vehicles
  void clear();
  
  // Step the simulation delta seconds
  void tick(float delta);
  
  
  PTerrain *getTerrain() { return terrain; }
  
public:
  
  friend class PRigidBody;
  friend class PVehicle;
};
