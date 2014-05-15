
// exception.h [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)



class PException : public std::exception
{
public:
  PException (const std::string &text) : text_ (text) { }
  
  ~PException () throw () { }
  
  const char *what () const throw ()
  {
    return text_.c_str ();
  }
  
private:
  std::string text_;
};



#define makestring2(x) #x
#define makestring(x) makestring2(x)

#define MakePException(text) PException (std::string () + text + std::string (" at " __FILE__ ":" makestring(__LINE__)))

