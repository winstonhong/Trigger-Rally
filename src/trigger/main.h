
// main.h

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "../pengine/pengine.h"

#include "../psim/psim.h"



// Forward declaration for TriggerGame to use
class MainApp;




struct CheckPoint {
  vec3f pt;
  
  CheckPoint(const vec3f &_pt) : pt(_pt) { }
};

/*
struct AIDriver {
  int vehic;
  
  AIDriver(int v) : vehic(v) { }
};
*/


#define GS_COUNTDOWN        0
#define GS_RACING           1
#define GS_FINISHED         2

#define GF_NOT_FINISHED     0
#define GF_PASS             1
#define GF_FAIL             2


class TriggerGame {
  friend class MainApp;
  
private:
  MainApp *app;
  
  PSim *sim;
  
  int randomseed;
  
  std::vector<PVehicle *> vehicle;
  
  //std::vector<AIDriver> aid;
  
  PTerrain *terrain;
  
  std::vector<CheckPoint> checkpt;
  
  int gamestate;
  
  float coursetime;
  float othertime; // used pre and post race
  float cptime; // checkpoint time
  float targettime; // the time needed to win
  
  std::string comment; // level comment string
  
  vec3f start_pos;
  quatf start_ori;
  
  struct {
    struct {
      std::string texname;
      float scrollrate;
    } cloud;
    struct {
      vec3f color;
      float density;
      float density_sky;
    } fog;
    struct {
      float rain;
    } precip;
  } weather;
  
public:
  std::vector<PVehicleType *> vehiclechoices;
  
public:
  TriggerGame(MainApp *parent);
  ~TriggerGame();
  
  bool loadLevel(const std::string &filename);
  
  void chooseVehicle(PVehicleType *type);
  
  void tick(float delta);
  
  bool isFinished() { return (gamestate == GS_FINISHED) && (othertime <= 0.0f); }
  int getFinishState() {
    if (gamestate != GS_FINISHED) return GF_NOT_FINISHED;
    if (coursetime <= targettime) return GF_PASS;
    else return GF_FAIL;
  }
};


#include "menu.h"


#define AS_LOAD_1           1
#define AS_LOAD_2           2
#define AS_LOAD_3           3
#define AS_LEVEL_SCREEN     10
#define AS_CHOOSE_VEHICLE   11
#define AS_IN_GAME          12
#define AS_END_SCREEN       13



struct TriggerLevel {
  std::string filename, name, comment, author, targettime;
};

struct TriggerEvent {
  std::string filename, name, comment, author;
  
  // Note that levels are not linked to... they are
  // stored in the event because an event may have
  // "hidden" levels not otherwise available
  
  std::vector<TriggerLevel> levels;
};


class DirtParticleSystem : public PParticleSystem {
public:
  void tick(float delta) {
    
    PParticleSystem::tick(delta);
    
    for (unsigned int i=0; i<part.size(); i++) {
      PULLTOWARD(part[i].linvel, vec3f::zero(), delta * 25.0f);
    }
  }
};


struct RainDrop {
  vec3f drop_pt, drop_vect;
  float life, prevlife;
};


struct UserControl {
  enum {
    TypeUnassigned,
    TypeKey,
    TypeJoyButton,
    TypeJoyAxis
  } type;
  union {
    struct {
      SDLKey sym;
    } key;
    struct {
      int button;
    } joybutton;
    struct {
      int axis;
      float sign;
      float deadzone;
      float maxrange;
    } joyaxis; // more like half-axis, really
  };
  
  float value; // from 0.0 to 1.0 depending on activation level
};

class MainApp : public PApp {
public:
  enum Speedunit {
    mph,
    kph
  };

  enum Speedstyle {
    analogue,
    hybrid
  };

private:
  
  int appstate;
  
  // Config settings
  
  int cfg_video_cx, cfg_video_cy;
  bool cfg_video_fullscreen;
  
  float cfg_drivingassist;
  bool cfg_enable_sound;
  
  Speedunit cfg_speed_unit;
  Speedstyle cfg_speed_style;
  float hud_speedo_start_deg;
  float hud_speedo_mps_deg_mult;
  float hud_speedo_mps_speed_mult;

  enum Action {
    ActionForward,
    ActionBack,
    ActionLeft,
    ActionRight,
    ActionHandbrake,
    ActionRecover,
    ActionCamMode,
    ActionCamLeft,
    ActionCamRight,
    ActionShowMap,
    ActionCount
  };
  struct {
    std::string action_name[ActionCount];
    UserControl map[ActionCount];
  } ctrl;
  
  //
  
  float splashtimeout;
  
  //
  
  std::vector<TriggerLevel> levels;
  std::vector<TriggerEvent> events;
  
  // for level screen
  Gui gui;
  LevelState lss;
  
  //
  
  TriggerGame *game;
  
  PVehicleType *vt_tank;
  
  PTexture *tex_font;
  
  PTexture *tex_detail,
           *tex_sky[1],
           *tex_water,
           *tex_dirt,
           *tex_shadow,
           *tex_hud_revs,
           *tex_hud_speedo,
           *tex_hud_gear,
           *tex_hud_life,
           *tex_splash_screen,
           *tex_end_screen;
  
  DirtParticleSystem *psys_dirt;
  
  PAudioSample *aud_engine,
               *aud_wind,
               *aud_gearchange,
               *aud_gravel,
               *aud_crash1;
  
  PAudioInstance *audinst_engine, *audinst_wind, *audinst_gravel;
  std::vector<PAudioInstance *> audinst;
  
  float cloudscroll;
  
  vec3f campos, campos_prev;
  quatf camori;
  
  vec3f camvel;
  
  float nextcpangle;
  
  float cprotate;
  
  int cameraview;
  float camera_angle;
  float camera_user_angle;
  
  bool renderowncar; // this is determined from cameraview
  
  bool showmap;
  
  float crashnoise_timeout;
  
  std::vector<RainDrop> rain;
  
  //
  
  int loadscreencount;
  
  float choose_spin;
  
  int choose_type;
  
protected:
  void renderWater();
  void renderSky(const mat44f &cammat);
  
  bool startGame(const std::string &filename);
  void startGame2();
  void endGame(int gamestate);
  
  void quitGame() {
    endGame(GF_NOT_FINISHED);
    splashtimeout = 0.0f;
    appstate = AS_END_SCREEN;
  }
  
  void levelScreenAction(int action, int index);
  void handleLevelScreenKey(const SDL_KeyboardEvent &ke);
  void finishRace(int gamestate, float coursetime);
  
public:
  MainApp(const std::string &title, const std::string &name) : PApp(title, name) { }
  //MainApp::~MainApp(); // should not have destructor, use unload
  
  void config();
  void load();
  void unload();
  
  void loadConfig();
  bool loadAll();
  bool loadLevelsAndEvents();
  bool loadLevel(TriggerLevel &tl);
  
  void tick(float delta);
  
  void resize();
  void render(float eyetranslation);
  
  void renderStateLoading(float eyetranslation);
  void renderStateEnd(float eyetranslation);
  void tickStateLevel(float delta);
  void renderStateLevel(float eyetranslation);
  void tickStateChoose(float delta);
  void renderStateChoose(float eyetranslation);
  void tickStateGame(float delta);
  void renderStateGame(float eyetranslation);
  
  void keyEvent(const SDL_KeyboardEvent &ke);
  void mouseMoveEvent(int dx, int dy);
  void cursorMoveEvent(int posx, int posy);
  void mouseButtonEvent(const SDL_MouseButtonEvent &mbe);
  void joyButtonEvent(int which, int button, bool down);
};




