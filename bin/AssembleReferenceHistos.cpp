#include "ConfiguratorXML.h"
#include "FWCore/ServiceRegistry/interface/ServiceRegistry.h"
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include <iostream>
#include <fstream>
#include "classlib/utils/RegexpMatch.h"
#include "classlib/utils/Regexp.h"
#include "classlib/utils/StringOps.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// BOOST RegExp
#include "boost/regex.hpp"

std::string get_uuidgen(std::string) ;

// -------------------------------------------------------------------
// Main program.
int main(int argc, char **argv)
{
  // Check command line arguments.
  char *cfgfile = (argc > 1 ? argv[1] : 0);
  if (! cfgfile)
    {
      std::cerr << "Usage: " << argv[0]
		<< " XML_FILE [OUTPUT_ ROOT_FILE]\n";
      return 1;
    }
  std::string rootOutputFile ;
  char *output = (argc > 2 ? argv[2] : 0);

  // Process each file given as argument.
  edm::ParameterSet emptyps;
  std::vector<edm::ParameterSet> emptyset;
  edm::ServiceToken services(edm::ServiceRegistry::createSet(emptyset));
  edm::ServiceRegistry::Operate operate(services);   
  //   edm::ParameterSet ps;
  DQMStore store(emptyps);
  store.setVerbose(2) ;

  // Initialize configurator parser
  Configurator cfg(cfgfile) ;

  if (! output)
    {
      rootOutputFile = cfg.getSinceAndTagFromMetaData() ;
      if( system("which uuidgen &> /dev/null") == 0)
	{
	  rootOutputFile += std::string("@") ;
	  rootOutputFile += get_uuidgen(std::string("uuidgen -t")) ;
	}
      rootOutputFile += std::string(".root") ;
    }
  else 
    rootOutputFile = std::string(output) ;

  if(rootOutputFile.find(".root") == std::string::npos)
    rootOutputFile += ".root" ;
  std::cout << "\nUsing     Config file: " << cfgfile 
	    << "\nUsing Output     file: " << rootOutputFile 
	    << std::endl ;



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
	  store.cd("/") ;
	  std::vector<MonitorElement*> allHistos = store.getAllContents("") ;
	  std::cout << "I discovered a total of " << allHistos.size() << " ME" << std::endl ;
	  for(unsigned int j = 0 ; j < refHistos.size() ; ++j)
	  {
	    std::vector<MonitorElement*> res = store.getMatchingContents(refHistos[j], lat::Regexp::Perl);
	    std::vector<MonitorElement*>::iterator ime = res.begin() ;
	    std::vector<MonitorElement*>::iterator eme = res.end() ;
	    for (; ime!=eme; ++ime)
	    {
	      // skip reference histograms
 	      if ( (*ime)->getPathname().compare(0, 10, "Reference/") != 0)
	      {
		std::cout << "Match against: " << (*ime)->getPathname()
			  << " " << (*ime)->getName() << std::endl;
		toKeepFromPreviousSource.push_back(*ime) ;
	      }
	    }
	  }
	  // Remove all other MEs that do not match the current RegExp.
	  for (unsigned int j = 0; j < allHistos.size(); ++j)
	  {
	    std::vector<MonitorElement*>::iterator ime = toKeepFromPreviousSource.begin() ;
	    std::vector<MonitorElement*>::iterator eme = toKeepFromPreviousSource.end() ;
	    bool skip = false ;
	    for(; ime != eme ; ++ime)
	    {
	      if(*ime == allHistos[j])
	      {
		skip = true ;
		break ;
	      }
	    }
	    if(!skip)
	    {
	      store.removeElement(allHistos[j]->getPathname(), allHistos[j]->getName()) ;
	      //  			      std::cout << "Removing "  << me.getName() 
	      //  					<< "from "      << me.getPathname() 
	      // 					<< "\tsize is " << allHistos.size() 
	      // 				<< std::	endl ;
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

std::string get_uuidgen(std::string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  result.erase(result.length()-1) ; // get rid of final \n character
  
  pclose(pipe);
  return result;
}

  
