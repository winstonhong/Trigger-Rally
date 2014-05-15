
// app.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



#ifndef DATADIR
#error DATADIR not defined! Use ./configure --datadir=...
#define DATADIR ""
#endif

#if 0
#ifndef LOCALSTATEDIR
#warning LOCALSTATEDIR not defined! Will attempt to determine at run time.
#warning Use ./configure --localstatedir=...
#endif
#endif


/*
FIXME: not bothering to close joysticks because I
suspect SDL does it for you on quit. Am I right?
*/


int PUtil::deblev = DEBUGLEVEL_ENDUSER;



void PApp::setScreenModeAutoWindow()
{
#ifdef WIN32
  cy = GetSystemMetrics(SM_CYSCREEN) * 6 / 7;
  cx = cy * 4 / 3;
#else
  //#error COMPLETE FOR PLATFORM
  cy = 1024 * 6 / 7;
  cx = cy * 4 / 3;
#endif

  fullscr = false;
}

void PApp::setScreenModeFastFullScreen()
{
#ifdef WIN32
  cx = GetSystemMetrics(SM_CXSCREEN);
  cy = GetSystemMetrics(SM_CYSCREEN);
#else
  //#error COMPLETE FOR PLATFORM
  cx = 1280;
  cy = 1024;
#endif

  fullscr = false;

  noframe = true;
}

/* This routine performs the perspective projection for one eye's subfield.
   The projection is in the direction of the negative z axis.

   xmin, ymax, ymin, ymax = the coordinate range, in the plane of zero
   parallax setting, that will be displayed on the screen. The ratio between
   (xmax-xmin) and (ymax-ymin) should equal the aspect ration of the display.

   znear, zfar = the z-coordinate values of the clipping planes.

   zzps = the z-coordinate of the plane of zero parallax setting.

   dist = the distance from the center of projection to the plane of zero
   parallax.

   eye = half the eye separation; positive for the right eye subfield,
   negative for the left eye subfield.
*/
void PApp::stereoGLProject(float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float zzps, float dist, float eye)
{
  float xmid, ymid, clip_near, clip_far, top, bottom, left, right, dx, dy, n_over_d;

  dx = xmax - xmin;
  dy = ymax - ymin;

  xmid = (xmax + xmin) / 2.0;
  ymid = (ymax + ymin) / 2.0;

  clip_near = dist + zzps - znear;
  clip_far = dist + zzps - zfar;

  n_over_d = clip_near / dist;

  top = n_over_d * dy / 2.0;
  bottom = -top;
  right = n_over_d * (dx / 2.0 - eye);
  left = n_over_d * (-dx / 2.0 - eye);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(left, right, bottom, top, clip_near, clip_far);
  glTranslatef(-xmid - eye, -ymid, -zzps - dist);
}

// I'm afraid I didn't understand how stereoGLProject worked, so I rewrote it

void PApp::stereoFrustum(float xmin, float xmax, float ymin, float ymax, float znear, float zfar, float zzps, float eye)
{
  // xmove = eye * (zzps - znear) / zzps - eye, simplifies to
  
  float xmove = -eye * znear / zzps;
  
  glFrustum(xmin + xmove, xmax + xmove, ymin, ymax, znear, zfar);
}

