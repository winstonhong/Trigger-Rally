
// menu.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#include <sstream>
#include "main.h"

const int MAX_RACES_ON_SCREEN = 10;


void MainApp::levelScreenAction(int action, int index)
{
  appstate = AS_LEVEL_SCREEN;
  
  switch (action) {
  case AA_INIT:
    lss.state = AM_TOP;
    break;
  case AA_RESUME:
    // lss.state should be AM_TOP_EVT_PREP, continuing event
    break;
  case AA_GO_TOP:
    lss.state = AM_TOP;
    break;
  case AA_GO_EVT:
    if (lss.state == AM_TOP_EVT_PREP &&
      lss.currentlevel > 0 &&
      lss.currentlevel < (int)events[lss.currentevent].levels.size()) {
      lss.state = AM_TOP_EVT_ABANDON;
    } else {
      lss.state = AM_TOP_EVT;
    }
    break;
  case AA_PICK_EVT:
    lss.currentevent = index;
    lss.currentlevel = 0;
    lss.livesleft = 3;
    lss.leveltimes.clear();
    lss.totaltime = 0.0f;
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_RESUME_EVT:
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_RESTART_EVT:
    lss.currentlevel = 0;
    lss.livesleft = 3;
    lss.leveltimes.clear();
    lss.totaltime = 0.0f;
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_GO_PRAC:
    lss.state = AM_TOP_PRAC;
    break;
  case AA_PICK_PRAC:
    lss.currentevent = index;
    lss.state = AM_TOP_PRAC_SEL;
    break;
  case AA_PICK_PRAC_LVL:
    lss.currentlevel = index;
    lss.state = AM_TOP_PRAC_SEL_PREP;
    break;
  case AA_GO_LVL:
    lss.state = AM_TOP_LVL;
    break;
  case AA_PICK_LVL:
    lss.currentlevel = index;
    lss.state = AM_TOP_LVL_PREP;
    break;
  case AA_GO_QUIT:
    lss.state = AM_TOP_QUIT;
    break;
  case AA_QUIT_CONFIRM:
    quitGame();
    break;
    
  case AA_START_EVT:
    startGame(events[lss.currentevent].levels[lss.currentlevel].filename);
    return;
  case AA_START_PRAC:
    startGame(events[lss.currentevent].levels[lss.currentlevel].filename);
    return;
  case AA_START_LVL:
    startGame(levels[lss.currentlevel].filename);
    return;
  
  default:
    PUtil::outLog() << "ERROR: invalid action code " << action << std::endl;
    requestExit();
    return;
  }
  
  gui.setSSRender(getSSRender());
  
  gui.setFont(tex_font);
  
  grabMouse(false);
  
  gui.clear();
  
  gui.addLabel(10.0f,590.0f, (std::string)"Trigger " + PACKAGE_VERSION, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
  
  {
    // Switch the icky M D Y date format to D M Y
    // TODO: do this at startup
    
    const char *ickydate = __DATE__;
    char pieces[3][10];
    char resultdate[30];
    sscanf(ickydate, "%s %s %s", pieces[0], pieces[1], pieces[2]);
    sprintf(resultdate, "%s %s %s", pieces[1], pieces[0], pieces[2]);
    
    gui.addLabel(10.0f,10.0f,
      (std::string)"Build: " + resultdate + " at " + __TIME__,
      PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 20.0f);
  }
  
  switch (lss.state) {
  case AM_TOP:
    gui.makeClickable(
      gui.addLabel(400.0f,400.0f, "Enter an Event", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f), AA_GO_EVT, 0);
    gui.makeClickable(
      gui.addLabel(400.0f,350.0f, "Practice", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f), AA_GO_PRAC, 0);
    gui.makeClickable(
      gui.addLabel(400.0f,300.0f, "Single Race", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f), AA_GO_LVL, 0);
    gui.makeClickable(
      gui.addLabel(400.0f,250.0f, "Quit", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f), AA_GO_QUIT, 0);
    break;
  case AM_TOP_EVT:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f), AA_GO_TOP, 0);
    gui.addLabel(790.0f,590.0f, "Event", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(100.0f,470.0f, "Choose Event:", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    for (unsigned int i = 0; i < events.size(); i++) {
      gui.makeClickable(
        gui.addLabel(100.0f,420.0f - (float)i * 30.0f,
        events[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f), AA_PICK_EVT, i);
    }
    break;
  case AM_TOP_EVT_PREP:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f), AA_GO_EVT, 0);
    gui.addLabel(790.0f,590.0f, "Event", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(100.0f,470.0f, "Races:", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    for (unsigned int i = 0; i < events[lss.currentevent].levels.size(); i++) {
      gui.addLabel(100.0f,420.0f - (float)i * 30.0f,
        events[lss.currentevent].levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f);
      std::string infotext = "Not yet raced";
      if (lss.currentlevel > (int)i)
        infotext = PUtil::formatTime(lss.leveltimes[i]);
      else if (lss.currentlevel == (int)i)
        infotext = "NEXT";
      gui.addLabel(700.0f,420.0f - (float)i * 30.0f,
        infotext, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f);
    }
    gui.addLabel(700.0f,420.0f - (float)events[lss.currentevent].levels.size() * 30.0f,
      "Total: " + PUtil::formatTime(lss.totaltime), PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f);
    if (lss.livesleft <= 0) {
      gui.addLabel(400.0f,150.0f, "No tries remaining", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 30.0f);
      gui.makeClickable(
        gui.addLabel(400.0f,100.0f, "Restart Event", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 30.0f),
        AA_RESTART_EVT, 0);
    } else if (lss.currentlevel >= (int)events[lss.currentevent].levels.size()) {
      gui.addLabel(400.0f,150.0f, "CONGRATULATIONS!", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f);
      gui.addLabel(400.0f,100.0f, "EVENT COMPLETED", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f);
    } else {
      gui.addLabel(300.0f,100.0f, "Tries remaining:", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 30.0f);
      for (int i = 0; i < lss.livesleft; i++) {
        gui.addGraphic(325.0f + i * 50.0f,75.0f, 50.0f,50.0f, tex_hud_life);
      }
      gui.makeDefault(
        gui.makeClickable(
          gui.addLabel(650.0f,100.0f, "START RACE", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f),
          AA_START_EVT, 0));
    }
    break;
  case AM_TOP_EVT_ABANDON:
    gui.addLabel(400.0f,350.0f, "Really leave event?", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f);
    gui.makeClickable(
      gui.addLabel(300.0f,250.0f, "Yes", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_GO_EVT, 0);
    gui.makeClickable(
      gui.addLabel(500.0f,250.0f, "No", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_RESUME_EVT, 0);
    break;
  case AM_TOP_PRAC:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f), AA_GO_TOP, 0);
    gui.addLabel(790.0f,590.0f, "Practice", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(100.0f,470.0f, "Choose Event:", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    for (unsigned int i = 0; i < events.size(); i++) {
      gui.makeClickable(
        gui.addLabel(100.0f,420.0f - (float)i * 30.0f,
        events[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f), AA_PICK_PRAC, i);
    }
    break;
  case AM_TOP_PRAC_SEL:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
      AA_GO_PRAC, 0);
    gui.addLabel(790.0f,590.0f, "Practice", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(100.0f,470.0f, "Choose Race:", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    for (unsigned int i = 0; i < events[lss.currentevent].levels.size(); i++) {
      gui.makeClickable(
        gui.addLabel(100.0f,420.0f - (float)i * 30.0f,
          events[lss.currentevent].levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f),
        AA_PICK_PRAC_LVL, i);
    }
    break;
  case AM_TOP_PRAC_SEL_PREP:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
      AA_PICK_PRAC, lss.currentevent);
    gui.addLabel(790.0f,590.0f, "Practice", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,450.0f, "Event", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,450.0f, events[lss.currentevent].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,400.0f, "Name", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,400.0f, events[lss.currentevent].levels[lss.currentlevel].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,350.0f, "Author", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,350.0f, events[lss.currentevent].levels[lss.currentlevel].author, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,300.0f, "Time", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,300.0f, events[lss.currentevent].levels[lss.currentlevel].targettime, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.makeDefault(
      gui.makeClickable(
        gui.addLabel(400.0f,100.0f, "START PRACTICE", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f),
        AA_START_PRAC, 0));
    break;
  case AM_TOP_LVL:
    {
      gui.makeClickable(
        gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
        AA_GO_TOP, 0);
      gui.addLabel(790.0f,590.0f, "Single Race", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
      gui.addLabel(100.0f,470.0f, "Choose Race:", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);

      int firstraceindex = index;
      int prevlabel = gui.addLabel(100.0f, 90.0f, "(prev)", PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
      if (firstraceindex > 0) {
        gui.makeClickable(prevlabel, AA_GO_LVL, firstraceindex - MAX_RACES_ON_SCREEN);
      }
      int racesonscreencount = levels.size() - firstraceindex;
      int nextlabel = gui.addLabel(380.0f, 90.0f, "(next)", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
      if (racesonscreencount > MAX_RACES_ON_SCREEN) {
        racesonscreencount = MAX_RACES_ON_SCREEN;
        gui.makeClickable( nextlabel, AA_GO_LVL, firstraceindex + MAX_RACES_ON_SCREEN);
      }
      std::stringstream racecountmsg;
      racecountmsg << "Races " << firstraceindex + 1 << "-" << firstraceindex + racesonscreencount << " of " << levels.size();
      gui.addLabel(790.0f, 10.0f, racecountmsg.str(), PTEXT_HZA_RIGHT | PTEXT_VTA_BOTTOM, 20.0f);

      for (int i = firstraceindex; i < firstraceindex + racesonscreencount; i++) {
        gui.makeClickable(
          gui.addLabel(100.0f, 420.0f - (float)(i - firstraceindex) * 30.0f,
          levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f),
          AA_PICK_LVL, i);
      }
    }
    break;
  case AM_TOP_LVL_PREP:
    gui.makeClickable(
      gui.addLabel(400.0f,590.0f, "(back)", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
      AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN);
    gui.addLabel(790.0f,590.0f, "Single Race", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,450.0f, "Name", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,450.0f, levels[lss.currentlevel].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,400.0f, "Author", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,400.0f, levels[lss.currentlevel].author, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(200.0f,350.0f, "Time", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 30.0f);
    gui.addLabel(250.0f,350.0f, levels[lss.currentlevel].targettime, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 30.0f);
    gui.makeDefault(
      gui.makeClickable(
        gui.addLabel(400.0f,100.0f, "START RACE", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f),
        AA_START_LVL, 0));
    break;
  case AM_TOP_QUIT:
    gui.addLabel(400.0f,350.0f, "Really quit?", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f);
    gui.makeClickable(
      gui.addLabel(300.0f,250.0f, "Yes", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_QUIT_CONFIRM, 0);
    gui.makeClickable(
      gui.addLabel(500.0f,250.0f, "No", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_GO_TOP, 0);
    break;
  default:
    gui.addLabel(400.0f,300.0f, "Error in menu system, sorry", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f);
    gui.makeClickable(
      gui.addLabel(400.0f,150.0f, "Go to top menu", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
      AA_GO_TOP, 0);
    break;
  }
  
  //gui.doLayout();
}

void MainApp::finishRace(int gamestate, float coursetime)
{
  switch (lss.state) {
  case AM_TOP_EVT_PREP:
    switch (gamestate) {
    case GF_PASS:
      lss.leveltimes.resize(events[lss.currentevent].levels.size(), 0.0f);
      lss.leveltimes[lss.currentlevel] += coursetime;
      lss.totaltime += coursetime;
      lss.currentlevel++;
      break;
    case GF_FAIL:
      lss.totaltime += coursetime;
      lss.livesleft--;
      break;
    default:
      break;
    }
    levelScreenAction(AA_RESUME, 0);
    break;
  case AM_TOP_PRAC_SEL_PREP:
    levelScreenAction(AA_PICK_PRAC_LVL, lss.currentlevel);
    break;
  case AM_TOP_LVL_PREP:
    // Calculate the index of first level in the page by truncating the current level index to the nearest 10
    levelScreenAction(AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN );
    break;
  default:
    PUtil::outLog() << "Race finished in invalid state " << lss.state << std::endl;
    break;
  }
}

void MainApp::tickStateLevel(float delta)
{
  gui.tick(delta);
}

void MainApp::cursorMoveEvent(int posx, int posy)
{
  if (appstate != AS_LEVEL_SCREEN) return;
  
  gui.setCursorPos(
    (float)posx / (float)getWidth() * 800.0f,
    (1.0f - (float)posy / (float)getHeight()) * 600.0f);
}

void MainApp::mouseButtonEvent(const SDL_MouseButtonEvent &mbe)
{
  if (mbe.type != SDL_MOUSEBUTTONDOWN) return;
  
  switch (appstate) {
  case AS_LEVEL_SCREEN:
    break;
  case AS_LOAD_3:
    levelScreenAction(AA_INIT, 0);
    break;
  default:
    return;
  }
  
  gui.setCursorPos(
    (float)mbe.x / (float)getWidth() * 800.0f,
    (1.0f - (float)mbe.y / (float)getHeight()) * 600.0f);
  
  int action, index;
  
  if (!gui.getClickAction(action, index)) return;
  
  levelScreenAction(action, index);
}

void MainApp::handleLevelScreenKey(const SDL_KeyboardEvent &ke)
{
  switch (ke.keysym.sym) {
  case SDLK_ESCAPE:
    switch(lss.state) {
    case AM_TOP:
      levelScreenAction(AA_GO_QUIT, 0);
      break;
    case AM_TOP_EVT_PREP:
    case AM_TOP_EVT_ABANDON:
      levelScreenAction(AA_GO_EVT, 0);
      break;
    case AM_TOP_PRAC_SEL:
      levelScreenAction(AA_GO_PRAC, 0);
      break;
    case AM_TOP_PRAC_SEL_PREP:
      levelScreenAction(AA_PICK_PRAC, lss.currentevent);
      break;
    case AM_TOP_LVL_PREP:
      levelScreenAction(AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN);
      break;
    case AM_TOP_QUIT:
      quitGame();
      break;
    default:
      levelScreenAction(AA_GO_TOP, 0);
      break;
    }
    break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER: {
      int data1, data2;
      
      if (gui.getDefaultAction(data1, data2))
        levelScreenAction(data1, data2);
    } break;
  default:
    break;
  }
}


void MainApp::renderStateLevel(float eyetranslation)
{
  eyetranslation = eyetranslation;
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  
  // draw background image
  
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glDisable(GL_LIGHTING);
  
  tex_splash_screen->bind();
  
  glColor4f(0.0f, 0.0f, 0.2f, 1.0f);
  
  glBegin(GL_QUADS);
  glTexCoord2f(1.0f,1.0f); glVertex2f(800.0f,600.0f);
  glTexCoord2f(0.0f,1.0f); glVertex2f(0.0f,600.0f);
  glTexCoord2f(0.0f,0.0f); glVertex2f(0.0f,0.0f);
  glTexCoord2f(1.0f,0.0f); glVertex2f(800.0f,0.0f);
  glEnd();
  
  // draw GUI
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
  
  tex_font->bind();
  
  glPushMatrix(); // 0
  
  gui.render();
  
  glPopMatrix(); // 0
  
  glBlendFunc(GL_ONE, GL_ZERO);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_FOG);
  glEnable(GL_LIGHTING);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}


void Gui::tick(float delta)
{
  float decay = delta * 3.0f;
  
  for (unsigned int i = 0; i < widget.size(); i++)
  {
    widget[i].glow -= decay;
    CLAMP_LOWER(widget[i].glow, 0.0f);
  }

  if (highlight != -1) {
    widget[highlight].glow = 1.0f;
  }
  
  defflash = fmodf(defflash + delta * 50.0f, PI*2.0f);
}

void Gui::setCursorPos(float x, float y)
{
  highlight = -1;
  
  for (unsigned int i = 0; i < widget.size(); i++) {
    
    if (!widget[i].clickable) continue;
    
    if (x >= widget[i].pos.x &&
      y >= widget[i].pos.y &&
      x < widget[i].pos.x + widget[i].dims_min.x &&
      y < widget[i].pos.y + widget[i].dims_min.y)
      highlight = i;
  }
}

bool Gui::getClickAction(int &data1, int &data2)
{
  if (highlight == -1) return false;
  
  data1 = widget[highlight].d1;
  data2 = widget[highlight].d2;
  
  return true;
}

bool Gui::getDefaultAction(int &data1, int &data2)
{
  if (defwidget == -1) return false;
  
  data1 = widget[defwidget].d1;
  data2 = widget[defwidget].d2;
  
  return true;
}

void Gui::render()
{
  for (unsigned int i = 0; i < widget.size(); i++) {
    
    switch(widget[i].type) {
    case GWT_LABEL: {
      vec4f colc;
      if (widget[i].clickable) {
        vec4f cola = vec4f(1.0f, 0.6f, 0.6f, 0.7f);
        vec4f colb = vec4f(0.4f, 0.8f, 1.0f, 0.85f);
        colc = INTERP(cola, colb, widget[i].glow);
      } else {
        colc = vec4f(1.0f, 1.0f, 1.0f, 0.8f);
      }
      
      if ((int)i == defwidget)
        colc += vec4f(0.1f, -0.1f, -0.1f, 0.0f) * sinf(defflash);
      
      glPushMatrix();
      
      vec2f ctr = widget[i].pos;
      glTranslatef(ctr.x, ctr.y, 0.0f);
      
      glScalef(widget[i].fontsize, widget[i].fontsize, 1.0f);
      
      fonttex->bind();
      
      glColor4fv(colc);
      ssRender->drawText(widget[i].text, PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM);
      glPopMatrix();
      } break;
      
    case GWT_GRAPHIC: {
      
      vec2f min = widget[i].pos;
      vec2f max = widget[i].pos + widget[i].dims_min;
      
      widget[i].tex->bind();
      
      glColor3f(1.0f, 1.0f, 1.0f);
      
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(min.x, min.y);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(max.x, min.y);
      glTexCoord2f(1.0f, 1.0f); glVertex2f(max.x, max.y);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(min.x, max.y);
      glEnd();
      } break;
    }
  }
}

// Widget tree stuff wasn't working properly, so I removed it for
// now. If I need ultra-snazzy menus, I may finish this code

#if 0

void Gui::doLayout()
{
  // Calculate sizes
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].parent == GWPARENT_NONE) {
      measureWidgetTree(i);
      placeWidgetTree(i);
    }
  }
}

void Gui::measureWidgetTree(int w)
{
  widget[w].childcount = 0;
  widget[w].fillercount = 0;
  
  switch (widget[w].type) {
  default:
    
    widget[w].dims_measure = widget[w].dims_min;
    
    break;
    
  case GWT_CONTAINER: {
    
    vec2f measure = vec2f(0.0f, 0.0f);
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        measureWidgetTree(i);
        
        widget[w].childcount++;
        
        if (widget[i].type == GWT_FILLER)
          widget[w].fillercount++;
        
        if (widget[w].vert) {
          CLAMP_LOWER(measure.x, widget[i].dims_measure.x);
          measure.y += widget[i].dims_measure.y;
        } else {
          measure.x += widget[i].dims_measure.x;
          CLAMP_LOWER(measure.y, widget[i].dims_measure.y);
        }
      }
    }
    
    widget[w].dims_measure = measure;
    
    } break;
  }
}

void Gui::placeWidgetTree(int w)
{
  if (widget[w].childcount <= 0) return;
  
  float extraspace = widget[w].vert ?
    - widget[w].dims_measure.x :
    - widget[w].dims_measure.y;
  if (widget[w].parent == GWPARENT_NONE) {
    extraspace += widget[w].vert ?
      widget[w].dims_min.x :
      widget[w].dims_min.y;
  }
  
  CLAMP_LOWER(extraspace, 0.0f);
  
  //CLAMP_LOWER(widget[w].dims_measure.x, widget[w].dims_min.x);
  //CLAMP_LOWER(widget[w].dims_measure.y, widget[w].dims_min.y);
  
  float
    addtofillers = 0.0f,
    addtochildren = 0.0f;
  /*
  if (widget[w].fillercount > 0)
    addtofillers = extraspace / (float)widget[w].fillercount;
  else
    addtochildren = extraspace / (float)widget[w].childcount;*/
    
  if (widget[w].vert) {
    float distrib = widget[w].pos.y;
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        
        widget[i].pos.x = widget[w].pos.x;
        widget[i].pos.y = distrib;
        
        widget[i].dims_measure.x = widget[w].dims_measure.x;
        
        switch (widget[i].type) {
        case GWT_FILLER:
          widget[i].dims_measure.y += addtofillers;
          break;
        case GWT_CONTAINER:
          widget[i].dims_measure.y += addtochildren;
          placeWidgetTree(i);
          break;
        default:
          widget[i].dims_measure.y += addtochildren;
          break;
        }
        
        distrib += widget[i].dims_measure.y;
      }
    }
  } else {
    float distrib = widget[w].pos.x;
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        
        widget[i].pos.x = distrib;
        widget[i].pos.y = widget[w].pos.y;
        
        widget[i].dims_measure.y = widget[w].dims_measure.y;
        
        switch (widget[i].type) {
        case GWT_FILLER:
          widget[i].dims_measure.x += addtofillers;
          break;
        case GWT_CONTAINER:
          widget[i].dims_measure.x += addtochildren;
          placeWidgetTree(i);
          break;
        default:
          widget[i].dims_measure.x += addtochildren;
          break;
        }
        
        distrib += widget[i].dims_measure.x;
      }
    }
  }
}

void Gui::render()
{
  // Render trees of all root containers
  
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].parent == GWPARENT_NONE)
      renderWidgetTree(i);
  }
}

void Gui::renderWidgetTree(int w)
{
  vec2f min, max;
  
  switch (widget[w].type) {
  case GWT_CONTAINER:
    glColor4f(1.0f,0.0f,0.0f,0.2f);
    break;
  case GWT_FILLER:
    glColor4f(0.0f,1.0f,0.0f,0.2f);
    break;
  case GWT_LABEL:
    glColor4f(0.0f,0.0f,1.0f,0.2f);
    break;
  }
  
  min = widget[w].pos;
  max = widget[w].pos + widget[w].dims_measure;
  
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glVertex2f(min.x, min.y);
  glVertex2f(max.x, min.y);
  glVertex2f(max.x, max.y);
  glVertex2f(min.x, max.y);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  
  // Render this widget
  switch (widget[w].type) {
  default:
    break;
    
  case GWT_LABEL: {
    glPushMatrix();
    vec2f ctr = widget[w].pos + widget[w].dims_measure * 0.5f;
    glTranslatef(ctr.x, ctr.y, 0.0f);
    glScalef(widget[w].fontsize, widget[w].fontsize, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    ssRender->drawText(widget[w].text, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
    glPopMatrix();
    } break;
  }
  
  // Render children
  switch (widget[w].type) {
  case GWT_CONTAINER:
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w)
        renderWidgetTree(i);
    }
    break;
  }
}

#endif

int Gui::getFreeWidget()
{
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].type == GWT_FREE)
      return i;
  }
  widget.push_back(GuiWidget(GWT_FREE));
  return widget.size() - 1;
}

