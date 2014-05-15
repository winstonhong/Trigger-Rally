
// main.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



#include "main.h"

#include <SDL/SDL_thread.h>


void MainApp::config()
{
  PUtil::setDebugLevel(DEBUGLEVEL_DEVELOPER);
  
  loadConfig();
  
  setScreenMode(cfg_video_cx, cfg_video_cy, cfg_video_fullscreen);
}

void MainApp::load()
{
  psys_dirt = null;
  
  audinst_engine = null;
  audinst_wind = null;
  audinst_gravel = null;
  game = null;
  
  // use PUtil, not boost
  //std::string buff = boost::str(boost::format("textures/splash/splash%u.jpg") % ((rand() % 3) + 1));
  //if (!(tex_splash_screen = getSSTexture().loadTexture(buff))) return false;
  
  if (!(tex_splash_screen = getSSTexture().loadTexture("textures/splash/splash.jpg"))) throw MakePException ("Failed to load splash screen");
  
  appstate = AS_LOAD_1;
  
  loadscreencount = 3;
  
  splashtimeout = 0.0f;
  
  // Check that controls are available where requested
  // (can't be done in config because joy info not available)
  
  for (int i = 0; i < ActionCount; i++) {
    
    switch(ctrl.map[i].type) {
    case UserControl::TypeUnassigned:
      break;
      
    case UserControl::TypeKey:
      if (ctrl.map[i].key.sym <= 0 || ctrl.map[i].key.sym >= SDLK_LAST)
        ctrl.map[i].type = UserControl::TypeUnassigned;
      break;
      
    case UserControl::TypeJoyButton:
      if (0 >= getNumJoysticks() || ctrl.map[i].joybutton.button >= getJoyNumButtons(0))
        ctrl.map[i].type = UserControl::TypeUnassigned;
      break;
      
    case UserControl::TypeJoyAxis:
      if (0 >= getNumJoysticks() || ctrl.map[i].joyaxis.axis >= getJoyNumAxes(0))
        ctrl.map[i].type = UserControl::TypeUnassigned;
      break;
    }
  }
}

