#ifndef Configurator_h
#define Configurator_h

#include <string>
#include <vector>
#include <map>

class Configurator
{
 public:
  virtual void getHistosToFetch(std::vector<std::string> & value) {_instance->getHistosToFetch(value) ;};
  virtual void getHistosToFetchAndSource(std::map<std::string, std::vector<std::string> > & value) {_instance->getHistosToFetchAndSource(value) ;};
  virtual ~Configurator() 
    {
      if(_instance)
	{
	  delete _instance ;
	}
    }
  Configurator(std::string filename) ;
 protected:
  Configurator() {_instance = 0;} ;
 private:
  Configurator * _instance ;
  Configurator(Configurator&) ;
  Configurator operator=(Configurator&);
};

#endif
