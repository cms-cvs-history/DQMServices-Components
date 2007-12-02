/** \file MEtoROOTConverter.cc
 *  
 *  See header file for description of class
 *
 *  $Date: 2007/11/29 13:36:36 $
 *  $Revision: 1.2 $
 *  \author M. Strang SUNY-Buffalo
 */

#include "DQMServices/Components/plugins/MEtoROOTConverter.h"

MEtoROOTConverter::MEtoROOTConverter(const edm::ParameterSet & iPSet) :
  fName(""), verbosity(0), frequency(0), count(0)
{
  std::string MsgLoggerCat = "MEtoROOTConverter_MEtoROOTConverter";

  // get information from parameter set
  fName = iPSet.getUntrackedParameter<std::string>("Name");
  verbosity = iPSet.getUntrackedParameter<int>("Verbosity");
  frequency = iPSet.getUntrackedParameter<int>("Frequency");
  
  // use value of first digit to determine default output level (inclusive)
  // 0 is none, 1 is basic, 2 is fill output, 3 is gather output
  verbosity %= 10;
  
  // print out Parameter Set information being used
  if (verbosity >= 0) {
    edm::LogInfo(MsgLoggerCat) 
      << "\n===============================\n"
      << "Initialized as EDProducer with parameter values:\n"
      << "    Name          = " << fName << "\n"
      << "    Verbosity     = " << verbosity << "\n"
      << "    Frequency     = " << frequency << "\n"
      << "===============================\n";
  }
  
  // get dqm info
  dbe = 0;
  dbe = edm::Service<DaqMonitorBEInterface>().operator->();
  if (dbe) {
    if (verbosity > 0 ) {
      dbe->setVerbose(1);
    } else {
      dbe->setVerbose(0);
    }
  }

  //if (dbe) {
  //  if (verbosity >= 0 ) dbe->showDirStructure();
  //}

  // clear out the vector holders
  items.clear();
  pkgvec.clear();
  pathvec.clear();
  mevec.clear();
  packages.clear();

  // get contents out of DQM
  dbe->getContents(items);
  
  if (!items.empty()) {
    for (i = items.begin (), e = items.end (); i != e; ++i) {

      // verify I have the expected string format
      assert(StringOps::contains(*i,':') == 1);

      // get list of things seperated by :
      StringList item = StringOps::split(*i, ":");

      // get list of directories
      StringList dir = StringOps::split(item[0],"/");

      // keep track of leading directory
      n = dir.begin();
      std::string package = *n;
      ++packages[package];

      // get list of monitor elements
      StringList me = StringOps::split(item[1],",");

      // keep track of package, path and me for each monitor element
      for (n = me.begin(), m = me.end(); n != m; ++n) {
	pkgvec.push_back(package);
	pathvec.push_back(item[0]);
	mevec.push_back(*n);
      }
    }
  }

  if (verbosity > 0) {
    // list unique packeges
    std::cout << "Packages accessing DQM:" << std::endl;
    for (pkgIter = packages.begin(); pkgIter != packages.end(); ++pkgIter) {
      std::cout << "  " << pkgIter->first << std::endl;
    }
    
    std::cout << "Monitor Elements detected:" << std::endl;
    for (unsigned int a = 0; a < pkgvec.size(); ++a) {
      std::cout << "   " << pkgvec[a] << " " << pathvec[a] << " " << mevec[a] 
		<< std::endl;
    }
  }

  // create persistent object
  produces<MEtoROOT, edm::InRun>(fName);

} // end constructor

MEtoROOTConverter::~MEtoROOTConverter() 
{
} // end destructor

void MEtoROOTConverter::beginJob(const edm::EventSetup& iSetup)
{
  return;
}

void MEtoROOTConverter::endJob()
{
  std::string MsgLoggerCat = "MEtoROOTConverter_endJob";
  if (verbosity >= 0)
    edm::LogInfo(MsgLoggerCat) 
      << "Terminating having processed " << count << " runs.";
  return;
}

void MEtoROOTConverter::beginRun(edm::Run& iRun, 
				 const edm::EventSetup& iSetup)
{
  std::string MsgLoggerCat = "MEtoROOTConverter_beginRun";
  
  // keep track of number of runs processed
  ++count;
  
  int nrun = iRun.run();
  
  if (verbosity > 0) {
    edm::LogInfo(MsgLoggerCat)
      << "Processing run " << nrun << " (" << count << " runs total)";
  } else if (verbosity == 0) {
    if (nrun%frequency == 0 || count == 1) {
      edm::LogInfo(MsgLoggerCat)
	<< "Processing run " << nrun << " (" << count << " runs total)";
    }
  }
  
  // clear out object holders
  version.clear();
  name.clear();
  tags.clear();
  object.clear();
  reference.clear();
  qreports.clear();
  flags.clear();

  taglist.clear();
  qreportsmap.clear();

  return;
}

