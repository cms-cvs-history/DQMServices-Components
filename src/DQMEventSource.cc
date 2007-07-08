/** \file 
 *
 *  $Date: 2007/06/24 14:25:10 $
 *  $Revision: 1.7 $
 *  \author S. Bolognesi - M. Zanetti
 */

#include "DQMServices/Components/interface/DQMEventSource.h"
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include <DQMServices/UI/interface/MonitorUIRoot.h>
#include "DQMServices/ClientConfig/interface/SubscriptionHandle.h"
#include "DQMServices/ClientConfig/interface/QTestHandle.h"
#include <DQMServices/Core/interface/MonitorElementBaseT.h>

#include <iostream>
#include <string>
#include <sys/time.h>

using namespace edm;
using namespace std;


DQMEventSource::DQMEventSource(const ParameterSet& pset, 
			       const InputSourceDescription& desc) 
  : RawInputSource(pset,desc), updatesCounter(0){

  // hold off to the monitor interface 
  mui = new MonitorUIRoot(pset.getUntrackedParameter<string>("server", "localhost"), 
			  pset.getUntrackedParameter<int>("port", 9090), 
			  pset.getUntrackedParameter<string>("name", "DTDQMClient"), 
			  pset.getUntrackedParameter<int>("reconnect_delay_secs", 5), 
			  pset.getUntrackedParameter<bool>("actAsServer", true));

  bei = mui->getBEInterface();

  subscriber=new SubscriptionHandle;
  qtHandler=new QTestHandle;

  getMESubscriptionListFromFile = pset.getUntrackedParameter<bool>("getMESubscriptionListFromFile", true);
  getQualityTestsFromFile = pset.getUntrackedParameter<bool>("getQualityTestsFromFile", false);
  skipUpdates = pset.getUntrackedParameter<int>("numberOfUpdatesToBeSkipped", 1);

  // subscribe to MEs ty tests
  if (getMESubscriptionListFromFile)
  subscriber->getMEList(pset.getUntrackedParameter<string>("meSubscriptionList", "MESubscriptionList.xml")); 
  // configure quality tests (default false, as this should usually be done in a separate module and NOT in the source)
  if (getQualityTestsFromFile)
    qtHandler->configureTests(pset.getUntrackedParameter<string>("qtList", "QualityTests.xml"),bei);

  iRunMEName = pset.getUntrackedParameter<string>("iRunMEName", "Collector/FU0/EventInfo/iRun");
  iEventMEName = pset.getUntrackedParameter<string>("iEventMEName", "Collector/FU0/EventInfo/iEvent");
  timeStampMEName = pset.getUntrackedParameter<string>("timeStampMEName", "Collector/FU0/EventInfo/timeStamp");


}


std::auto_ptr<Event> DQMEventSource::readOneEvent() {

  // the "onUpdate" call. 
  mui->doMonitoring();

  if (getMESubscriptionListFromFile) subscriber->makeSubscriptions(mui);

  if (getQualityTestsFromFile) qtHandler->attachTests(bei);
  
  // getting the run coordinates 
  RunNumber_t iRun = 0;
  MonitorElementInt * iRun_p = dynamic_cast<MonitorElementInt*>(bei->get(iRunMEName));
  if (iRun_p) iRun = iRun_p->getValue(); 
  setRunNumber(iRun); // <<=== here is where the run is set

  EventNumber_t iEvent = 0;
  MonitorElementInt * iEvent_p = dynamic_cast<MonitorElementInt*>(bei->get(iEventMEName));
  if (iEvent_p) iEvent = iEvent_p->getValue(); 
  else iEvent=updatesCounter; // if the event is not received set number of updates as eventId 

  TimeValue_t tStamp = 0;
  MonitorElementInt * tStamp_p = dynamic_cast<MonitorElementInt*>(bei->get(timeStampMEName));
  if (tStamp_p) tStamp = tStamp_p->getValue(); 
  else tStamp = 1;
  
  EventID eventId(iRun,iEvent);
  Timestamp timeStamp (tStamp);

  // make a fake event containing no data but the evId and runId from DQM sources
  std::auto_ptr<Event> e = makeEvent(eventId,timeStamp);
  
  // run the quality tests: skip the first when the ME created by the Clients are not yet there
  if (updatesCounter > skipUpdates && getQualityTestsFromFile) {
    cout<<"[DQMEventSource]: Running the quality tests"<<endl;
    bei->runQTests();
  }

  // counting the updates
  updatesCounter++;

  return e;
}
