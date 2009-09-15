/** \file EDMtoMEConverter.cc
 *
 *  See header file for description of class
 *
 *  $Date: 2009/09/06 12:07:14 $
 *  $Revision: 1.21.2.1 $
 *  \author M. Strang SUNY-Buffalo
 */

#include "DQMServices/Components/plugins/EDMtoMEConverter.h"

EDMtoMEConverter::EDMtoMEConverter(const edm::ParameterSet & iPSet) :
  verbosity(0), frequency(0)
{
  std::string MsgLoggerCat = "EDMtoMEConverter_EDMtoMEConverter";

  // get information from parameter set
  name = iPSet.getUntrackedParameter<std::string>("Name");
  verbosity = iPSet.getUntrackedParameter<int>("Verbosity");
  frequency = iPSet.getUntrackedParameter<int>("Frequency");

  convertOnEndLumi = iPSet.getUntrackedParameter<bool>("convertOnEndLumi",false);
  convertOnEndRun = iPSet.getUntrackedParameter<bool>("convertOnEndRun",false);

  prescaleFactor = iPSet.getUntrackedParameter<int>("prescaleFactor", 1);

  // reset the release tag
  releaseTag = false;

  // use value of first digit to determine default output level (inclusive)
  // 0 is none, 1 is basic, 2 is fill output, 3 is gather output
  verbosity %= 10;

  // get dqm info
  dbe = 0;
  dbe = edm::Service<DQMStore>().operator->();

  // print out Parameter Set information being used
  if (verbosity >= 0) {
    edm::LogInfo(MsgLoggerCat)
      << "\n===============================\n"
      << "Initialized as EDAnalyzer with parameter values:\n"
      << "    Name          = " << name << "\n"
      << "    Verbosity     = " << verbosity << "\n"
      << "    Frequency     = " << frequency << "\n"
      << "===============================\n";
  }

  classtypes.clear();
  classtypes.push_back("TH1F");
  classtypes.push_back("TH1S");
  classtypes.push_back("TH1D");
  classtypes.push_back("TH2F");
  classtypes.push_back("TH2S");
  classtypes.push_back("TH2D");
  classtypes.push_back("TH3F");
  classtypes.push_back("TProfile");
  classtypes.push_back("TProfile2D");
  classtypes.push_back("Float");
  classtypes.push_back("Int");
  classtypes.push_back("String");

  count.clear();
  countf = 0;

} // end constructor

EDMtoMEConverter::~EDMtoMEConverter() {}

void EDMtoMEConverter::beginJob()
{
}

void EDMtoMEConverter::endJob()
{
  std::string MsgLoggerCat = "EDMtoMEConverter_endJob";
  if (verbosity >= 0)
    edm::LogInfo(MsgLoggerCat)
      << "Terminating having processed " << count.size() << " runs across "
      << countf << " files.";
  return;
}

void EDMtoMEConverter::respondToOpenInputFile(const edm::FileBlock& iFb)
{
  ++countf;

  return;
}

void EDMtoMEConverter::beginRun(const edm::Run& iRun,
                                const edm::EventSetup& iSetup)
{
  std::string MsgLoggerCat = "EDMtoMEConverter_beginRun";

  int nrun = iRun.run();

  // keep track of number of unique runs processed
  ++count[nrun];

  if (verbosity) {
    edm::LogInfo(MsgLoggerCat)
      << "Processing run " << nrun << " (" << count.size() << " runs total)";
  } else if (verbosity == 0) {
    if (nrun%frequency == 0 || count.size() == 1) {
      edm::LogInfo(MsgLoggerCat)
        << "Processing run " << nrun << " (" << count.size() << " runs total)";
    }
  }

}

void EDMtoMEConverter::endRun(const edm::Run& iRun,
                              const edm::EventSetup& iSetup)
{
  if (convertOnEndRun) {
    convert(iRun, true);
  }
  return;
}

void EDMtoMEConverter::beginLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                            const edm::EventSetup& iSetup)
{
  return;
}

void EDMtoMEConverter::endLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                          const edm::EventSetup& iSetup)
{
  if (convertOnEndLumi) {
    if (prescaleFactor > 0 &&
        iLumi.id().luminosityBlock() % prescaleFactor == 0) {
      const edm::Run& iRun = iLumi.getRun();
      convert(iRun, false);
    }
  }
  return;
}