void MEtoROOTConverter::endRun(edm::Run& iRun, const edm::EventSetup& iSetup)
{
 
 
  std::string MsgLoggerCat = "MEtoROOTConverter_endRun";
  
  if (verbosity > 0)
    edm::LogInfo (MsgLoggerCat)
      << "\nStoring MEtoROOT dataformat histograms.";

  // extract ME information into vectors
  for (unsigned int a = 0; a < pkgvec.size(); ++a) {

    taglist.clear();
    qreportsmap.clear();
    flag = 0;

    //set full path
    std::string fullpath;
    fullpath.reserve(pathvec[a].size() + mevec[a].size() + 2);
    fullpath += pathvec[a];
    if (!pathvec[a].empty()) fullpath += "/";
    fullpath += mevec[a];

    //std::cout << std::endl;    
    //std::cout << "name: " << fullpath << std::endl;
    //set name
    name.push_back(fullpath);

    //std::cout << "version: " << s_version.ns() << std::endl;    
    // set version
    version.push_back(s_version.ns());
    
    // get tags
    bool foundtags  = false;
    dqm::me_util::dirt_it idir = dbe->allTags.find(pathvec[a]);
    if (idir != dbe->allTags.end()) {
      dqm::me_util::tags_it itag = idir->second.find(mevec[a]);
      if (itag != idir->second.end()) {
	taglist.resize(itag->second.size());
	std::copy(itag->second.begin(), itag->second.end(), taglist.begin());
	foundtags = true;
      }
    }
    if (!foundtags) taglist.clear();
    //std::cout << "taglist:" << std::endl;
    for (unsigned int ii = 0; ii < taglist.size(); ++ii) {
      std::cout << "   " << taglist[ii] << std::endl;
    }
    tags.push_back(taglist);

    // get monitor elements
    dbe->cd();
    dbe->cd(pathvec[a]);
    bool validME = false;
    
    //std::cout << "MEobject:" << std::endl;
    if (MonitorElement *me = dbe->get(fullpath)) {
      
      if (me->hasError()) flag |= FLAG_ERROR;
      if (me->hasWarning()) flag |= FLAG_WARNING;
      if (me->hasOtherReport()) flag |= FLAG_REPORT;
      
      // Save the ROOT object.  This is either a genuine ROOT object,
      // or a scalar one that stores its value as TObjString.
      if (ROOTObj *ob = dynamic_cast<ROOTObj *>(me)) {
	if (TObject *tobj = ob->operator->()){
	  validME = true;
	  //std::cout << "   normal: " << me->getName() << std::endl;
	  object.push_back(tobj);
	  if (dynamic_cast<TObjString *> (tobj)) {
	    //std::cout << "  normal scalar: " << me->getName() << std::endl;
	    flag |= FLAG_SCALAR;
	  }
	}
      } else if (FoldableMonitor *ob = dynamic_cast<FoldableMonitor *>(me)) {
	if (TObject *tobj = ob->getTagObject()) {
	  validME = true;
	  //std::cout << "   foldable: " << me->getName() << std::endl;	  
	  object.push_back(tobj);
	  if (dynamic_cast<TObjString *> (tobj)) {
	    //std::cout << "  foldable scalar: " << me->getName() << std::endl;
	    flag |= FLAG_SCALAR;
	  }
	}
      }
      if (!validME) {
	edm::LogError(MsgLoggerCat)
	  << "ERROR: The DQM object '" << fullpath
	  << "' is neither a ROOT object nor a recognised simple object.\n";
	return;
      }
      
      // get quality reports
      // fill with dummy empty report for now
      MEtoROOT::QValue qvalue;
      qvalue.code = 0;
      qvalue.message = "";
      qreportsmap[""] = qvalue;

      //std::cout << "qreports:" << std::endl;
      //MEtoROOT::QReports::iterator qmapIt;
      //for (qmapIt = qreportsmap.begin(); qmapIt != qreportsmap.end(); 
      //	   ++qmapIt) {
      //	std::cout << "   " << qmapIt->first << ":" << std::endl;
      //	std::cout << "      " << "code: " << (qmapIt->second).code
      //		  << std::endl;
      //       std::cout << "      " << "message: " << (qmapIt->second).message
      //		  << std::endl;	
      //      }
      qreports.push_back(qreportsmap);

    } // end get monitor element

    // get flags
    //std::cout << "flag: " << flag << std::endl;
    flags.push_back(flag);

    // get reference
    TObject *tobjref = new TObject();
    //std::cout << "reference:" << std::endl;
    //std::cout << "   " << tobjref->GetName() << std::endl;
    reference.push_back(tobjref);

  } // end loop through all monitor elements

  // produce object to put in events
  std::auto_ptr<MEtoROOT> pOut(new MEtoROOT);

  // fill pOut with vector info
  pOut->putMERootObject(version,name,tags,object,reference,qreports,flags);

  // put into run tree
  iRun.put(pOut,fName);

  return;
}

void MEtoROOTConverter::produce(edm::Event& iEvent, 
				const edm::EventSetup& iSetup)
{
  return;
}