void MainApp::loadConfig()
{
  PUtil::outLog() << "Loading game configuration" << std::endl;
  
  // Set defaults
  
  cfg_video_cx = 640;
  cfg_video_cy = 480;
  cfg_video_fullscreen = false;
  
  cfg_drivingassist = 1.0f;
  cfg_enable_sound = true;
  cfg_speed_unit = mph;
  cfg_speed_style = analogue;
  
  hud_speedo_start_deg = MPH_ZERO_DEG;
  hud_speedo_mps_deg_mult = MPS_MPH_DEG_MULT;
  hud_speedo_mps_speed_mult = MPS_MPH_SPEED_MULT;

  ctrl.action_name[ActionForward] = std::string("forward");
  ctrl.action_name[ActionBack] = std::string("back");
  ctrl.action_name[ActionLeft] = std::string("left");
  ctrl.action_name[ActionRight] = std::string("right");
  ctrl.action_name[ActionHandbrake] = std::string("handbrake");
  ctrl.action_name[ActionRecover] = std::string("recover");
  ctrl.action_name[ActionCamMode] = std::string("cammode");
  ctrl.action_name[ActionCamLeft] = std::string("camleft");
  ctrl.action_name[ActionCamRight] = std::string("camright");
  ctrl.action_name[ActionShowMap] = std::string("showmap");
  
  for (int i = 0; i < ActionCount; i++) {
    ctrl.map[i].type = UserControl::TypeUnassigned;
    ctrl.map[i].value = 0.0f;
  }
  
  // Do config file management
  
  std::string cfgfilename = "trigger.config";
  
  if (!PHYSFS_exists(cfgfilename.c_str())) {
    
    PUtil::outLog() << "No user config file, copying over defaults" << std::endl;
    
    std::string cfgdefaults = "trigger.config.defs";
    
    if (!PUtil::copyFile(cfgdefaults, cfgfilename)) {
    
      PUtil::outLog() << "Couldn't create user config file. Proceeding with defaults." << std::endl;
      
      cfgfilename = cfgdefaults;
    }
  }
  
  // Load actual settings from file
  
  TiXmlDocument xmlfile(cfgfilename.c_str());
  
  TiXmlElement *rootelem = PUtil::loadRootElement(xmlfile, "config");
  if (!rootelem) {
    PUtil::outLog() << "Error: Couldn't load configuration file" << std::endl;
    PUtil::outLog() << "TinyXML: " << xmlfile.ErrorDesc() << std::endl;
    PUtil::outLog() << "Your data paths are probably not set up correctly" << std::endl;
    throw MakePException ("Boink");
  }
  
  const char *val;
  
  for (TiXmlElement *walk = rootelem->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {
    
    if (!strcmp(walk->Value(), "video")) {
      
      val = walk->Attribute("width");
      if (val) cfg_video_cx = atoi(val);
      
      val = walk->Attribute("height");
      if (val) cfg_video_cy = atoi(val);
      
      val = walk->Attribute("fullscreen");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_video_fullscreen = true;
        else if (!strcmp(val, "no"))
          cfg_video_fullscreen = false;
      }
      
      val = walk->Attribute("requirergb");
      if (val) {
        if (!strcmp(val, "yes"))
          requireRGB(true);
        else if (!strcmp(val, "no"))
          requireRGB(false);
      }
      
      val = walk->Attribute("requirealpha");
      if (val) {
        if (!strcmp(val, "yes"))
          requireAlpha(true);
        else if (!strcmp(val, "no"))
          requireAlpha(false);
      }
      
      val = walk->Attribute("requiredepth");
      if (val) {
        if (!strcmp(val, "yes"))
          requireDepth(true);
        else if (!strcmp(val, "no"))
          requireDepth(false);
      }
      
      val = walk->Attribute("requirestencil");
      if (val) {
        if (!strcmp(val, "yes"))
          requireStencil(true);
        else if (!strcmp(val, "no"))
          requireStencil(false);
      }
      
      val = walk->Attribute("stereo");
      if (val) {
        if (!strcmp(val, "none"))
          setStereoMode(PApp::StereoNone);
        else if (!strcmp(val, "quadbuffer"))
          setStereoMode(PApp::StereoQuadBuffer);
        else if (!strcmp(val, "red-blue"))
          setStereoMode(PApp::StereoRedBlue);
        else if (!strcmp(val, "red-green"))
          setStereoMode(PApp::StereoRedGreen);
        else if (!strcmp(val, "red-cyan"))
          setStereoMode(PApp::StereoRedCyan);
        else if (!strcmp(val, "yellow-blue"))
          setStereoMode(PApp::StereoYellowBlue);
      }
      
      float sepMult = 1.0f;
      val = walk->Attribute("stereoswapeyes");
      if (val && !strcmp(val, "yes"))
        sepMult = -1.0f;
      
      val = walk->Attribute("stereoeyeseparation");
      if (val) {
        setStereoEyeSeperation(atof(val) * sepMult);
      }
      
    } else if (!strcmp(walk->Value(), "parameters")) {
      
      val = walk->Attribute("drivingassist");
      if (val) cfg_drivingassist = atof(val);
      
      val = walk->Attribute("enablesound");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_enable_sound = true;
        else if (!strcmp(val, "no"))
          cfg_enable_sound = false;
      }

      val = walk->Attribute("speedunit");
      if (val) {
        if (!strcmp(val, "mph")) {
            cfg_speed_unit = mph;
            hud_speedo_start_deg = MPH_ZERO_DEG;
            hud_speedo_mps_deg_mult = MPS_MPH_DEG_MULT;
            hud_speedo_mps_speed_mult = MPS_MPH_SPEED_MULT;
          }
        else if (!strcmp(val, "kph")) {
           cfg_speed_unit = kph;
           hud_speedo_start_deg = KPH_ZERO_DEG;
           hud_speedo_mps_deg_mult = MPS_KPH_DEG_MULT;
           hud_speedo_mps_speed_mult = MPS_KPH_SPEED_MULT;
         }
      }
      val = walk->Attribute("speedstyle");
      if (val) {
        if (!strcmp(val, "analogue")) {
          cfg_speed_style = analogue;
        }
        else if (!strcmp(val, "hybrid")) {
          cfg_speed_style = hybrid;
        }
      }

    } else if (!strcmp(walk->Value(), "controls")) {
      
      for (TiXmlElement *walk2 = walk->FirstChildElement();
        walk2; walk2 = walk2->NextSiblingElement()) {
        
        if (!strcmp(walk2->Value(), "keyboard")) {
          
          val = walk2->Attribute("enable");
          if (val && !strcmp(val, "no"))
            continue;
          
          for (TiXmlElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {
            
            if (!strcmp(walk3->Value(), "key")) {
              
              val = walk3->Attribute("action");
              
              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;
              
              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }
              
              // TODO: implement string to keycode mapping
              val = walk3->Attribute("code");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Key has no code" << std::endl;
                continue;
              }
              
              ctrl.map[a].type = UserControl::TypeKey;
              ctrl.map[a].key.sym = (SDLKey) atoi(val);
            }
          }
          
        } else if (!strcmp(walk2->Value(), "joystick")) {
          
          val = walk2->Attribute("enable");
          if (val && !strcmp(val, "no"))
            continue;
          
          for (TiXmlElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {
            
            if (!strcmp(walk3->Value(), "button")) {
              
              val = walk3->Attribute("action");
              
              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;
              
              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }
              
              val = walk3->Attribute("index");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy button has no index" << std::endl;
                continue;
              }
              
              ctrl.map[a].type = UserControl::TypeJoyButton;
              ctrl.map[a].joybutton.button = atoi(val);
              
            } else if (!strcmp(walk3->Value(), "axis")) {
              
              val = walk3->Attribute("action");
              
              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;
              
              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }
              
              val = walk3->Attribute("index");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy axis has no index" << std::endl;
                continue;
              }
              
              int index = atoi(val);
              
              bool positive;
              
              val = walk3->Attribute("direction");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy axis has no direction" << std::endl;
                continue;
              }
              if (!strcmp(val, "+"))
                positive = true;
              else if (!strcmp(val, "-"))
                positive = false;
              else {
                PUtil::outLog() << "Config ctrls: Joy axis direction \"" << val <<
                  "\" is neither \"+\" nor \"-\"" << std::endl;
                continue;
              }
              
              ctrl.map[a].type = UserControl::TypeJoyAxis;
              ctrl.map[a].joyaxis.axis = index;
              ctrl.map[a].joyaxis.sign = positive ? 1.0f : -1.0f;
              ctrl.map[a].joyaxis.deadzone = 0.0f;
              ctrl.map[a].joyaxis.maxrange = 1.0f;
              
              val = walk3->Attribute("deadzone");
              if (val) ctrl.map[a].joyaxis.deadzone = atof(val);
              
              val = walk3->Attribute("maxrange");
              if (val) ctrl.map[a].joyaxis.maxrange = atof(val);
            }
          }
        }
      }
    }
  }
}