void EDMtoMEConverter::analyze(const edm::Event& iEvent,
                               const edm::EventSetup& iSetup)
{
  return;
}

void EDMtoMEConverter::convert(const edm::Run& iRun, const bool endrun)
{
  std::string MsgLoggerCat = "EDMtoMEConverter::convert";

  for (unsigned int ii = 0; ii < classtypes.size(); ++ii) 
  {
    if (classtypes[ii] == "TH1F") 
    {
      if (verbosity >= 0)
          edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH1F MonitorElements.";

      edm::Handle<MEtoEDM<TH1F> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH1F> doesn't exist in run";
        continue;
      }
      std::vector<MEtoEDM<TH1F>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth1f.resize(metoedmobject.size());
      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth1f[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth1f[i] = dbe->book1D(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth1f[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH1F creation

//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH1S") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH1S MonitorElements.";

      edm::Handle<MEtoEDM<TH1S> > metoedm;
      iRun.getByType(metoedm);

      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH1S> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH1S>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth1s.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth1s[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth1s[i] = dbe->book1S(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth1s[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH1S creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH1D") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH1D MonitorElements.";

      edm::Handle<MEtoEDM<TH1D> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH1D> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH1D>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth1d.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth1d[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth1d[i] = dbe->book1DD(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth1d[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH1D creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH2F") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH2F MonitorElements.";

      edm::Handle<MEtoEDM<TH2F> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH2F> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH2F>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth2f.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth2f[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth2f[i] = dbe->book2D(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth2f[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH2F creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH2S") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH2S MonitorElements.";

      edm::Handle<MEtoEDM<TH2S> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH2S> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH2S>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth2s.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth2s[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth2s[i] = dbe->book2S(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth2s[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH2S creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH2D") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH2D MonitorElements.";

      edm::Handle<MEtoEDM<TH2D> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH2D> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH2D>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth2d.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth2d[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth2d[i] = dbe->book2DD(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth2d[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH2D creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TH3F") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TH3F MonitorElements.";

      edm::Handle<MEtoEDM<TH3F> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        edm::LogWarning(MsgLoggerCat) << "MEtoEDM<TH3F> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TH3F>::MEtoEDMObject> metoedmobject =
                                                 metoedm->getMEtoEdmObject();
      meth3f.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meth3f[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;
        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd(); 
	  dbe->bookString( "ReleaseTag",metoedmobject[i].
                           release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }
        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          meth3f[i] = dbe->book3D(metoedmobject[i].object.GetName(),
                               &metoedmobject[i].object);
        } // end define new monitor elements
        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meth3f[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TH3F creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "TProfile") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TProfile MonitorElements.";

      edm::Handle<MEtoEDM<TProfile> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        //edm::LogWarning(MsgLoggerCat)
        //  << "MEtoEDM<TProfile> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TProfile>::MEtoEDMObject> metoedmobject =
                                                    metoedm->getMEtoEdmObject();
      metprof.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        metprof[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;

        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd();
          dbe->bookString("ReleaseTag",metoedmobject[i].
                      release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;

        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }

        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          metprof[i] = dbe->bookProfile(metoedmobject[i].object.GetName(),
                                    &metoedmobject[i].object);
        } // end define new monitor elements

        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(metprof[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TProfile creation
//---------------------------------------------------------------------------

    if (classtypes[ii] == "TProfile2D") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving TProfile2D MonitorElements.";

      edm::Handle<MEtoEDM<TProfile2D> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        //edm::LogWarning(MsgLoggerCat)
        //  << "MEtoEDM<TProfile2D> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TProfile2D>::MEtoEDMObject> metoedmobject =
                                                  metoedm->getMEtoEdmObject();

      metprof2d.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        metprof2d[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;

        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd();
          dbe->bookString("ReleaseTag",metoedmobject[i].
                          release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }

        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          metprof2d[i] = dbe->bookProfile2D(metoedmobject[i].object.GetName(),
                                      &metoedmobject[i].object);
        } // end define new monitor elements

        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(metprof2d[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end TProfile2D creation
//---------------------------------------------------------------------------
    if (classtypes[ii] == "Float") {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving Float MonitorElements.";

      edm::Handle<MEtoEDM<double> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) {
        //edm::LogWarning(MsgLoggerCat)
        //  << "MEtoEDM<double> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<double>::MEtoEDMObject> metoedmobject =
        metoedm->getMEtoEdmObject();

      mefloat.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) {

        mefloat[i] = 0;

        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;

        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd();
          dbe->bookString("ReleaseTag",metoedmobject[i].
                       release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        std::string name;

        // deconstruct path from fullpath

        StringList fulldir = StringOps::split(pathname,"/");
        name = *(fulldir.end() - 1);

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) {
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }

        // define new monitor element
        if (dbe) {
          dbe->setCurrentFolder(dir);
          mefloat[i] = dbe->bookFloat(name);
          mefloat[i]->Fill(metoedmobject[i].object);
        } // end define new monitor elements

        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) {
          dbe->tag(mefloat[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end Float creation

    if (classtypes[ii] == "Int") {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving Int MonitorElements.";

      edm::Handle<MEtoEDM<int> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) {
        //edm::LogWarning(MsgLoggerCat)
        //  << "MEtoEDM<int> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<int>::MEtoEDMObject> metoedmobject =
        metoedm->getMEtoEdmObject();

      meint.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        meint[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;

        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd();
          dbe->bookString("ReleaseTag",metoedmobject[i].
                        release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        std::string name;

        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");
        name = *(fulldir.end() - 1);

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }

        // define new monitor element
        if (dbe) {
          dbe->setCurrentFolder(dir);
          int ival = 0;
          if ( endrun ) {
            if (name.find("processedEvents") != std::string::npos) {
              if (MonitorElement* me = dbe->get(dir+"/"+name)) {
                ival = me->getIntValue();
              }
            }
          }
          meint[i] = dbe->bookInt(name);
          meint[i]->Fill(metoedmobject[i].object+ival);
        } // end define new monitor elements

        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(meint[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end Int creation

    if (classtypes[ii] == "String") 
    {
      if (verbosity >= 0)
        edm::LogInfo (MsgLoggerCat) << "\nRetrieving String MonitorElements.";

      edm::Handle<MEtoEDM<TString> > metoedm;
      iRun.getByType(metoedm);
      if (!metoedm.isValid()) 
      {
        //edm::LogWarning(MsgLoggerCat)
        //  << "MEtoEDM<TString> doesn't exist in run";
        continue;
      }

      std::vector<MEtoEDM<TString>::MEtoEDMObject> metoedmobject =
        metoedm->getMEtoEdmObject();

      mestring.resize(metoedmobject.size());

      for (unsigned int i = 0; i < metoedmobject.size(); ++i) 
      {
        mestring[i] = 0;
        // get full path of monitor element
        std::string pathname = metoedmobject[i].name;
        if (verbosity) std::cout << pathname << std::endl;

        // set the release tag if it has not be yet done
        if (!releaseTag)
        {
          dbe->cd();
          dbe->bookString("ReleaseTag",metoedmobject[i].
                         release.substr(1,metoedmobject[i].release.size()-2));
          releaseTag = true;
        }

        std::string dir;
        std::string name;

        // deconstruct path from fullpath
        StringList fulldir = StringOps::split(pathname,"/");
        name = *(fulldir.end() - 1);

        for (unsigned j = 0; j < fulldir.size() - 1; ++j) 
	{
          dir += fulldir[j];
          if (j != fulldir.size() - 2) dir += "/";
        }

        // define new monitor element
        if (dbe) 
	{
          dbe->setCurrentFolder(dir);
          std::string scont = metoedmobject[i].object.Data();
          mestring[i] = dbe->bookString(name,scont);
        } // end define new monitor elements

        // attach taglist
        TagList tags = metoedmobject[i].tags;

        for (unsigned int j = 0; j < tags.size(); ++j) 
	{
          dbe->tag(mestring[i]->getFullname(),tags[j]);
        }
      } // end loop thorugh metoedmobject
    } // end String creation
  }

  // verify tags stored properly
  if (verbosity) {
    std::vector<std::string> stags;
    dbe->getAllTags(stags);
    for (unsigned int i = 0; i < stags.size(); ++i) {
      std::cout << "Tags: " << stags[i] << std::endl;
    }
  }

  return;
}

