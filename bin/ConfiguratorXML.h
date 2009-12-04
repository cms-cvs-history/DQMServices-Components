#ifndef ConfiguratorXML_h
#define ConfiguratorXML_h

#include "Configurator.h"
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class ConfiguratorXML : public Configurator
{
 public:
  virtual void getHistosToFetch(std::vector<std::string> & ) ;
  virtual void  getHistosToFetchAndSource(std::map<std::string, std::vector<std::string> > &) ;
  virtual std::string getDatasetAndSoftwareVersionAndTag() ;
  virtual std::string getSinceAndTagFromMetaData() ;
  virtual void getMetadataInfo(std::vector<std::pair<std::string, std::string> > & metainfo) ;
  void    saveFinalXML(std::string filename) ;
  virtual ~ConfiguratorXML() ; 
 private:
  ConfiguratorXML(std::string) ;
  ConfiguratorXML(ConfiguratorXML&) ;
  ConfiguratorXML operator=(ConfiguratorXML&) ;
  virtual void initialize() ;
  xercesc::XercesDOMParser * _ConfigFileParser;
  std::string _filename ;
  friend class Configurator ;
};

XERCES_CPP_NAMESPACE_USE

class DOMTreeErrorReporter : public ErrorHandler
{
 public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  DOMTreeErrorReporter() :
    fSawErrors(false)
    {;}

  ~DOMTreeErrorReporter()
    {;}


  // -----------------------------------------------------------------------
  //  Implementation of the error handler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& toCatch) 
    {
      fSawErrors = true;
      std::string mthn("[DOMTreeErrorReporter]\t") ;
      std::cerr << mthn << "Error at file \"" << XMLString::transcode(toCatch.getSystemId())
		<< "\", line " << toCatch.getLineNumber()
		<< ", column " << toCatch.getColumnNumber()
		<< "\n" << mthn << "Message: " << XMLString::transcode(toCatch.getMessage()) << std::endl;
      exit(-1) ;
    }
  void error(const SAXParseException& toCatch) {warning(toCatch) ;}
  void fatalError(const SAXParseException& toCatch){warning(toCatch);};
  void resetErrors() {fSawErrors = false ;};

  // -----------------------------------------------------------------------
  //  Getter methods
  // -----------------------------------------------------------------------
  bool getSawErrors() const;

  // -----------------------------------------------------------------------
  //  Private data members
  //
  //  fSawErrors
  //      This is set if we get any errors, and is queryable via a getter
  //      method. Its used by the main code to suppress output if there are
  //      errors.
  // -----------------------------------------------------------------------
  bool    fSawErrors;
};

inline bool DOMTreeErrorReporter::getSawErrors() const
{
  return fSawErrors;
}


#endif

