#include "Configurator.h"
#include "ConfiguratorXML.h"


Configurator::Configurator(std::string filename)
{
  if(filename.find(".xml") != std::string::npos)
    {
      _instance = new ConfiguratorXML(filename) ;
    }
} 
