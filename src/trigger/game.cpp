
// game.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



#include "main.h"


TriggerGame::TriggerGame(MainApp *parent)
{
  app = parent;
  
  sim = null;
  terrain = null;
  
  randomseed = 0;
}

TriggerGame::~TriggerGame()
{
  if (sim) delete sim;
  if (terrain) delete terrain;
}

bool TriggerGame::loadLevel(const std::string &filename)
{
  
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading level \"" << filename << "\"\n";

  if (sim == null)
    sim = new PSim();
  
  sim->setGravity(vec3f(0.0,0.0,-9.81));
  
  start_pos = vec3f::zero();
  start_ori = quatf::identity();
  
  targettime = 754.567f;
  
  weather.cloud.texname = std::string("");
  weather.cloud.scrollrate = 0.001f;
  weather.fog.color = vec3f(1.0f, 1.0f, 1.0f);
  weather.fog.density = 0.01f;
  weather.fog.density_sky = 0.8f;
  weather.precip.rain = 0.0f;
  
  TiXmlDocument xmlfile(filename.c_str());
  TiXmlElement *rootelem = PUtil::loadRootElement(xmlfile, "level");
  if (!rootelem) return false;
  
  const char *val;
  
  val = rootelem->Attribute("comment");
  if (val) comment = val;
  
  for (TiXmlElement *walk = rootelem->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {
    
    if (!strcmp(walk->Value(), "terrain")) {
      try
      {
        terrain = new PTerrain (walk, filename, app->getSSTexture ());
      }
      catch (PException e)
      {
        PUtil::outLog () << "Terrain problem: " << e.what () << std::endl;
        return false;
      }
      sim->setTerrain(terrain);
    } else if (!strcmp(walk->Value(), "vehicle")) {
      PVehicle *vh = sim->createVehicle(walk, filename, app->getSSModel());
      if (vh) {
        vehicle.push_back(vh);
      } else {
        PUtil::outLog() << "Warning: failed to load vehicle\n";
      }
    } else if (!strcmp(walk->Value(), "vehicleoption")) {
      
      val = walk->Attribute("type");
      
      if (val) {
        PVehicleType *vt = sim->loadVehicleType(PUtil::assemblePath(val, filename), app->getSSModel());
        if (vt) {
          vehiclechoices.push_back(vt);
        } else {
          PUtil::outLog() << "Warning: failed to load vehicle option\n";
        }
      } else {
        PUtil::outLog() << "Warning: vehicle option has no type\n";
      }
    } else if (!strcmp(walk->Value(), "race")) {
      
      // TODO: check race type... laps? once only?
      
      vec2f coordscale = vec2f(1.0f, 1.0f);
      val = walk->Attribute("coordscale");
      if (val) sscanf(val, "%f , %f", &coordscale.x, &coordscale.y);
      
      val = walk->Attribute("targettime");
      if (val) targettime = atof(val);
      
      for (TiXmlElement *walk2 = walk->FirstChildElement();
        walk2; walk2 = walk2->NextSiblingElement()) {
        
        if (!strcmp(walk2->Value(), "checkpoint")) {
          val = walk2->Attribute("coords");
          if (val) {
            vec2f coords;
            if (sscanf(val, "%f , %f", &coords.x, &coords.y) == 2) {
              coords.x *= coordscale.x;
              coords.y *= coordscale.y;
              checkpt.push_back(vec3f(coords.x, coords.y, terrain->getHeight(coords.x, coords.y)));
            } else
              PUtil::outLog() << "Error reading checkpoint coords\n";
          } else {
            PUtil::outLog() << "Warning: checkpoint has no coords\n";
          }
        } else if (!strcmp(walk2->Value(), "startposition")) {
          
          val = walk2->Attribute("pos");
          if (val) sscanf(val, "%f , %f , %f", &start_pos.x, &start_pos.y, &start_pos.z);
          
          start_pos.x *= coordscale.x;
          start_pos.y *= coordscale.y;
          
          val = walk2->Attribute("oridegrees");
          if (val) {
            float deg = atof(val);
            start_ori.fromZAngle(-RADIANS(deg));
          }
          
          val = walk2->Attribute("ori");
          if (val) sscanf(val, "%f , %f , %f , %f", &start_ori.w, &start_ori.x, &start_ori.y, &start_ori.z);
        }
      }
    } else if (!strcmp(walk->Value(), "weather")) {
      
      val = walk->Attribute("cloudtexture");
      if (val) weather.cloud.texname = PUtil::assemblePath(val, filename);
      
      val = walk->Attribute("cloudscrollrate");
      if (val) weather.cloud.scrollrate = atof(val);
      
      val = walk->Attribute("fogcolor");
      if (val) sscanf(val, "%f , %f , %f", &weather.fog.color.x, &weather.fog.color.y, &weather.fog.color.z);
      
      val = walk->Attribute("fogdensity");
      if (val) weather.fog.density = atof(val);
      
      val = walk->Attribute("fogdensitysky");
      if (val) weather.fog.density_sky = atof(val);
      
      val = walk->Attribute("rain");
      if (val) weather.precip.rain = atof(val);
    }
  }
  
  srand(1000);
  
  if (checkpt.size() == 0) {
    int cpsize = 3;
    
    std::vector<vec2f> temp1;
    
    temp1.resize(cpsize);
    
    float ang = randm11 * PI;
    
    temp1[0] = vec2f(cosf(ang),sinf(ang)) * (100.0f + rand01 * 300.0f);
    
    for (int i=1; i<cpsize; i++) {
      ang += randm11 * (PI * 0.3f);
      
      temp1[i] = temp1[i-1] + vec2f(cosf(ang),sinf(ang)) * (100.0f + rand01 * 300.0f);
    }
    
    checkpt.resize(cpsize, vec3f::zero());
    
    for (int i=0; i<cpsize; i++) {
      vec2f coords = temp1[i];
      checkpt[i].pt = vec3f(coords.x, coords.y, terrain->getHeight(coords.x, coords.y));
    }
  }
  
  for (unsigned int i=0; i<vehicle.size(); i++) {
    vehicle[i]->ctrl.brake1 = 1.0f;
  }
  
  // get cars on ground  
  for (float t = 0.0f; t < 2.0f; t += 0.01f)
    sim->tick(0.01f);
  
  /*
  for (int i=1; i<vehicle.size(); i++) {
    aid.push_back(AIDriver(i));
  }
  */
  
  coursetime = 0.0f;
  
  othertime = 3.0f;
  
  cptime = -4.0f;
  
  gamestate = GS_COUNTDOWN;
  
  return true;
}

void TriggerGame::chooseVehicle(PVehicleType *type)
{
  vec3f pos = vec3f::zero();
  
  quatf ori = quatf::identity();
  
  PVehicle *vh = sim->createVehicle(type, start_pos, start_ori, app->getSSModel());
  
  if (vh) vehicle.push_back(vh);
  else PUtil::outLog() << "Warning: failed to load vehicle\n";
}

void TriggerGame::tick(float delta)
{
  
  switch (gamestate) {
  case GS_COUNTDOWN:
    othertime -= delta;
    if (othertime <= 0.0f) {
      othertime = 5.0f;
      gamestate = GS_RACING;
    }
    
    for (unsigned int i=0; i<vehicle.size(); i++) {
      vehicle[i]->ctrl.setZero();
      vehicle[i]->ctrl.brake1 = 1.0f;
      vehicle[i]->ctrl.brake2 = 1.0f;
    }
    //return;
    break;
  case GS_RACING:
    coursetime += delta;
    
    if (coursetime > targettime) {
      gamestate = GS_FINISHED;
    }
    break;
  case GS_FINISHED:
    othertime -= delta;
    
    for (unsigned int i=0; i<vehicle.size(); i++) {
      vehicle[i]->ctrl.setZero();
      vehicle[i]->ctrl.brake1 = 1.0f;
      vehicle[i]->ctrl.brake2 = 1.0f;
    }
    
    break;
  }
  
  //vehic->getBody().addLocTorque(vec3f(50.0,0.0,0.0));
  
  sim->tick(delta);
  
  for (unsigned int i=0; i<vehicle.size(); i++) {
    
    vec2f diff = makevec2f(checkpt[vehicle[i]->nextcp].pt) - makevec2f(vehicle[i]->body->getPosition());
    if (diff.lengthsq() < 30.0f * 30.0f) {
      //vehicle[i]->nextcp = (vehicle[i]->nextcp + 1) % checkpt.size();
        cptime = coursetime;
      if (++vehicle[i]->nextcp >= (int)checkpt.size()) {
        vehicle[i]->nextcp = 0;
        if (i == 0) gamestate = GS_FINISHED;
      }
    }
  }
  
  /*
  for (int i=0; i<aid.size(); i++) {
    PVehicle *vehic = vehicle[aid[i].vehic];
    
    vec2f diff = makevec2f(checkpt[vehic->nextcp].pt) - makevec2f(vehic->body->getPosition());
    float diffangle = -atan2(diff.y, diff.x);
    
    vec2f diff2 = makevec2f(checkpt[(vehic->nextcp+1)%checkpt.size()].pt) - makevec2f(checkpt[vehic->nextcp].pt);
    float diff2angle = -atan2(diff2.y, diff2.x);
    
    vec3f forw = makevec3f(vehic->body->getOrientationMatrix().row[0]);
    float forwangle = atan2(forw.y, forw.x);
    
    float correction = diffangle - forwangle + PI*0.5f + vehic->body->getAngularVel().z * -1.0f;
    
    #if 0
    float fact = diff.length() * 0.01f;
    if (fact > 0.3f) fact = 0.3f;
    correction += (diffangle - diff2angle) * fact;
    #endif
    
    if (correction >= PI) { correction -= PI*2.0f; if (correction >= PI) correction -= PI*2.0f; }
    if (correction < -PI) { correction += PI*2.0f; if (correction < -PI) correction += PI*2.0f; }
    
    vehic->ctrl.turn.z = correction * 2.0f + randm11 * 0.2f;
    
    vehic->ctrl.throttle = 1.0f - fabsf(correction) * 0.8f;
    if (vehic->ctrl.throttle < 0.1f)
      vehic->ctrl.throttle = 0.1f;
    
    vehic->ctrl.brake1 = 0.0f;
  }
  */
}