#if 0
int Gui::addRootContainer(float x, float y, float width, float height, bool vert)
{
  int w = getFreeWidget();
  widget[w].type = GWT_CONTAINER;
  widget[w].vert = vert;
  widget[w].parent = GWPARENT_NONE;
  widget[w].dims_min = vec2f(width, height);
  widget[w].pos = vec2f(x, y);
  
  return w;
}

int Gui::addFiller(int parent, float minwidth, float minheight)
{
  int w = getFreeWidget();
  widget[w].type = GWT_FILLER;
  widget[w].parent = parent;
  widget[w].dims_min = vec2f(minwidth, minheight);
  
  return w;
}

int Gui::addContainer(float x, float y, float width, float height, bool vert)
{
  int w = getFreeWidget();
  widget[w].type = GWT_CONTAINER;
  widget[w].vert = vert;
  widget[w].parent = parent;
  widget[w].dims_min = vec2f(minwidth, minheight);
  
  return w;
}
#endif

int Gui::addLabel(float x, float y, const std::string &text, uint32 flags, float fontsize)
{
  int w = getFreeWidget();
  widget[w].type = GWT_LABEL;
  widget[w].text = text;
  widget[w].fontsize = fontsize;
  widget[w].dims_min = ssRender->getTextDims(text) * fontsize;
  widget[w].pos = vec2f(x, y);
  
  if (flags & PTEXT_HZA_CENTER)
    widget[w].pos.x -= widget[w].dims_min.x * 0.5f;
  else if (flags & PTEXT_HZA_RIGHT)
    widget[w].pos.x -= widget[w].dims_min.x;
  
  if (flags & PTEXT_VTA_CENTER)
    widget[w].pos.y -= widget[w].dims_min.y * 0.5f;
  else if (flags & PTEXT_VTA_TOP)
    widget[w].pos.y -= widget[w].dims_min.y;
  
  return w;
}

int Gui::addGraphic(float x, float y, float width, float height, PTexture *tex)
{
  int w = getFreeWidget();
  widget[w].type = GWT_GRAPHIC;
  widget[w].dims_min = vec2f(width, height);
  widget[w].pos = vec2f(x, y);
  widget[w].tex = tex;
  
  return w;
}