bool MainApp::loadLevel(TriggerLevel &tl)
{
  tl.name = "Untitled";
  tl.comment = "";
  tl.author = "";
  tl.targettime = "";
  
  TiXmlDocument xmlfile(tl.filename.c_str());
  TiXmlElement *rootelem = PUtil::loadRootElement(xmlfile, "level");
  if (!rootelem) {
    PUtil::outLog() << "Couldn't read level \"" << tl.filename << "\"" << std::endl;
    return false;
  }
  
  const char *val;
  
  val = rootelem->Attribute("name");
  if (val) tl.name = val;
  val = rootelem->Attribute("comment");
  if (val) tl.comment = val;
  val = rootelem->Attribute("author");
  if (val) tl.author = val;
  for (TiXmlElement *walk = rootelem->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {
    
    if (!strcmp(walk->Value(), "race")) {
      val = walk->Attribute("targettime");
      if (val) tl.targettime = PUtil::formatTime(atof(val));

    }
  }
  
  return true;
}

bool MainApp::loadLevelsAndEvents()
{
  PUtil::outLog() << "Loading levels and events" << std::endl;
  
  // Find levels
  
  std::list<std::string> results = PUtil::findFiles("/maps", ".level");
  
  for (std::list<std::string>::iterator i = results.begin();
    i != results.end(); i++) {
    
    TriggerLevel tl;
    tl.filename = *i;
    
    if (!loadLevel(tl)) continue;
    
    // Insert level in alphabetical order
    std::vector<TriggerLevel>::iterator j = levels.begin();
    while (j != levels.end() && j->name < tl.name) j++;
    levels.insert(j, tl);
  }
  
  // Find events
  
  results = PUtil::findFiles("/events", ".event");
  
  for (std::list<std::string>::iterator i = results.begin();
    i != results.end(); i++) {
    
    TriggerEvent te;
    
    TiXmlDocument xmlfile(i->c_str());
    TiXmlElement *rootelem = PUtil::loadRootElement(xmlfile, "event");
    if (!rootelem) {
      PUtil::outLog() << "Couldn't read event \"" << *i << "\"" << std::endl;
      continue;
    }
    
    const char *val;
    
    val = rootelem->Attribute("name");
    if (val) te.name = val;
    val = rootelem->Attribute("comment");
    if (val) te.comment = val;
    val = rootelem->Attribute("author");
    if (val) te.author = val;
    
    for (TiXmlElement *walk = rootelem->FirstChildElement();
      walk; walk = walk->NextSiblingElement()) {
      
      if (!strcmp(walk->Value(), "level")) {
        
        TriggerLevel tl;
        
        val = walk->Attribute("file");
        if (!val) {
          PUtil::outLog() << "Warning: Event level has no filename" << std::endl;
          continue;
        }
        tl.filename = PUtil::assemblePath(val, *i);
        
        if (loadLevel(tl))
          te.levels.push_back(tl);
          
        PUtil::outLog() << tl.filename << std::endl;
      }
    }
    
    if (te.levels.size() <= 0) {
      PUtil::outLog() << "Warning: Event has no levels" << std::endl;
      continue;
    }
    
    // Insert event in alphabetical order
    std::vector<TriggerEvent>::iterator j = events.begin();
    while (j != events.end() && j->name < te.name) j++;
    events.insert(j, te);
  }
  
  return true;
}

bool MainApp::loadAll()
{
  if (!(tex_font = getSSTexture().loadTexture("/textures/consolefont.png"))) return false;
  
  if (!(tex_end_screen = getSSTexture().loadTexture("/textures/splash/endgame.jpg"))) return false;
  
  if (!(tex_hud_life = getSSTexture().loadTexture("/textures/life_helmet.png"))) return false;
  
  if (!(tex_detail = getSSTexture().loadTexture("/textures/detail.jpg"))) return false;
  if (!(tex_dirt = getSSTexture().loadTexture("/textures/dust.png"))) return false;
  if (!(tex_shadow = getSSTexture().loadTexture("/textures/shadow.png", true, true))) return false;
  
  if (!(tex_hud_revs = getSSTexture().loadTexture("/textures/dial_rev.png"))) return false;
  if (cfg_speed_unit == mph) {
    if (cfg_speed_style) {
      if (!(tex_hud_speedo = getSSTexture().loadTexture("/textures/dial_speed_hybrid_mph.png"))) return false;
    } else {
      if (!(tex_hud_speedo = getSSTexture().loadTexture("/textures/dial_speed_mph.png"))) return false;
    }
  } else if (cfg_speed_unit == kph) {
    if (cfg_speed_style) {
      if (!(tex_hud_speedo = getSSTexture().loadTexture("/textures/dial_speed_hybrid_kph.png"))) return false;
    } else {
      if (!(tex_hud_speedo = getSSTexture().loadTexture("/textures/dial_speed_kph.png"))) return false;
    }
  }
  if (!(tex_hud_gear = getSSTexture().loadTexture("/textures/dial_gear.png"))) return false;
  
  if (cfg_enable_sound) {
    if (!(aud_engine = getSSAudio().loadSample("/sounds/engine.wav", false))) return false;
    if (!(aud_wind = getSSAudio().loadSample("/sounds/wind.wav", false))) return false;
    if (!(aud_gearchange = getSSAudio().loadSample("/sounds/gear.wav", false))) return false;
    if (!(aud_gravel = getSSAudio().loadSample("/sounds/gravel.wav", false))) return false;
    if (!(aud_crash1 = getSSAudio().loadSample("/sounds/bang.wav", false))) return false;
  }
  
  if (!loadLevelsAndEvents()) {
    PUtil::outLog() << "Couldn't load levels/events" << std::endl;
    return false;
  }
  
  //quatf tempo;
  //tempo.fromThreeAxisAngle(vec3f(-0.38, -0.38, 0.0));
  //vehic->getBody().setOrientation(tempo);
  
  campos = campos_prev = vec3f(-15.0,0.0,30.0);
  //camori.fromThreeAxisAngle(vec3f(-1.0,0.0,1.5));
  camori = quatf::identity();
  
  camvel = vec3f::zero();
  
  cloudscroll = 0.0f;
  
  cprotate = 0.0f;
  
  cameraview = 0;
  camera_user_angle = 0.0f;
  
  showmap = true;
  
  crashnoise_timeout = 0.0f;
  
  psys_dirt = new DirtParticleSystem();
  psys_dirt->setColorStart(0.5f, 0.4f, 0.2f, 1.0f);
  psys_dirt->setColorEnd(0.5f, 0.4f, 0.2f, 0.0f);
  psys_dirt->setSize(0.1f, 0.5f);
  psys_dirt->setDecay(7.0f);
  psys_dirt->setTexture(tex_dirt);
  psys_dirt->setBlend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  //
  
  choose_type = 0;
  
  choose_spin = 0.0f;
  
  return true;
}

void MainApp::unload()
{
  endGame(GF_NOT_FINISHED);
  
  delete psys_dirt;
}

bool MainApp::startGame(const std::string &filename)
{
  PUtil::outLog() << "Starting level \"" << filename << "\"" << std::endl;
  
  grabMouse(true);
  
  game = new TriggerGame(this);
  
  if (!game->loadLevel(filename)) {
    PUtil::outLog() << "Error: failed to load level" << std::endl;
    return false;
  }
  
  choose_type = game->vehiclechoices.size() - 1;
  
  if (game->vehiclechoices.size() > 1) {
    appstate = AS_CHOOSE_VEHICLE;
  } else {
    game->chooseVehicle(game->vehiclechoices[choose_type]);
    startGame2();
    appstate = AS_IN_GAME;
  }
  
  tex_sky[0] = null;
  
  if (game->weather.cloud.texname.length() > 0)
    tex_sky[0] = getSSTexture().loadTexture(game->weather.cloud.texname);
  
  if (tex_sky[0] == null) {
    tex_sky[0] = getSSTexture().loadTexture("textures/sky/blue.jpg");
    
    if (tex_sky[0] == null) tex_sky[0] = tex_detail; // last fallback...
  }
  
  return true;
}

void MainApp::startGame2()
{
  if (cfg_enable_sound) {
    audinst_engine = new PAudioInstance(aud_engine, true);
    audinst_engine->setGain(0.0);
    audinst_engine->play();
    
    audinst_wind = new PAudioInstance(aud_wind, true);
    audinst_wind->setGain(0.0);
    audinst_wind->play();
    
    audinst_gravel = new PAudioInstance(aud_gravel, true);
    audinst_gravel->setGain(0.0);
    audinst_gravel->play();
  }
}

void MainApp::endGame(int gamestate)
{
  float coursetime = (gamestate == GF_NOT_FINISHED) ? 0.0f : game->coursetime;
  
  if (audinst_engine) {
    delete audinst_engine;
    audinst_engine = null;
  }
  
  if (audinst_wind) {
    delete audinst_wind;
    audinst_wind = null;
  }
  
  if (audinst_gravel) {
    delete audinst_gravel;
    audinst_gravel = null;
  }
  
  for (unsigned int i=0; i<audinst.size(); i++) {
    delete audinst[i];
  }
  audinst.clear();
  
  if (game) {
    delete game;
    game = null;
  }
  
  finishRace(gamestate, coursetime);
}

void MainApp::tick(float delta)
{
  switch (appstate) {
  case AS_LOAD_1:
    splashtimeout -= delta;
    if (--loadscreencount <= 0)
      appstate = AS_LOAD_2;
    break;
  case AS_LOAD_2:
    splashtimeout -= delta;
    if (!loadAll()) {
      requestExit();
      return;
    }
    appstate = AS_LOAD_3;
    break;
  case AS_LOAD_3:
    splashtimeout -= delta;
    if (splashtimeout <= 0.0f)
      levelScreenAction(AA_INIT, 0);
    break;
  
  case AS_LEVEL_SCREEN:
    tickStateLevel(delta);
    break;
    
  case AS_CHOOSE_VEHICLE:
    tickStateChoose(delta);
    break;
  
  case AS_IN_GAME:
    tickStateGame(delta);
    break;
  
  case AS_END_SCREEN:
    splashtimeout += delta * 0.04f;
    if (splashtimeout >= 1.0f)
      requestExit();
    break;
  }
}

void MainApp::tickStateChoose(float delta)
{
  choose_spin += delta * 2.0f;
}

void MainApp::tickStateGame(float delta)
{
  PVehicle *vehic = game->vehicle[0];
  
  if (game->isFinished()) {
    
    int gfs = game->getFinishState();
    
    endGame(gfs);
    
    return;
  }
  
  cloudscroll = fmodf(cloudscroll + delta * game->weather.cloud.scrollrate, 1.0f);
  
  cprotate = fmodf(cprotate + delta * 1.0f, 1000.0f);
  
  // Do input/control processing
  
  for (int a = 0; a < ActionCount; a++) {
    
    switch(ctrl.map[a].type) {
    case UserControl::TypeUnassigned:
      break;
      
    case UserControl::TypeKey:
      ctrl.map[a].value = keyDown(ctrl.map[a].key.sym) ? 1.0f : 0.0f;
      break;
      
    case UserControl::TypeJoyButton:
      ctrl.map[a].value = getJoyButton(0, ctrl.map[a].joybutton.button) ? 1.0f : 0.0f;
      break;
      
    case UserControl::TypeJoyAxis:
      ctrl.map[a].value = ctrl.map[a].joyaxis.sign *
        getJoyAxis(0, ctrl.map[a].joyaxis.axis);
        
      RANGEADJUST(ctrl.map[a].value, ctrl.map[a].joyaxis.deadzone, ctrl.map[a].joyaxis.maxrange, 0.0f, 1.0f);
      
      CLAMP_LOWER(ctrl.map[a].value, 0.0f);
      break;
    }
  }
  
  // Bit of a hack for turning, because you simply can't handle analogue
  // and digital steering the same way, afaics
  
  if (ctrl.map[ActionLeft].type == UserControl::TypeJoyAxis ||
    ctrl.map[ActionRight].type == UserControl::TypeJoyAxis) {
    
    // Analogue mode
    
    vehic->ctrl.turn.z = 0.0f;
    vehic->ctrl.turn.z -= ctrl.map[ActionLeft].value;
    vehic->ctrl.turn.z += ctrl.map[ActionRight].value;
    
  } else {
    
    // Digital mode
    
    static float turnaccel = 0.0f;
    
    if (ctrl.map[ActionLeft].value > 0.0f) {
      if (turnaccel > -0.0f) turnaccel = -0.0f;
      turnaccel -= 8.0f * delta;
      vehic->ctrl.turn.z += turnaccel * delta;
    } else if (ctrl.map[ActionRight].value > 0.0f) {
      if (turnaccel < 0.0f) turnaccel = 0.0f;
      turnaccel += 8.0f * delta;
      vehic->ctrl.turn.z += turnaccel * delta;
    } else {
      PULLTOWARD(turnaccel, 0.0f, delta * 5.0f);
      PULLTOWARD(vehic->ctrl.turn.z, 0.0f, delta * 5.0f);
    }
  }
  
  // Computer aided steering
  if (vehic->forwardspeed > 1.0f)
    vehic->ctrl.turn.z -= vehic->body->getAngularVel().z * cfg_drivingassist / (1.0f + vehic->forwardspeed);
  
  
  float throttletarget = 0.0f;
  float braketarget = 0.0f;
  
  if (ctrl.map[ActionForward].value > 0.0f) {
    if (vehic->wheel_angvel > -10.0f)
      throttletarget = ctrl.map[ActionForward].value;
    else
      braketarget = ctrl.map[ActionForward].value;
  }
  if (ctrl.map[ActionBack].value > 0.0f) {
    if (vehic->wheel_angvel < 10.0f)
      throttletarget = -ctrl.map[ActionBack].value;
    else
      braketarget = ctrl.map[ActionBack].value;
  }
  
  PULLTOWARD(vehic->ctrl.throttle, throttletarget, delta * 15.0f);
  PULLTOWARD(vehic->ctrl.brake1, braketarget, delta * 25.0f);

  vehic->ctrl.brake2 = ctrl.map[ActionHandbrake].value;
  
  
  //PULLTOWARD(vehic->ctrl.aim.x, 0.0, delta * 2.0);
  //PULLTOWARD(vehic->ctrl.aim.y, 0.0, delta * 2.0);
  
  game->tick(delta);
  
  #if 1
  for (unsigned int i=0; i<game->vehicle.size(); i++) {
    for (unsigned int j=0; j<game->vehicle[i]->part.size(); j++) {
      for (unsigned int k=0; k<game->vehicle[i]->part[j].wheel.size(); k++) {
        if (rand01 * 20.0f < game->vehicle[i]->part[j].wheel[k].dirtthrow)
          psys_dirt->addParticle(
            game->vehicle[i]->part[j].wheel[k].dirtthrowpos,
            game->vehicle[i]->part[j].wheel[k].dirtthrowvec + vec3f::rand() * 10.0f);
      }
    }
  }
  #endif
  
  float angtarg = 0.0f;
  angtarg -= ctrl.map[ActionCamLeft].value;
  angtarg += ctrl.map[ActionCamRight].value;
  angtarg *= PI*0.75f;
  
  PULLTOWARD(camera_user_angle, angtarg, delta * 4.0f);
  
  quatf tempo;
  //tempo.fromThreeAxisAngle(vec3f(-1.3,0.0,0.0));
  
  // allow temporary camera view changes for this frame
  int cameraview_mod = cameraview;
  
  if (game->gamestate == GS_FINISHED) {
    cameraview_mod = 0;
    static float spinner = 0.0f;
    spinner += 1.4f * delta;
    tempo.fromThreeAxisAngle(vec3f(-PI*0.5f,0.0f,spinner));
  } else {
    tempo.fromThreeAxisAngle(vec3f(-PI*0.5f,0.0f,0.0f));
  }
  
  renderowncar = (cameraview_mod != 1);
  
  campos_prev = campos;
  
  //PReferenceFrame *rf = &vehic->part[2].ref_world;
  PReferenceFrame *rf = &vehic->getBody();
  
  vec3f forw = makevec3f(rf->getOrientationMatrix().row[0]);
  float forwangle = atan2(forw.y, forw.x);
  
  mat44f cammat;
  
  switch (cameraview_mod) {
    default:   
  case 0:  {
    quatf temp2;
    temp2.fromZAngle(forwangle + camera_user_angle);
    
    quatf target = tempo * temp2;
    
    if (target.dot(camori) < 0.0f) target = target * -1.0f;
    
    PULLTOWARD(camori, target, delta * 3.0f);
    
    camori.normalize();
    
    cammat = camori.getMatrix();
    cammat = cammat.transpose();
    //campos = rf->getPosition() + makevec3f(cammat.row[2]) * 100.0;
    campos = rf->getPosition() +
      makevec3f(cammat.row[1]) * 1.6f +
      makevec3f(cammat.row[2]) * 5.0f;
    } break;
    
  case 1: {
    quatf temp2;
    temp2.fromZAngle(camera_user_angle);
    
    quatf target = tempo * temp2 * rf->ori;
    
    if (target.dot(camori) < 0.0f) target = target * -1.0f;
    
    PULLTOWARD(camori, target, delta * 25.0f);
    
    camori.normalize();
    
    cammat = camori.getMatrix();
    cammat = cammat.transpose();
    const mat44f &rfmat = rf->getInverseOrientationMatrix();
    //campos = rf->getPosition() + makevec3f(cammat.row[2]) * 100.0;
    campos = rf->getPosition() +
      makevec3f(rfmat.row[1]) * 1.7f +
      makevec3f(rfmat.row[2]) * 0.4f;
    } break;
    
  case 2: {
    quatf temp2;
    temp2.fromZAngle(camera_user_angle);
    
    quatf target = tempo * temp2 * rf->ori;
    
    if (target.dot(camori) < 0.0f) target = target * -1.0f;
    
    //PULLTOWARD(camori, target, delta * 25.0f);
    camori = target;
    
    camori.normalize();
    
    cammat = camori.getMatrix();
    cammat = cammat.transpose();
    const mat44f &rfmat = rf->getInverseOrientationMatrix();
    //campos = rf->getPosition() + makevec3f(cammat.row[2]) * 100.0;
    campos = rf->getPosition() +
      makevec3f(rfmat.row[0]) * 1.1f +
      makevec3f(rfmat.row[1]) * 0.3f +
      makevec3f(rfmat.row[2]) * 0.1f;
    } break;
  }
  
  forw = makevec3f(cammat.row[0]);
  camera_angle = atan2(forw.y, forw.x);
  
  vec2f diff = makevec2f(game->checkpt[vehic->nextcp].pt) - makevec2f(vehic->body->getPosition());
  nextcpangle = -atan2(diff.y, diff.x) - forwangle + PI*0.5f;
  
  if (cfg_enable_sound) {
    audinst_engine->setGain(0.5f);
    audinst_engine->setPitch(vehic->getEngineRPM() / 7500.0f);
    
    float windlevel = fabsf(vehic->forwardspeed) * 0.6f;
    
    audinst_wind->setGain(windlevel * 0.03f);
    audinst_wind->setPitch(windlevel * 0.02f + 0.9f);
    
    audinst_gravel->setGain(vehic->getSkidLevel() * 0.2f);
    audinst_gravel->setPitch(1.0f);//vehic->getEngineRPM() / 7500.0f);
    
    if (vehic->getFlagGearChange()) {
      audinst.push_back(new PAudioInstance(aud_gearchange));
      audinst.back()->setPitch(1.0f + randm11*0.02f);
      audinst.back()->setGain(0.3f);
      audinst.back()->play();
    }
    
    if (crashnoise_timeout <= 0.0f) {
      float crashlevel = vehic->getCrashNoiseLevel();
      if (crashlevel > 0.0f) {
        audinst.push_back(new PAudioInstance(aud_crash1));
        audinst.back()->setPitch(1.0f + randm11*0.02f);
        audinst.back()->setGain(logf(1.0f + crashlevel));
        audinst.back()->play();
      }
      crashnoise_timeout = rand01 * 0.1f + 0.01f;
    } else {
      crashnoise_timeout -= delta;
    }
  
    for (unsigned int i=0; i<audinst.size(); i++) {
      if (!audinst[i]->isPlaying()) {
        delete audinst[i];
        audinst.erase(audinst.begin() + i);
        i--;
        continue;
      }
    }
  }
  
  psys_dirt->tick(delta);
  
#define RAIN_START_LIFE         0.6f
#define RAIN_POS_RANDOM         15.0f
#define RAIN_VEL_RANDOM         2.0f
  
  const vec3f def_drop_vect(2.5f,0.0f,17.0f);
  
  vec3f camvel = (campos - campos_prev) * (1.0f / delta);
  
  // randomised number of drops calculation
  float numdrops = game->weather.precip.rain * delta;
  int inumdrops = (int)numdrops;
  if (rand01 < numdrops - inumdrops) inumdrops++;
  for (int i=0; i<inumdrops; i++) {
    rain.push_back(RainDrop());
    rain.back().drop_pt = vec3f(campos.x,campos.y,0);
    rain.back().drop_pt += camvel * RAIN_START_LIFE;
    rain.back().drop_pt += vec3f::rand() * RAIN_POS_RANDOM;
    rain.back().drop_pt.z = game->terrain->getHeight(rain.back().drop_pt.x, rain.back().drop_pt.y);
    rain.back().drop_vect = def_drop_vect + vec3f::rand() * RAIN_VEL_RANDOM;
    rain.back().life = RAIN_START_LIFE;
  }
  
  // update life and delete dead raindrops
  unsigned int j=0;
  for (unsigned int i = 0; i < rain.size(); i++) {
    if (rain[i].life <= 0.0f) continue;
    rain[j] = rain[i];
    rain[j].prevlife = rain[j].life;
    rain[j].life -= delta;
    if (rain[j].life < 0.0f)
      rain[j].life = 0.0f; // will be deleted next time round
    j++;
  }
  rain.resize(j);
  
  // update stuff for SSRender
  
  cam_pos = campos;
  cam_orimat = cammat;
  cam_linvel = camvel;
}

// TODO: mark instant events with flags, deal with them in tick()
// this will get rid of the silly doubling up between keyEvent and joyButtonEvent
// and possibly mouseButtonEvent in future

void MainApp::keyEvent(const SDL_KeyboardEvent &ke)
{
  if (ke.type == SDL_KEYDOWN) {
    
    if (ke.keysym.sym == SDLK_F12) {
      saveScreenshot();
      return;
    }
    
    switch (appstate) {
    case AS_LOAD_1:
    case AS_LOAD_2:
      // no hitting escape allowed... end screen not loaded!
      return;
    case AS_LOAD_3:
      levelScreenAction(AA_INIT, 0);
      return;
    case AS_LEVEL_SCREEN:
      handleLevelScreenKey(ke);
      return;
    case AS_CHOOSE_VEHICLE:
      
      if (ctrl.map[ActionLeft].type == UserControl::TypeKey &&
        ctrl.map[ActionLeft].key.sym == ke.keysym.sym) {
        if (--choose_type < 0)
          choose_type = (int)game->vehiclechoices.size()-1;
        return;
      }
      if (ctrl.map[ActionRight].type == UserControl::TypeKey &&
        ctrl.map[ActionRight].key.sym == ke.keysym.sym) {
        if (++choose_type >= (int)game->vehiclechoices.size())
          choose_type = 0;
        return;
      }
      
      switch (ke.keysym.sym) {
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        startGame2();
        game->chooseVehicle(game->vehiclechoices[choose_type]);
        appstate = AS_IN_GAME;
        return;
      case SDLK_ESCAPE:
        endGame(GF_NOT_FINISHED);
        return;
      default:
        break;
      }
      break;
    case AS_IN_GAME:
      
      if (ctrl.map[ActionRecover].type == UserControl::TypeKey &&
        ctrl.map[ActionRecover].key.sym == ke.keysym.sym) {
        game->vehicle[0]->doReset();
        return;
      }
      if (ctrl.map[ActionCamMode].type == UserControl::TypeKey &&
        ctrl.map[ActionCamMode].key.sym == ke.keysym.sym) {
        cameraview = (cameraview + 1) % 3;
        camera_user_angle = 0.0f;
        return;
      }
      if (ctrl.map[ActionShowMap].type == UserControl::TypeKey &&
        ctrl.map[ActionShowMap].key.sym == ke.keysym.sym) {
        showmap = !showmap;
        return;
      }
      
      switch (ke.keysym.sym) {
      case SDLK_ESCAPE:
        endGame(GF_FAIL);
        return;
      default:
        break;
      }
      break;
    case AS_END_SCREEN:
      requestExit();
      return;
    }
    
    switch (ke.keysym.sym) {
    case SDLK_ESCAPE:
      quitGame();
      return;
    default:
      break;
    }
  }
}

void MainApp::mouseMoveEvent(int dx, int dy)
{
  //PVehicle *vehic = game->vehicle[0];
  
  //vehic->ctrl.tank.turret_turn.x += dx * -0.002;
  //vehic->ctrl.tank.turret_turn.y += dy * 0.002;
  
  //vehic->ctrl.turn.x += dy * 0.005;
  //vehic->ctrl.turn.y += dx * -0.005;
  
  dy = dy;
  
  if (appstate == AS_IN_GAME) {
    PVehicle *vehic = game->vehicle[0];
    vehic->ctrl.turn.z += dx * 0.01f;
  }
}

void MainApp::joyButtonEvent(int which, int button, bool down)
{
  if (which == 0 && down) {
    
    switch (appstate) {
    case AS_CHOOSE_VEHICLE:
      
      if (ctrl.map[ActionLeft].type == UserControl::TypeJoyButton &&
        ctrl.map[ActionLeft].joybutton.button == button) {
        if (--choose_type < 0)
          choose_type = (int)game->vehiclechoices.size()-1;
        return;
      }
      if (ctrl.map[ActionRight].type == UserControl::TypeJoyButton &&
        ctrl.map[ActionRight].joybutton.button == button) {
        if (++choose_type >= (int)game->vehiclechoices.size())
          choose_type = 0;
        return;
      }
      
      break;
      
    case AS_IN_GAME:
      
      if (ctrl.map[ActionRecover].type == UserControl::TypeJoyButton &&
        ctrl.map[ActionRecover].joybutton.button == button) {
        game->vehicle[0]->doReset();
        return;
      }
      if (ctrl.map[ActionCamMode].type == UserControl::TypeJoyButton &&
        ctrl.map[ActionCamMode].joybutton.button == button) {
        cameraview = (cameraview + 1) % 3;
        camera_user_angle = 0.0f;
        return;
      }
      if (ctrl.map[ActionShowMap].type == UserControl::TypeJoyButton &&
        ctrl.map[ActionShowMap].joybutton.button == button) {
        showmap = !showmap;
        return;
      }
    }
  }
}


int main(int argc, char *argv[])
{
  MainApp *game = new MainApp("Trigger", ".trigger");

  int ret = game->run(argc, argv);

  delete game;

  return ret;
}



