/*
 * \file testEcalEndcapMonitorPedestalClient.cpp
 *
 *  $Date: 2007/08/17 09:05:10 $
 *  $Revision: 1.9 $
 *  \author G. Della Ricca
 *
 */

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"

#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

#include "DQM/EcalCommon/interface/UtilsClient.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TThread.h"

#include <iostream>
#include <math.h>

using namespace std;

TCanvas* c1;
TCanvas* c2;
TCanvas* c3;

MonitorUserInterface* mui;

bool exit_now = false;
bool exit_done = false;

void *pth1(void *) {

  bool stay_in_loop = true;

  // last time monitoring objects were plotted
  int last_plotting = -1;

  while ( stay_in_loop && ! exit_now ) {

    // this is the "main" loop where we receive monitoring
    stay_in_loop = mui->update();

    // subscribe to new monitorable matching pattern
    mui->subscribeNew("*/EcalEndcap/EcalInfo/STATUS");
    mui->subscribeNew("*/EcalEndcap/EcalInfo/RUN");
    mui->subscribeNew("*/EcalEndcap/EcalInfo/EVT");
    mui->subscribeNew("*/EcalEndcap/EcalInfo/RUNTYPE");
    mui->subscribeNew("*/EcalEndcap/EEPedestalTask/Gain01/EEPT pedestal EE+01 G01");
    mui->subscribeNew("*/EcalEndcap/EEPedestalTask/Gain06/EEPT pedestal EE+01 G06");
    mui->subscribeNew("*/EcalEndcap/EEPedestalTask/Gain12/EEPT pedestal EE+01 G12");

    // # of full monitoring cycles processed
    int updates = mui->getNumUpdates();

    MonitorElement* me;

    string s;
    string status;
    string run;
    string evt;
    string type;

    // draw monitoring objects every monitoring cycle
    if ( updates != last_plotting ) {

      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EcalInfo/STATUS");
      if ( me ) {
        s = me->valueString();
        status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
      }

      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EcalInfo/RUN");
      if ( me ) {
        s = me->valueString();
        run = s.substr(2,s.length()-2);
        cout << "run = " << run << endl;
      }

      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EcalInfo/EVT");
      if ( me ) {
        s = me->valueString();
        evt = s.substr(2,s.length()-2);
        cout << "event = " << evt << endl;
      }

      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EcalInfo/RUNTYPE");
      if ( me ) {
        s = me->valueString();
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::COSMIC ) type = "COSMIC";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::LASER_STD ) type = "LASER";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::PEDESTAL_STD ) type = "PEDESTAL";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::TESTPULSE_MGPA ) type = "TEST_PULSE";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::BEAMH4 ) type = "BEAMH4";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::BEAMH2 ) type = "BEAMH2";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::MTCC ) type = "PHYSICS";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::COSMICS_LOCAL ) type = "COSMIC";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::COSMICS_GLOBAL ) type = "COSMIC";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::LASER_GAP ) type = "LASER";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::PEDESTAL_GAP ) type = "PEDESTAL";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::TESTPULSE_GAP ) type = "TEST_PULSE";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::PHYSICS_LOCAL ) type = "PHYSICS";
        if ( atoi(s.substr(2,s.size()-2).c_str()) == EcalDCCHeaderBlock::COSMICS_GLOBAL ) type = "PHYSICS";
        cout << "type = " << type << endl;
      }

      TProfile2D* h;

//      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EEPedestalTask/Gain01/EEPT pedestal EE+01 G01");
      me = mui->getBEInterface()->get("EcalEndcap/Sums/EEPedestalTask/Gain01/EEPT pedestal EE+01 G01");
      h = UtilsClient::getHisto<TProfile2D*>(me);
      if ( h ) {
        c1->cd();
        h->SetOption("col");
        h->Draw();
        c1->Update();
      }

//      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EEPedestalTask/Gain06/EEPT pedestal EE+01 G06");
      me = mui->getBEInterface()->get("EcalEndcap/Sums/EEPedestalTask/Gain06/EEPT pedestal EE+01 G06");
      h = UtilsClient::getHisto<TProfile2D*>(me);
      if ( h ) {
        c2->cd();
        h->SetOption("col");
        h->Draw();
        c2->Update();
      }

//      me = mui->getBEInterface()->get("Collector/Ecal/EcalEndcap/EEPedestalTask/Gain12/EEPT pedestal EE+01 G12");
      me = mui->getBEInterface()->get("EcalEndcap/Sums/EEPedestalTask/Gain12/EEPT pedestal EE+01 G12");
      h = UtilsClient::getHisto<TProfile2D*>(me);
      if ( h ) {
        c3->cd();
        h->SetOption("col");
        h->Draw();
        c3->Update();
      }

      last_plotting = updates;
    }

  }

  exit_done = true;

  return 0;
}

int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Endcap Pedestal Monitor Client ***" << endl;
  cout << endl;

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserPedestal";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Endcap Pedestal Monitoring G01","Ecal Endcap Pedestal Monitoring G01", 0,  0,800,250);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Endcap Pedestal Monitoring G06","Ecal Endcap Pedestal Monitoring G06", 0,310,800,250);
  c2->Modified();
  c2->Update();
  c3 = new TCanvas("Ecal Endcap Pedestal Monitoring G12","Ecal Endcap Pedestal Monitoring G12", 0,620,800,250);
  c3->Modified();
  c3->Update();

  if ( argc >= 2 ) cfuname = argv[1];
  if ( argc >= 3 ) hostname = argv[2];
  if ( argc >= 4 ) port_no = atoi(argv[3]);

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname, port_no, cfuname);

  mui->getBEInterface()->setVerbose(1);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("*/EcalEndcap/EcalInfo/STATUS");
  mui->subscribe("*/EcalEndcap/EcalInfo/RUN");
  mui->subscribe("*/EcalEndcap/EcalInfo/EVT");
  mui->subscribe("*/EcalEndcap/EcalInfo/RUNTYPE");
  mui->subscribe("*/EcalEndcap/EEPedestalTask/Gain01/EEPT pedestal EE+01 G01");
  mui->subscribe("*/EcalEndcap/EEPedestalTask/Gain06/EEPT pedestal EE+01 G06");
  mui->subscribe("*/EcalEndcap/EEPedestalTask/Gain12/EEPT pedestal EE+01 G12");

  CollateMonitorElement* cme;

  cme = mui->collateProf2D("EEPT pedestal EE+01 G01", "EEPT pedestal EE+01 G01", "EcalEndcap/Sums/EEPedestalTask/Gain01");
  mui->add(cme, "*/EcalEndcap/EEPedestalTask/Gain01/EEPT pedestal EE+01 G01");

  cme = mui->collateProf2D("EEPT pedestal EE+01 G06", "EEPT pedestal EE+01 G06", "EcalEndcap/Sums/EEPedestalTask/Gain06");
  mui->add(cme, "*/EcalEndcap/EEPedestalTask/Gain06/EEPT pedestal EE+01 G06");

  cme = mui->collateProf2D("EEPT pedestal EE+01 G12", "EEPT pedestal EE+01 G12", "EcalEndcap/Sums/EEPedestalTask/Gain12");
  mui->add(cme, "*/EcalEndcap/EEPedestalTask/Gain12/EEPT pedestal EE+01 G12");

  TThread *th1 = new TThread("th1",pth1);

  th1->Run();

  try { app.Run(kTRUE); } catch (...) { throw; }

  mui->unsubscribe("*");

  exit_now = true;

  while ( ! exit_done ) { usleep(100); }

  th1->Delete();

  delete mui;

  return 0;

}

