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
  virtual std::string getDatasetAndSoftwareVersionAndTag() {return _instance->getDatasetAndSoftwareVersionAndTag();};
  virtual std::string getSinceAndTagFromMetaData() {return _instance->getSinceAndTagFromMetaData();};
  virtual void getMetadataInfo(std::vector<std::pair<std::string,std::string > > & metainfo) {_instance->getMetadataInfo(metainfo) ;} ;
  virtual void saveFinalXML(std::string filename) {_instance->saveFinalXML(filename) ;} ;
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
