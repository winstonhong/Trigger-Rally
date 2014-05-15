
// app.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



struct joystick_s {
  SDL_Joystick *sdl_joystick;
  std::string name;
  std::vector<float> axis;
  std::vector<bool> button;
  std::vector<vec2i> hat;
};

class PApp {
public:
  
  enum StereoMode {
    StereoNone,
    StereoQuadBuffer,
    StereoRedBlue,
    StereoRedGreen,
    StereoRedCyan,
    StereoYellowBlue
  };
  
private:
  std::string appname, apptitle;
  
  SDL_Surface *screen;
  int cx, cy, bpp;
  bool fullscr, noframe;
  bool reqRGB, reqAlpha, reqDepth, reqStencil;
  bool grabinput;
  
  StereoMode stereo;
  float stereoEyeTranslation;
  
  uint8* sdl_keymap;
  int sdl_numkeys;
  uint8 sdl_mousemap;
  std::vector<joystick_s> sdl_joy;
  
  bool exit_requested, screenshot_requested;

  PSSRender *ssrdr;
  PSSTexture *sstex;
  PSSEffect *ssfx;
  PSSModel *ssmod;
  PSSAudio *ssaud;
  
protected:
  // the derived app should keep these up to date
  vec3f cam_pos;
  mat44f cam_orimat;
  vec3f cam_linvel;
  
public:
  PApp(const std::string &title = "PGame", const std::string &name = ".pgame") {
    appname = name; // for ~/.name
    apptitle = title; // for window title
    cx = cy = 0; bpp = 0; fullscr = false; noframe = false;
    exit_requested = false;
    screenshot_requested = false;
    reqRGB = reqAlpha = reqDepth = reqStencil = false;
    stereo = StereoNone;
    stereoEyeTranslation = 0.0f;
    grabinput = false;
  }
  virtual ~PApp() { }
  
  int run(int argc, char *argv[]);
  
public:
  std::ostream &outLog() { return std::cout; } // superceded by PUtil::outLog, remove?
  
  int getWidth() { return cx; }
  int getHeight() { return cy; }
  int getNumJoysticks() { return (sdl_joy.size()); }
  int getJoyNumAxes(int j) { return (sdl_joy[j].axis.size()); }
  int getJoyNumButtons(int j) { return (sdl_joy[j].button.size()); }
  int getJoyNumHats(int j) { return (sdl_joy[j].hat.size()); }
  float getJoyAxis(int j, int a) { return (sdl_joy[j].axis[a]); }
  bool getJoyButton(int j, int b) { return (sdl_joy[j].button[b]); }
  const vec2i &getJoyHat(int j, int h) { return (sdl_joy[j].hat[h]); }

  PSSRender &getSSRender() { return *ssrdr; }
  PSSTexture &getSSTexture() { return *sstex; }
  PSSEffect &getSSEffect() { return *ssfx; }
  PSSModel &getSSModel() { return *ssmod; }
  PSSAudio &getSSAudio() { return *ssaud; }

protected:
  bool keyDown(int key) { return (sdl_keymap[key] != 0); }
  bool mouseButtonDown(int bt) { return ((sdl_mousemap & SDL_BUTTON(bt)) != 0); }

  void requestExit() {
    exit_requested = true;
  }
  
  void saveScreenshot() {
    screenshot_requested = true;
  }
  
  void grabMouse(bool grab = true);
  
  void drawModel(PModel &model);
  
  void stereoGLProject(float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float zzps, float dist, float eye);
  void stereoFrustum(float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float zzps, float eye);
  
  // config stuff
  
  void setScreenMode(int w, int h, bool fullScreen = false, bool hideFrame = false)
  { cx = w; cy = h; fullscr = fullScreen; noframe = hideFrame; }
  void setScreenBPP(int _bpp)
  { bpp = _bpp; }
  void setScreenModeAutoWindow();
  void setScreenModeFastFullScreen();
  void requireRGB(bool req = true) { reqRGB = req; }
  void requireAlpha(bool req = true) { reqAlpha = req; }
  void requireDepth(bool req = true) { reqDepth = req; }
  void requireStencil(bool req = true) { reqStencil = req; }
  
  void setStereoMode(StereoMode mode) { stereo = mode; }
  void setStereoEyeSeperation(float distance) { stereoEyeTranslation = distance * 0.5f; }
  
  // callbacks for derived classes
  
  virtual void config() /* throw (PUserException) */ ; // very light setup/config func
  virtual void load() /* throw (PUserException) */ ; // main resource loading
  virtual void unload(); // free resources
  
  virtual void tick(float delta);
  virtual void resize();
  virtual void render(float eyetranslation);
  virtual void keyEvent(const SDL_KeyboardEvent &ke);
  virtual void mouseButtonEvent(const SDL_MouseButtonEvent &mbe);
  virtual void mouseMoveEvent(int dx, int dy);
  virtual void cursorMoveEvent(int posx, int posy);
  virtual void joyButtonEvent(int which, int button, bool down);
};



