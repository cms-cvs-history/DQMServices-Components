#include "DQMServices/Components/interface/DQMBaseClient.h"
#include "DQMServices/Components/interface/Updater.h"
#include "DQMServices/Components/interface/UpdateObserver.h"
#include "DQMServices/Core/interface/MonitorUserInterface.h"

#include <vector>
#include <string>
#include <iostream>

class ExampleClient : public DQMBaseClient, public dqm::UpdateObserver
{
public:
  XDAQ_INSTANTIATOR();
  ExampleClient(xdaq::ApplicationStub *s) : DQMBaseClient(s,"test")
  {}
  void configure(){}
  void newRun(){upd_->registerObserver(this);}
  void endRun(){}
  void onUpdate() const
  {
    std::vector<std::string> uplist;
    mui_->getUpdatedContents(uplist);
    std::cout << "updated objects" << std::endl;
    for(unsigned i=0; i<uplist.size(); i++)
      std::cout << uplist[i] << std::endl;
  }
};

XDAQ_INSTANTIATOR_IMPL(ExampleClient)
