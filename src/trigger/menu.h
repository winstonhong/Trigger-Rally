
// menu.h

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


/*

The organisation of this is horrible, I'm afraid... the menu.* files
contain both menu GUI implementation and level progress logic for
the game. Ick.

*/

// menus
#define AM_TOP                21  // Event / Practice / Single race / Quit
#define AM_TOP_EVT            22  // Choose event to race
#define AM_TOP_EVT_PREP       23  // Shows event progress
#define AM_TOP_EVT_ABANDON    24  // Really leave event?
#define AM_TOP_PRAC           25  // Choose event to practice
#define AM_TOP_PRAC_SEL       26  // Choose level to practice
#define AM_TOP_PRAC_SEL_PREP  27  // Show track info (practice)
#define AM_TOP_LVL            28  // Choose single race
#define AM_TOP_LVL_PREP       29  // Show track info (single race)
#define AM_TOP_QUIT           30  // Are you sure you want to quit?

// actions
#define AA_INIT               1
#define AA_RESUME             2
#define AA_GO_TOP             11
#define AA_GO_EVT             21
#define AA_PICK_EVT           22
#define AA_START_EVT          23
#define AA_RESUME_EVT         24
#define AA_RESTART_EVT        25
#define AA_GO_PRAC            31
#define AA_PICK_PRAC          32
#define AA_PICK_PRAC_LVL      33
#define AA_START_PRAC         34
#define AA_GO_LVL             41
#define AA_PICK_LVL           42
#define AA_START_LVL          43
#define AA_GO_QUIT            51
#define AA_QUIT_CONFIRM       52


struct LevelState {
  
  int state;
  
  int currentevent;
  int currentlevel;
  
  int livesleft;
  float totaltime;
  
  std::vector<float> leveltimes;
};


#define GWT_FREE              0
#define GWT_CONTAINER         10
#define GWT_FILLER            11
#define GWT_LABEL             12
#define GWT_GRAPHIC           13

#define GWPARENT_NONE         -1

struct GuiWidget {
  
  int type;
  
  bool clickable;
  int d1, d2;
  
  std::string text;
  float fontsize;
  
  vec2f
    dims_min,
    pos;
  
  float glow;
  
  PTexture *tex;
  
  GuiWidget(int t) : type(t), clickable(false), glow(0.0f) { }
};


class Gui {
  
private:
  std::vector<GuiWidget> widget;
  
  PSSRender *ssRender;
  
  vec2f cursor;
  
  int highlight, defwidget;
  
  float defflash;
  
  PTexture *fonttex;
  
protected:
  int getFreeWidget();
  
  void measureWidgetTree(int w);
  void placeWidgetTree(int w);
  
  void renderWidgetTree(int w);
  
public:
  Gui() : cursor(vec2f::zero()), defflash(0.0f) { }
  
  void setSSRender(PSSRender &render) { ssRender = &render; }
  void setFont(PTexture *tex) { fonttex = tex; }
  
  void tick(float delta);
  
  void setCursorPos(float x, float y);
  
  bool getClickAction(int &data1, int &data2);
  bool getDefaultAction(int &data1, int &data2);
  
  void doLayout();
  
  void render();
  
  void clear() { widget.clear(); highlight = -1; defwidget = -1; }
  
  int addContainer(int parent, float minwidth, float minheight, bool vert);
  
  int addLabel(float x, float y, const std::string &text, uint32 flags, float fontsize);
  
  int addGraphic(float x, float y, float width, float height, PTexture *tex);
  
  int makeClickable(int w, int data1, int data2) {
    widget[w].clickable = true;
    widget[w].d1 = data1;
    widget[w].d2 = data2;
    return w;
  }
  
  int makeUnclickable(int w) {
    widget[w].clickable = false;
    return w;
  }
  
  void makeDefault(int w) { defwidget = w; }
};


