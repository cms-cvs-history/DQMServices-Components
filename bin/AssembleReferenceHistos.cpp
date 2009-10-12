#include "ConfiguratorXML.h"
#include "FWCore/ServiceRegistry/interface/ServiceRegistry.h"
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include <iostream>
#include <fstream>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// BOOST RegExp
#include "boost/regex.hpp"

// -------------------------------------------------------------------
// Main program.
int main(int argc, char **argv)
{
  // Check command line arguments.
  char *output = (argc > 1 ? argv[1] : 0);
  if (! output)
    {
      std::cerr << "Usage: " << argv[0]
		<< " OUTPUT-FILE CFGFILE\n";
      return 1;
    }
  char *cfgfile = (argc > 2 ? argv[2] : 0);
  if (! cfgfile)
    {
      std::cerr << "Usage: " << argv[0]
		<< " OUTPUT-FILE CFGFILE\n";
      return 1;
    }

  std::string rootOutputFile(output) ;
  if(rootOutputFile.find(".root") == std::string::npos)
    rootOutputFile += ".root" ;
  std::cout << "\nUsing     Config file: " << cfgfile 
	    << "\nUsing Output     file: " << rootOutputFile 
	    << std::endl ;

  // Process each file given as argument.
  edm::ParameterSet emptyps;
  std::vector<edm::ParameterSet> emptyset;
  edm::ServiceToken services(edm::ServiceRegistry::createSet(emptyset));
  edm::ServiceRegistry::Operate operate(services);   
  //   edm::ParameterSet ps;
  DQMStore store(emptyps);
  store.setVerbose(0) ;

  Configurator cfg(cfgfile) ;
  //   std::vector<std::string>  refHistos = cfg->getHistosToFetch() ;
  std::map<std::string, std::vector<std::string> > refHistosAndSource ;
  cfg.getHistosToFetchAndSource( refHistosAndSource ) ;

  std::map<std::string, std::vector<std::string> >::iterator i = refHistosAndSource.begin() ;
  std::map<std::string, std::vector<std::string> >::iterator e = refHistosAndSource.end() ;

  std::vector<MonitorElement*> toKeepFromPreviousSource ;
  for (; i!=e ; ++i)
    {
      try
	{
	  std::vector<std::string> & refHistos = i->second ;
	  // Read in the file.
	  std::cout << "Working on " << i->first << std::endl ;
	  std::cout << "Requested ref histos " << i->second.size() << std::endl ;
	  store.open(i->first);
	  std::cout << i->first << std::endl ;
	  std::vector<MonitorElement*> allHistos = store.getAllContents("") ;
	  std::cout << "I discovere a total of " << allHistos.size() << " ME" << std::endl ;
	  // Select histograms to keep from configuration files and remove all others that do not match
	  unsigned int allHistosSize = allHistos.size() ;
	  //       std::vector<MonitorElement*>::iterator e = allHistos.end() ;
	  for(unsigned int k = 0 ; k < allHistosSize  ; ++k)
	    {
	      if( k%10000 == 0 )
		std::cout << "[" << k << "/" << allHistosSize << "]"<< std::endl ;
	      MonitorElement &me = *allHistos[k];
	      std::string thisHisto(me.getFullname()) ;
	      // 	      std::cout << "Analizing " << me.getFullname() << std::endl ;
	      bool remove = true  ;
	      // remove Reference Histos by definition ...
	      if (!(thisHisto.size() > 10 && thisHisto.compare(0, 10, "Reference/") == 0) )
		{
		  for(unsigned int j = 0 ; j < refHistos.size() ; ++j)
		    {
//  		      std::cout << "\tComparing " << thisHisto << " against " << refHistos[j] << std::endl ;
		      boost::regex rge(refHistos[j].c_str()) ;
		      boost::smatch what;
		      if(boost::regex_match(thisHisto, what, rge))
			{
			  std::cout << "I've got a match on " << thisHisto  << " and " << refHistos[j] << std::endl ;
			  remove = false ;
			  toKeepFromPreviousSource.push_back(allHistos[k]) ;
			  break ;
			  exit(0) ;
			}
		    }
		}
	      if(remove)
		{
		  std::vector<MonitorElement*>::iterator ime = toKeepFromPreviousSource.begin() ;
		  std::vector<MonitorElement*>::iterator eme = toKeepFromPreviousSource.end() ;
		  bool skip = false ;
		  for(; ime != eme ; ++ime)
		    {
		      if(*ime == allHistos[k])
			{
			  skip = true ;
			  break ;
			}
		    }
		  if(!skip)
		    store.removeElement(me.getFullname()) ;
		  // 	      std::cout << "Removing " << thisHisto << "\tsize is " << allHistos.size() << std::endl ;
		}
	    }
	}
      catch (std::exception &e)
	{
	  std::cerr << "*** FAILED TO READ FILE " << i->first << ":\n"
		    << e.what() << std::endl;
	  exit(1);
	}
    }

  // Compose the correct ROOT & METADATA output file name, with DataSet and 
  // SoftwareVersion as specified into the XML configuration file
  std::string toBeAppended(cfg.getDatasetAndSoftwareVersionAndTag()) ;
  toBeAppended += ".root" ;
  rootOutputFile.replace(rootOutputFile.find(".root"), toBeAppended.length(), toBeAppended) ;
  std::vector<MonitorElement*> toBeSaved = store.getAllContents("") ;
  unsigned int total = toBeSaved.size() ;
  for(unsigned int j = 0 ; j < total ; ++j)
    std::cout << toBeSaved[j]->getFullname() << std::endl ;

  DQMStore::SaveReferenceTag ref = DQMStore::SaveWithoutReference ;
  store.save(rootOutputFile.c_str(), "", "", "", ref);
  
  std::vector<std::pair< std::string, std::string> > metainfo ;
  cfg.getMetadataInfo(metainfo) ;
  std::vector<std::pair<std::string, std::string> >::iterator it  = metainfo.begin() ;
  std::vector<std::pair<std::string, std::string> >::iterator ite = metainfo.end() ;

  std::string metaOutputFileName(rootOutputFile) ;
  metaOutputFileName.replace(metaOutputFileName.find(".root"), metaOutputFileName.length(), ".txt") ;
  std::ofstream metaos(metaOutputFileName.c_str()) ;
  if(!metaos)
    {
      std::cout << "Error opening " << metaOutputFileName << std::endl ;
      exit(1) ;
    }
  for(; it != ite ; ++it)
    metaos << it->first << " " << it->second << std::endl ;
  
  metaos.close() ;

  // Compose the correct TOTAL XML output file name, with DataSet and 
  // SoftwareVersion as specified into the XML configuration file

  std::string xmlOutputFileName(rootOutputFile) ;
  xmlOutputFileName.replace(xmlOutputFileName.find(".root"), metaOutputFileName.length(), ".xml") ;
  cfg.saveFinalXML(xmlOutputFileName) ;

  return 0;
}
  