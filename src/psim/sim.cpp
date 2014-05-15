
// psim.cpp [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#include "psim.h"



PSim::PSim() : terrain(null), gravity(vec3f::zero())
{
}


PSim::~PSim()
{
  clear();
}



PVehicleType *PSim::loadVehicleType(const std::string &filename, PSSModel &ssModel)
{
  PVehicleType *vtype = vtypelist.find(filename);
  if (!vtype) {
    vtype = new PVehicleType();
    if (!vtype->load(filename, ssModel)) {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << filename << "\n";
      return null;
    }
    vtypelist.add(vtype);
  }
  return vtype;
}


PRigidBody *PSim::createRigidBody()
{
  PRigidBody *newbody = new PRigidBody(*this);

  body.push_back(newbody);

  return newbody;
}

PVehicle *PSim::createVehicle(TiXmlElement *element, const std::string &filepath, PSSModel &ssModel)
{
  const char *val;
  
  const char *type = element->Attribute("type");
  if (!type) {
    PUtil::outLog() << "Vehicle has no type\n";
    return null;
  }
  
  vec3f pos = vec3f::zero();
  
  val = element->Attribute("pos");
  if (val) sscanf(val, "%f , %f , %f", &pos.x, &pos.y, &pos.z);
  
  quatf ori = quatf::identity();
  
  val = element->Attribute("ori");
  if (val) sscanf(val, "%f , %f , %f , %f", &ori.w, &ori.x, &ori.y, &ori.z);
  
  return createVehicle(type, pos, ori, filepath, ssModel);
}

PVehicle *PSim::createVehicle(const std::string &type, const vec3f &pos, const quatf &ori, const std::string &filepath, PSSModel &ssModel)
{
  PVehicleType *vtype = loadVehicleType(PUtil::assemblePath(type, filepath), ssModel);
  
  return createVehicle(vtype, pos, ori, ssModel);
}

PVehicle *PSim::createVehicle(PVehicleType *type, const vec3f &pos, const quatf &ori, PSSModel &ssModel)
{
  PSSModel *unused = &ssModel; unused = unused;
  
  if (!type) return null;
  
  PVehicle *newvehicle = new PVehicle(*this, type);
  
  vec3f vpos = pos;
  if (terrain) vpos.z += terrain->getHeight(vpos.x, vpos.y);
  newvehicle->getBody().setPosition(vpos);
  
  newvehicle->getBody().setOrientation(ori);
  newvehicle->getBody().updateMatrices();
  
  newvehicle->updateParts();
  
  vehicle.push_back(newvehicle);
  return newvehicle;
}

void PSim::clear()
{
  for (unsigned int i=0; i<body.size(); ++i)
    delete body[i];
  body.clear();

  for (unsigned int i=0; i<vehicle.size(); ++i)
    delete vehicle[i];
  vehicle.clear();

  vtypelist.clear();
}


void PSim::tick(float delta)
{
  if (delta <= 0.0) return;

  //lta *= 0.1;

  float timeslice = 0.005;
  int num = (int)(delta / timeslice) + 1;
  timeslice = delta / (float)num;

  for (int timestep=0; timestep<num; ++timestep) {
    for (unsigned int i=0; i<vehicle.size(); ++i) {
      vehicle[i]->tick(timeslice);
    }

    for (unsigned int i=0; i<body.size(); ++i) {
      body[i]->tick(timeslice);
    }

    for (unsigned int i=0; i<vehicle.size(); ++i) {
      vehicle[i]->updateParts();
    }
  }
}



