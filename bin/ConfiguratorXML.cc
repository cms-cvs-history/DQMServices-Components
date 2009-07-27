#include "ConfiguratorXML.h"
#include <xercesc/util/OutOfMemoryException.hpp>
#include <stdexcept>

XERCES_CPP_NAMESPACE_USE

ConfiguratorXML::ConfiguratorXML(std::string filename)
{
  _ConfigFileParser = 0 ; 
  _filename = filename ;
}

void ConfiguratorXML::initialize()
{
  std::cout << "ConfiguratorXML::initialize()" << std::endl ;
  // Initialize Xerces System
  try 
    {
      XMLPlatformUtils::Initialize();
    }
  catch (const XMLException& toCatch) 
    {
      // Do your failure processing here
      char* message = XMLString::transcode( toCatch.getMessage() );
      std::cerr << "XML toolkit initialization error: " << message << std::endl;
      XMLString::release( &message );
    }
  std::cout << "ConfiguratorXML::initialize() - end" << std::endl ;
}

void ConfiguratorXML::getHistosToFetch(std::vector<std::string> & histosToFetch) 
{
  std::cout << "ConfiguratorXML::getHistosToFetch()" << std::endl ;
  initialize() ;
  _ConfigFileParser = new XercesDOMParser() ;

  DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();                                                                                                                            
  _ConfigFileParser->setErrorHandler(errReporter);   
  
  struct stat fileStatus;
  
  if( stat(_filename.c_str(), &fileStatus) == -1 )
    throw ( std::runtime_error(strerror(int errno) ) );
  
  // Configure DOM parser.
  
  _ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
  _ConfigFileParser->setDoNamespaces( false );
  _ConfigFileParser->setDoSchema( false );
  _ConfigFileParser->setLoadExternalDTD( false );
  
  try
    {
      _ConfigFileParser->parse( _filename.c_str() );
      // no need to free this pointer - owned by the parent parser object
      DOMDocument* xmlDoc = _ConfigFileParser->getDocument();
      
      // Get the top-level element: NAme is "root". No attributes for "root"
      
      DOMElement* elementRoot = xmlDoc->getDocumentElement();
      if( !elementRoot ) throw(std::runtime_error( "empty XML document" ));
      
      // Parse XML file for tags of interest: "SubDetectors"
      // Look one level nested within "root". (child of root)
      
      DOMNodeList*      subdets = elementRoot->getElementsByTagName(XMLString::transcode("SubDetectors"));
      const  XMLSize_t nodeCount = subdets->getLength();
      // For all nodes, children of "root" in the XML tree.
      
      for( XMLSize_t xx = 0; xx < nodeCount; ++xx )
	{
	  DOMNode* currentNode = subdets->item(xx);
	  if( currentNode->getNodeType() &&  // true is not NULL
	      currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is a Specific SubDetector Element
	    {
	      // Found node which is an Element. Re-cast node as element
	      DOMElement* currentElement
		= dynamic_cast< xercesc::DOMElement* >( currentNode );
	      std::string subdetName( XMLString::transcode( currentElement->getAttribute(XMLString::transcode("name")) ) ) ;
	      std::string category( XMLString::transcode( currentElement->getAttribute(XMLString::transcode("category")) ) ) ;
	      DOMNodeList*      subdets_refhistos = currentElement->getChildNodes() ;
	      const  XMLSize_t refHistosCount = subdets_refhistos->getLength();
	      for( XMLSize_t i = 0; i < refHistosCount; ++i )
		{
		  DOMNode* aref = subdets_refhistos->item(i);
		  if( aref->getNodeType() &&  // true is not NULL
		      aref->getNodeType() == DOMNode::ELEMENT_NODE ) // is a Specific SubDetector Element
		    {
		      std::stringstream fullName ;
		      // Found node which is an Element. Re-cast node as element
		      DOMElement* arefel
			= dynamic_cast< xercesc::DOMElement* >( aref );
		      fullName << ".*?" 
			       << subdetName 
			       << "/" 
			       << category
			       << "/" 
			       << XMLString::transcode(arefel->getAttribute(XMLString::transcode("name")))  
			       << ".*?" ;
		      histosToFetch.push_back(fullName.str()) ;
		      std::cout << "Adding fullName: " << fullName.str()  << std::endl ;
		    }
		}
	    }
	}
    }
  catch( xercesc::XMLException& e )
    {
      char* message = xercesc::XMLString::transcode( e.getMessage() );
      std::ostringstream errBuf;
      errBuf << "Error parsing file: " << message << std::flush;
      XMLString::release( &message );
    }  
}

void ConfiguratorXML::getHistosToFetchAndSource( std::map<std::string, std::vector<std::string> > & histosToFetchAndSource) 
{
  std::string mthn("[ConfiguratorXML::getHistosToFetchAndSource]\t") ;
  std::cout << mthn << std::endl ;
  initialize() ;
  _ConfigFileParser = new XercesDOMParser() ;
  
  DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();                                                                                                                            
  _ConfigFileParser->setErrorHandler(errReporter);   
  
  struct stat fileStatus;
  
  if( stat(_filename.c_str(), &fileStatus) == -1 )
    throw ( std::runtime_error(strerror(int errno) ) );
  
  // Configure DOM parser.
  
  _ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Auto );
  _ConfigFileParser->setDoNamespaces( false );
  _ConfigFileParser->setDoSchema( false );
  _ConfigFileParser->setValidationSchemaFullChecking( false );                                                                                                                              

  try
    {
      _ConfigFileParser->parse( _filename.c_str() );

      // no need to free this pointer - owned by the parent parser object
      DOMDocument* xmlDoc = _ConfigFileParser->getDocument();
      
      // Get the top-level element: NAme is "root". No attributes for "root"
      
      DOMElement* elementRoot = xmlDoc->getDocumentElement();
      if( !elementRoot ) throw(std::runtime_error( "empty XML document" ));
      
      // Parse XML file for tags of interest: "SubDetectors"
      // Look one level nested within "root". (child of root)
      
      DOMNodeList*      subdets = elementRoot->getElementsByTagName(XMLString::transcode("SubDetectors"));
      const  XMLSize_t nodeCount = subdets->getLength();
      // For all nodes, children of "root" in the XML tree.
      
      for( XMLSize_t xx = 0; xx < nodeCount; ++xx )
	{
	  DOMNode* currentNode = subdets->item(xx);
	  if( currentNode->getNodeType() &&  // true is not NULL
	      currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is a Specific SubDetector Element
	    {
	      // Found node which is an Element. Re-cast node as element
	      DOMElement* currentElement
		= dynamic_cast< xercesc::DOMElement* >( currentNode );
	      std::string subdetName( XMLString::transcode( currentElement->getAttribute(XMLString::transcode("name"    ) ) ) ) ;
	      std::string category  ( XMLString::transcode( currentElement->getAttribute(XMLString::transcode("category") ) ) ) ;
	      std::string source    ( XMLString::transcode( currentElement->getAttribute(XMLString::transcode("source"  ) ) ) ) ;
	      DOMNodeList*      subdets_refhistos = currentElement->getChildNodes() ;
	      const  XMLSize_t refHistosCount = subdets_refhistos->getLength();
	      std::vector<std::string> tmp_histos ;
	      std::cout << mthn << "Extracting from source: " << source << std::endl ;
	      for( XMLSize_t i = 0; i < refHistosCount; ++i )
		{
		  DOMNode* aref = subdets_refhistos->item(i);
		  if( aref->getNodeType() &&  // true is not NULL
		      aref->getNodeType() == DOMNode::ELEMENT_NODE ) // is a Specific SubDetector Element
		    {
		      std::stringstream fullName ;
		      // Found node which is an Element. Re-cast node as element
		      DOMElement* arefel
			= dynamic_cast< xercesc::DOMElement* >( aref );
		      fullName << ".*?" 
			       << subdetName 
			       << "/" 
			       << category
			       << "/" 
			       << XMLString::transcode(arefel->getAttribute(XMLString::transcode("name")))  
			       << ".*?" ;
		      tmp_histos.push_back(fullName.str()) ;
		      std::cout << mthn << "Adding fullName: " << fullName.str()  << std::endl ;
		    }
		}
	      // append histos to the same input source!!
	      if(histosToFetchAndSource.find(source) != histosToFetchAndSource.end())
		{
		  // 		  std::cout << mthn << "Appending new histos to previous source: " << source << std::endl ;
		  // 		  std::cout << mthn << "Before " << _histosToFetchAndSource[source].size() << std::endl ;
		  histosToFetchAndSource[source].insert(histosToFetchAndSource[source].end(), 
							tmp_histos.begin(), 
							tmp_histos.end());
		  // 		  std::cout << mthn << "After  " << _histosToFetchAndSource[source].size() << std::endl ;
		}
	      else
		{
		  // 		  std::cout << mthn << "Appending new histos to new source: " << source << std::endl ;
		  histosToFetchAndSource[source] = tmp_histos ;
		}
	    }
	}
    }
  catch (const OutOfMemoryException&)
    {
      std::cerr << "OutOfMemoryException" << std::endl;
    }
  catch( const XMLException& e )
    {
      char* message = XMLString::transcode( e.getMessage() );
      std::ostringstream errBuf;
      errBuf << "Error parsing file: " << message << std::flush;
      std::cout << mthn << errBuf << std::endl ;
      XMLString::release( &message );
    }  
  //   catch (...)
  //     {
  //       std::cerr << "An error occurred during parsing\n " << std::endl;
  //     }
}

ConfiguratorXML::~ConfiguratorXML()
{
  // Terminate XErces system
  XMLPlatformUtils::Terminate();
}