int PApp::run(int argc, char *argv[])
{
  outLog() << apptitle << " init" << std::endl;
  
  outLog() << "Build: " << PACKAGE_VERSION << " on " << __DATE__ << " at " << __TIME__ << std::endl;
  
  if (exit_requested) {
    outLog() << "Exit requested" << std::endl;
    return 0;
  }
  
  outLog() << "Initialising PhysFS" << std::endl;
  
  if (PHYSFS_init((argc >= 1) ? argv[0] : null) == 0) {
    outLog() << "PhysFS failed to initialise" << std::endl;
    outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    return 1;
  }
  
  PHYSFS_permitSymbolicLinks(1);
  
  {
    std::string lsdbuff;
    
    lsdbuff = (std::string)PHYSFS_getUserDir();
    
    // this is a hack because PhysFS doesn't appear to live
    // up to its claim of creating the WriteDir if necessary.
    // set write dir to home, create ~/.whatever, then change
    // over to that dir.
    
#if 1
    outLog() << "Set writable user directory to \"" << lsdbuff << "\"" << std::endl;
    
    if (PHYSFS_setWriteDir(lsdbuff.c_str()) == 0) {
      outLog() << "Failed to set PhysFS writable directory to \"" << lsdbuff << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    
    if (PHYSFS_mkdir(appname.c_str()) == 0) {
      outLog() << "Failed to create directory \"" << appname << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
#endif
    
    lsdbuff += appname;
    
    outLog() << "Reset writable user directory to \"" << lsdbuff << "\"" << std::endl;
    
    if (PHYSFS_setWriteDir(lsdbuff.c_str()) == 0) {
      outLog() << "Failed to set PhysFS writable directory to \"" << lsdbuff << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    
    // Adding "." to the search path seems to add more trouble than it's worth
#if 0
    if (PHYSFS_addToSearchPath(".", 1) == 0) {
      outLog() << "Failed to add PhysFS search directory \".\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
#endif
    
    std::string basedir = PHYSFS_getBaseDir();
    PUtil::outLog() << "Application base directory \"" << basedir << '\"' << std::endl;
    if (PHYSFS_addToSearchPath(basedir.c_str(), 1) == 0) {
      outLog() << "Failed to add PhysFS search directory \"" << basedir << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    
    if (PHYSFS_addToSearchPath(lsdbuff.c_str(), 1) == 0) {
      outLog() << "Failed to add PhysFS search directory \"" << lsdbuff << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    
    std::string ddbuff = DATADIR;
    
    outLog() << "Main game data directory datadir=\"" << ddbuff << "\"" << std::endl;
    
    if (PHYSFS_addToSearchPath(ddbuff.c_str(), 1) == 0) {
      outLog() << "Failed to add PhysFS search directory \"" << ddbuff << "\"" << std::endl
          << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    
    // Find any .zip files and add them to search path
    std::list<std::string> zipfiles = PUtil::findFiles("", ".zip");
    
    for (std::list<std::string>::iterator i = zipfiles.begin();
      i != zipfiles.end(); i++) {
      
      const char *realpath = PHYSFS_getRealDir(i->c_str());
      
      if (realpath) {
        std::string fullpath = (std::string)realpath + *i;
        
        if (PHYSFS_addToSearchPath(fullpath.c_str(), 1) == 0) {
          outLog() << "Failed to add archive \"" << fullpath << "\"" << std::endl
              << "PhysFS: " << PHYSFS_getLastError() << std::endl;
        }
      } else {
        outLog() << "Failed to find path of archive \"" << *i << "\"" << std::endl
            << "PhysFS: " << PHYSFS_getLastError() << std::endl;
      }
    }
  }
  
  try
  {
    config ();
  }
  catch (PException e)
  {
    PUtil::outLog() << "Config failed: " << e.what () << std::endl;
    
    if (PHYSFS_deinit() == 0) {
      outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    return 1;
  }
  
  outLog() << "Initialising SDL" << std::endl;
  
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);
  //SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);
  
  srand(SDL_GetTicks());
  
  SDL_WM_SetCaption(apptitle.c_str(), null);
  
  outLog() << "Create window and set video mode" << std::endl;
  
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  if (reqRGB) {
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  }
  
  if (reqAlpha) {
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
  }
  
  if (reqDepth) {
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
  }
  
  if (reqStencil)
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
  
  if (stereo == StereoQuadBuffer) {
    SDL_GL_SetAttribute( SDL_GL_STEREO, 1 );
  }
  
  if (cx <= 0 || cy <= 0) setScreenModeAutoWindow();
  
  screen = SDL_SetVideoMode(cx,cy,bpp,
    SDL_OPENGL |
    (fullscr ? SDL_FULLSCREEN : 0) |
    (noframe ? SDL_NOFRAME : 0));
  
  if (!screen) {
    outLog() << "Failed to create window or set video mode" << std::endl;
    outLog() << "SDL error: " << SDL_GetError() << std::endl;
    outLog() << "Try changing your video settings in data/trigger.config" << std::endl;
    SDL_Quit();
    if (PHYSFS_deinit() == 0) {
      outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    return 1;
  }
  
  sdl_mousemap = 0;
  
  sdl_joy.resize(SDL_NumJoysticks());
  
  outLog() << "Found " << sdl_joy.size() << " joystick" <<
    (sdl_joy.size() == 1 ? "" : "s") << std::endl;
  
  for (unsigned int i=0; i<sdl_joy.size(); i++) {
    outLog() << "Joystick " << (i+1) << ": ";
    sdl_joy[i].sdl_joystick = SDL_JoystickOpen(i);
    if (sdl_joy[i].sdl_joystick == null) {
      outLog() << "failed to open joystick" << std::endl;
      SDL_Quit();
      if (PHYSFS_deinit() == 0) {
        outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
      }
      return 1;
    }
    sdl_joy[i].name = SDL_JoystickName(i);
    sdl_joy[i].axis.resize(SDL_JoystickNumAxes(sdl_joy[i].sdl_joystick));
    for (unsigned int j=0; j<sdl_joy[i].axis.size(); j++)
      sdl_joy[i].axis[j] = ((float)SDL_JoystickGetAxis(sdl_joy[i].sdl_joystick, j) + 0.5f) / 32767.5f;
    sdl_joy[i].button.resize(SDL_JoystickNumButtons(sdl_joy[i].sdl_joystick));
    for (unsigned int j=0; j<sdl_joy[i].button.size(); j++)
      sdl_joy[i].button[j] = (SDL_JoystickGetButton(sdl_joy[i].sdl_joystick, j) != 0);
    sdl_joy[i].hat.resize(SDL_JoystickNumHats(sdl_joy[i].sdl_joystick));
    for (unsigned int j=0; j<sdl_joy[i].hat.size(); j++) {
      Uint8 state = SDL_JoystickGetHat(sdl_joy[i].sdl_joystick, j);
      sdl_joy[i].hat[j] = vec2i::zero();
      if (state & SDL_HAT_RIGHT) sdl_joy[i].hat[j].x = 1;
      else if (state & SDL_HAT_LEFT) sdl_joy[i].hat[j].x = -1;
      if (state & SDL_HAT_UP) sdl_joy[i].hat[j].y = 1;
      else if (state & SDL_HAT_DOWN) sdl_joy[i].hat[j].y = -1;
    }
    
    outLog() << sdl_joy[i].name << ", " <<
      sdl_joy[i].axis.size() << " axis, " <<
      sdl_joy[i].button.size() << " button, " <<
      sdl_joy[i].hat.size() << " hat" << std::endl;
  }
  
  SDL_JoystickEventState(SDL_ENABLE);
  
  int err = glewInit();
  
  if (err != GLEW_OK) {
    outLog() << "GLEW failed to initialise: " << glewGetErrorString(err) << std::endl;
    SDL_Quit();
    if (PHYSFS_deinit() == 0) {
      outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    return 1;
  }
  
  outLog() << "GLEW initialized" << std::endl;
  
  outLog() << "Graphics: " <<
    glGetString(GL_VENDOR) << " " << glGetString(GL_RENDERER) << std::endl;
  
  outLog() << "Using OpenGL ";
  if (false) ;
  else if (GLEW_VERSION_2_0)
    outLog() << "2.0" << std::endl;
  else if (GLEW_VERSION_1_5)
    outLog() << "1.5" << std::endl;
  else if (GLEW_VERSION_1_4)
    outLog() << "1.4" << std::endl;
  else if (GLEW_VERSION_1_3)
    outLog() << "1.3" << std::endl;
  else if (GLEW_VERSION_1_2)
    outLog() << "1.2" << std::endl;
  else if (GLEW_VERSION_1_1)
    outLog() << "1.1" << std::endl;
  else
    outLog() << "1.0" << std::endl;

  switch (stereo) {
  default: break;
  case StereoQuadBuffer:
    PUtil::outLog() << "Using hardware quad buffer stereo, separation ="
      << stereoEyeTranslation * 2.0f << std::endl;
    break;
  case StereoRedBlue:
    PUtil::outLog() << "Using red-blue anaglyph stereo, separation = "
      << stereoEyeTranslation * 2.0f << std::endl;
    break;
  case StereoRedGreen:
    PUtil::outLog() << "Using red-green anaglyph stereo, separation = "
      << stereoEyeTranslation * 2.0f << std::endl;
    break;
  case StereoRedCyan:
    PUtil::outLog() << "Using red-cyan anaglyph stereo, separation = "
      << stereoEyeTranslation * 2.0f << std::endl;
    break;
  case StereoYellowBlue:
    PUtil::outLog() << "Using yellow-blue anaglyph stereo, separation = "
      << stereoEyeTranslation * 2.0f << std::endl;
    break;
  }

  std::list<PSubsystem *> sslist;

  try
  {
    sslist.push_back(ssrdr = new PSSRender(*this));
    sslist.push_back(sstex = new PSSTexture(*this));
    sslist.push_back(ssfx = new PSSEffect(*this));
    sslist.push_back(ssmod = new PSSModel(*this));
    sslist.push_back(ssaud = new PSSAudio(*this));
  }
  catch (PException e)
  {
    PUtil::outLog () << "Subsystem failed to init: " << e.what () << std::endl;
    
    while (!sslist.empty())
    {
      delete sslist.back();
      sslist.pop_back();
    }
    
    SDL_Quit();
    if (PHYSFS_deinit() == 0)
    {
      PUtil::outLog () << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    return 1;
  }
  
  outLog() << "Performing app load" << std::endl;
  
  try
  {
    load ();
  }
  catch (PException e)
  {
    outLog() << "App load failed: " << e.what () << std::endl;

    while (!sslist.empty()) {
      delete sslist.back();
      sslist.pop_back();
    }

    SDL_Quit();
    if (PHYSFS_deinit() == 0) {
      outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
    }
    return 1;
  }

  //SDL_ShowCursor(SDL_DISABLE);
  //SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_WM_GrabInput(SDL_GRAB_OFF);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  //SDL_EnableUNICODE(1);

  sdl_keymap = SDL_GetKeyState(&sdl_numkeys);

  resize();

  outLog() << "Initialisation complete, entering main loop" << std::endl;

  srand(rand() + SDL_GetTicks());

  bool active = true, repaint = true;
  uint32 curtime = SDL_GetTicks() - 1;

  while (1) {
    SDL_Event event;

    while ( SDL_PollEvent(&event) ) {
      switch(event.type) {
      
      // Using ACTIVEEVENT only seems to cause trouble.
      
      /*
      case SDL_ACTIVEEVENT:
        active = event.active.gain;
        if (active) {
          SDL_ShowCursor(SDL_DISABLE);
          //SDL_WM_GrabInput(SDL_GRAB_ON);
          outLog() << "Window made active" << std::endl;
        } else {
          SDL_ShowCursor(SDL_ENABLE);
          //SDL_WM_GrabInput(SDL_GRAB_OFF);
          outLog() << "Window made inactive" << std::endl;
        }
        break;
      */

      case SDL_VIDEOEXPOSE:
        repaint = true;
        break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        keyEvent(event.key);
        break;

      case SDL_MOUSEMOTION:
        if (grabinput)
          mouseMoveEvent(event.motion.xrel, -event.motion.yrel);
        else
          cursorMoveEvent(event.motion.x, event.motion.y);
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        mouseButtonEvent(event.button);
        break;

      case SDL_JOYAXISMOTION:
        sdl_joy[event.jaxis.which].axis[event.jaxis.axis] =
          ((float)event.jaxis.value + 0.5f) / 32767.5f;
        break;

      case SDL_JOYBUTTONDOWN:
        sdl_joy[event.jbutton.which].button[event.jbutton.button] = true;
        joyButtonEvent(event.jbutton.which,event.jbutton.button,true);
        break;

      case SDL_JOYBUTTONUP:
        sdl_joy[event.jbutton.which].button[event.jbutton.button] = false;
        joyButtonEvent(event.jbutton.which,event.jbutton.button,false);
        break;

      case SDL_JOYHATMOTION:
        sdl_joy[event.jhat.which].hat[event.jhat.hat] = vec2i::zero();
        if (event.jhat.value & SDL_HAT_RIGHT) sdl_joy[event.jhat.which].hat[event.jhat.hat].x = 1;
        else if (event.jhat.value & SDL_HAT_LEFT) sdl_joy[event.jhat.which].hat[event.jhat.hat].x = -1;
        if (event.jhat.value & SDL_HAT_UP) sdl_joy[event.jhat.which].hat[event.jhat.hat].y = 1;
        else if (event.jhat.value & SDL_HAT_DOWN) sdl_joy[event.jhat.which].hat[event.jhat.hat].y = -1;
        break;

      case SDL_QUIT:
        requestExit();
        break;
      }
      if (exit_requested) break;
    }

    if (exit_requested) {
      break;
    }

    sdl_mousemap = SDL_GetMouseState(null, null);

#define TIMESCALE 1.0

    uint32 nowtime = SDL_GetTicks();

    if (1) {//if (active) {
      uint32 timepassed = nowtime - curtime;
      if (timepassed > 100) timepassed = 100;
      if (timepassed > 0) {
        float delta = (float)timepassed * 0.001 * TIMESCALE;

        tick(delta);

        for (std::list<PSubsystem *>::iterator i = sslist.begin();
          i != sslist.end(); ++i) {
          (*i)->tick(delta, cam_pos, cam_orimat, cam_linvel);
        }
      }
    }

    curtime = nowtime;

    if (exit_requested) break;

    if (active || repaint) {
      switch (stereo) {
        
      case StereoNone: // Normal, non-stereo rendering
        
        render(0.0f);
        glFlush();
        SDL_GL_SwapBuffers();
        break;
        
      case StereoQuadBuffer: // Hardware quad buffer stereo
        
        glDrawBuffer(GL_BACK_LEFT);
        render(-stereoEyeTranslation);
        glFlush();
        
        glDrawBuffer(GL_BACK_RIGHT);
        render(stereoEyeTranslation);
        glFlush();
        
        SDL_GL_SwapBuffers();
        break;
        
      case StereoRedBlue: // Red-blue anaglyph stereo
        
        // Green will not be rendered to, so clear it
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
        render(-stereoEyeTranslation);
        glFlush();
        
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
        render(stereoEyeTranslation);
        glFlush();
        
        SDL_GL_SwapBuffers();
        break;
        
      case StereoRedGreen: // Red-green anaglyph stereo
        
        // Blue will not be rendered to, so clear it
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
        render(-stereoEyeTranslation);
        glFlush();
        
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
        render(stereoEyeTranslation);
        glFlush();
        
        SDL_GL_SwapBuffers();
        break;
        
      case StereoRedCyan: // Red-cyan anaglyph stereo
        
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
        render(-stereoEyeTranslation);
        glFlush();
        
        glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
        render(stereoEyeTranslation);
        glFlush();
        
        SDL_GL_SwapBuffers();
        break;
        
      case StereoYellowBlue: // Yellow-blue anaglyph stereo
        
        glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
        render(-stereoEyeTranslation);
        glFlush();
        
        glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
        render(stereoEyeTranslation);
        glFlush();
        
        SDL_GL_SwapBuffers();
        break;
      }
      repaint = false;
      
      if (screenshot_requested) {
        glReadBuffer(GL_FRONT);
        unsigned char *data1 = new unsigned char[cx*(cy+1)*3];
        glReadPixels(0, 0, cx, cy, GL_RGB, GL_UNSIGNED_BYTE, data1);
        glReadBuffer(GL_BACK);
        const int rowsize = cx * 3;
        for(int i = 0; i < cy/2; ++i) {
          memcpy(&data1[(cy) * rowsize], &data1[(cy-1-i) * rowsize], rowsize);
          memcpy(&data1[(cy-1-i) * rowsize], &data1[(i) * rowsize], rowsize);
          memcpy(&data1[(i) * rowsize], &data1[(cy) * rowsize], rowsize);
          //memset(&data1[(i) * rowsize], 128, rowsize);
        }
        char buff[200];
        sprintf(buff, "P6\n"
          "# CREATOR: Trigger PNM Screenshot\n"
          "%i %i\n255\n", cx, cy);
        char filename[100];
        sprintf(filename, "screen-%09u.ppm", SDL_GetTicks());
        PUtil::outLog() << "Writing screenshot \"" << filename << "\"" << std::endl;
        PHYSFS_file* pfile = PHYSFS_openWrite(filename);
        if (pfile) {
          PHYSFS_write(pfile, buff, strlen(buff), 1);
          PHYSFS_write(pfile, data1, cx*cy*3, 1);
          PHYSFS_close(pfile);
        } else {
          PUtil::outLog() << "Screenshot write failed" << std::endl;
          PUtil::outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
        }
        delete[] data1;
        screenshot_requested = false;
      }
      
    } else {
      SDL_WaitEvent(null);
    }

    if (exit_requested) break;
  }

  outLog() << "Exit requested" << std::endl;

  unload();
  
  while (!sslist.empty()) {
    delete sslist.back();
    sslist.pop_back();
  }
  
  SDL_WM_GrabInput(SDL_GRAB_OFF);
  SDL_ShowCursor(SDL_ENABLE);
  
  SDL_Quit();
  
  if (PHYSFS_deinit() == 0) {
    outLog() << "PhysFS: " << PHYSFS_getLastError() << std::endl;
  }
  
  outLog() << "Shutdown complete" << std::endl;
  
  return 0;
}

void PApp::grabMouse(bool grab)
{
  //SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
  SDL_ShowCursor(grab ? SDL_DISABLE : SDL_ENABLE);

  grabinput = grab;
}

void PApp::drawModel(PModel &model)
{
  for (std::vector<PMesh>::iterator mesh = model.mesh.begin();
    mesh != model.mesh.end();
    mesh++) {
    if (!mesh->effect)
      mesh->effect = getSSEffect().loadEffect(mesh->fxname);

    int numPasses = 0;
    if (mesh->effect->renderBegin(&numPasses, getSSTexture())) {
      for (int i=0; i<numPasses; i++) {
        mesh->effect->renderPass(i);
        glBegin(GL_TRIANGLES);
        for (unsigned int f=0; f<mesh->face.size(); f++) {
          //glNormal3fv(mesh->face[f].facenormal);

          glNormal3fv(mesh->norm[mesh->face[f].nr[0]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[0]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[0]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[1]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[1]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[1]]);

          glNormal3fv(mesh->norm[mesh->face[f].nr[2]]);
          glTexCoord2fv(mesh->texco[mesh->face[f].tc[2]]);
          glVertex3fv(mesh->vert[mesh->face[f].vt[2]]);
        }
        glEnd();
      }
      mesh->effect->renderEnd();
    }
  }
}


// default callback functions

void PApp::config()
{
}

void PApp::load()
{
}

void PApp::unload()
{
}

void PApp::tick(float delta)
{
  delta = delta;
}

void PApp::resize()
{
}

void PApp::render(float eyetranslation)
{
  eyetranslation = eyetranslation;
  
  glClearColor(0.5, 0.5, 0.5, 0.0);
  
  glClear(GL_COLOR_BUFFER_BIT);
}

void PApp::keyEvent(const SDL_KeyboardEvent &ke)
{
  if (ke.type != SDL_KEYDOWN) return;

  switch (ke.keysym.sym) {
  case SDLK_ESCAPE:
    requestExit();
    break;
  default:
    break;
  }
}

void PApp::mouseButtonEvent(const SDL_MouseButtonEvent &mbe)
{
  int unused = mbe.type; unused = unused;
}

void PApp::mouseMoveEvent(int dx, int dy)
{
  dx = dx; dy = dy;
}

void PApp::cursorMoveEvent(int posx, int posy)
{
  posx = posx; posy = posy;
}

void PApp::joyButtonEvent(int which, int button, bool down)
{
  which = which; button = button; down = down;
}


