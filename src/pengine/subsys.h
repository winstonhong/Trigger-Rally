
// subsys.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



class PResource {
protected:
  std::string name;

public:
  const std::string &getName() { return name; }
};

template <class T>
class PResourceList {
private:
  std::vector<T *> reslist;

public:
  
  ~PResourceList() { clear(); }
  
  T *add(T *newresource) {
    reslist.push_back(newresource);
    return newresource;
  }
  T *find(const std::string &name) {
    for (typename std::vector<T *>::iterator i = reslist.begin(); i != reslist.end(); i++)
      if (name == (*i)->getName())
        return *i;
    return null;
  }

  void clear() {
    for (typename std::vector<T *>::iterator i = reslist.begin(); i != reslist.end(); i++) {
      delete *i;
    }
    reslist.clear();
  }
};


class PSubsystem {
protected:
  PApp &app;
  
private:
  // init() is deprecated, should use RAII now
  virtual bool init() { return true; }

public:
  PSubsystem(PApp &parentApp) : app(parentApp) { }
  virtual ~PSubsystem() { }
  virtual void tick(float delta, const vec3f &eyepos, const mat44f &eyeori, const vec3f &eyevel)
  { float blah = delta; blah = eyepos.x; blah = eyeori.row[0][0]; blah = eyevel.x; }
};



