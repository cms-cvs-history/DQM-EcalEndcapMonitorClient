/*
 * \file testEcalEndcapMonitorIntegrityClient.cpp
 *
 *  $Date: 2007/07/27 15:05:23 $
 *  $Revision: 1.6 $
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
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/Gain/EEIT gain EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/ChId/EEIT ChId EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/TTId/EEIT TTId EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/EEIT DCC size error");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemChId/EEIT MemChId EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemGain/EEIT MemGain EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemTTId/EEIT MemTTId EB+01");
    mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemSize/EEIT MemSize EB+01");

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

      me = mui->get("Collector/FU0/EcalEndcap/EcalInfo/STATUS");
      if ( me ) {
        s = me->valueString();
        status = "unknown";
        if ( s.substr(2,1) == "0" ) status = "start-of-run";
        if ( s.substr(2,1) == "1" ) status = "running";
        if ( s.substr(2,1) == "2" ) status = "end-of-run";
        cout << "status = " << status << endl;
      }

      me = mui->get("Collector/FU0/EcalEndcap/EcalInfo/RUN");
      if ( me ) {
        s = me->valueString();
        run = s.substr(2,s.length()-2);
        cout << "run = " << run << endl;
      }

      me = mui->get("Collector/FU0/EcalEndcap/EcalInfo/EVT");
      if ( me ) {
        s = me->valueString();
        evt = s.substr(2,s.length()-2);
        cout << "event = " << evt << endl;
      }

      me = mui->get("Collector/FU0/EcalEndcap/EcalInfo/RUNTYPE");
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

      TH1F* h1;
      TH2F* h2;

      //      me = mui->get("Collector/FU0/EcalEndcap/EEIntegrityTask/EEIT DCC size error");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/EEIT DCC size error");
      h1 = UtilsClient::getHisto<TH1F*>(me);
      if ( h1 ) {
        c1->cd();
        h1->SetOption("text");
        h1->Draw();
        c1->Update();
      }
    
      //      me = mui->get("Collector/FU0/EcalEndcap/EEIntegrityTask/Gain/EEIT gain EB+01");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/Gain/EEIT gain EB+01");
      h2 = UtilsClient::getHisto<TH2F*>(me);
      if ( h2 ) {
        c2->cd(1);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalEndcap/EEIntegrityTask/ChId/EEIT ChId EB+01");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/ChId/EEIT ChId EB+01");
      h2 = UtilsClient::getHisto<TH2F*>(me);
      if ( h2 ) {
        c2->cd(2);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalEndcap/EEIntegrityTask/TTId/EEIT TTId EB+01");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/TTId/EEIT TTId EB+01");
      h2 = UtilsClient::getHisto<TH2F*>(me);
      if ( h2 ) {
        c2->cd(3);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      //      me = mui->get("Collector/FU0/EcalEndcap/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");
      h2 = UtilsClient::getHisto<TH2F*>(me);
      if ( h2 ) {
        c2->cd(4);
        h2->SetOption("text");
        h2->Draw();
        c2->Update();
      }

      c2->cd();
      c2->Modified();
      c2->Update();

      // me = mui->get("EcalEndcap/Sums/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");
      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/MemChId/EEIT MemChId EB+01");
       h2 = UtilsClient::getHisto<TH2F*>(me);
       if ( h2 ) {
         c3->cd(1);
         h2->SetOption("col");
         h2->Draw();
         c3->Update();
       }

      c3->cd();
      c3->Modified();
      c3->Update();

      me = mui->get("EcalEndcap/Sums/EEIntegrityTask/MemGain/EEIT MemGain EB+01");
      h2 = UtilsClient::getHisto<TH2F*>(me);
      if ( h2 ) {
        c3->cd(2);
        h2->SetOption("col");
        h2->Draw();
        c3->Update();
      }

      c3->cd();
      c3->Modified();
      c3->Update();

      last_plotting = updates;
    }

  }

  exit_done = true;

  return 0;
}




int main(int argc, char** argv) {

  cout << endl;
  cout << " *** Ecal Endcap Monitor Integrity Client ***" << endl;
  cout << endl;

  TApplication app("app", &argc, argv);

  // default client name
  string cfuname = "UserIntegrity";

  // default collector host name
  string hostname = "localhost";

  // default port #
  int port_no = 9090;

  c1 = new TCanvas("Ecal Endcap Integrity Monitoring 1","Ecal Endcap Integrity Monitoring 1",  0, 0,400,400);
  c1->Modified();
  c1->Update();
  c2 = new TCanvas("Ecal Endcap Integrity Monitoring 2","Ecal Endcap Integrity Monitoring 2",430, 0,600,600);
  c2->Divide(2,2);
  c2->Modified();
  c2->Update();
  c3 = new TCanvas("Ecal Endcap Integrity Monitoring 3","Ecal Endcap Integrity Monitoring 3", 430,615,600,370);
  c3->Divide(1,2);
  c3->Modified();
  c3->Update();

  if ( argc >= 2 ) cfuname = argv[1];
  if ( argc >= 3 ) hostname = argv[2];
  if ( argc >= 4 ) port_no = atoi(argv[3]);

  cout << " Client " << cfuname
       << " begins requesting monitoring from host " << hostname << endl;

  // start user interface instance
  mui = new MonitorUIRoot(hostname, port_no, cfuname);

  mui->setVerbose(1);

  // will attempt to reconnect upon connection problems (w/ a 5-sec delay)
  mui->setReconnectDelay(5);

  // subscribe to all monitorable matching pattern
  mui->subscribe("*/EcalEndcap/EcalInfo/STATUS");
  mui->subscribe("*/EcalEndcap/EcalInfo/RUN");
  mui->subscribe("*/EcalEndcap/EcalInfo/EVT");
  mui->subscribe("*/EcalEndcap/EcalInfo/RUNTYPE");
  mui->subscribe("*/EcalEndcap/EEIntegrityTask/Gain/EEIT gain EB+01");
  mui->subscribe("*/EcalEndcap/EEIntegrityTask/ChId/EEIT ChId EB+01");
  mui->subscribe("*/EcalEndcap/EEIntegrityTask/TTId/EEIT TTId EB+01");
  mui->subscribe("*/EcalEndcap/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");
  mui->subscribe("*/EcalEndcap/EEIntegrityTask/EEIT DCC size error");
  mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemChId/EEIT MemChId EB+01");
  mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemGain/EEIT MemGain EB+01");
  mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemTTId/EEIT MemTTId EB+01");
  mui->subscribeNew("*/EcalEndcap/EEIntegrityTask/MemSize/EEIT MemSize EB+01");

  CollateMonitorElement* cme;
  
  cme = mui->collate2D("EEIT gain EB+01", "EEIT gain EB+01", "EcalEndcap/Sums/EEIntegrityTask/Gain");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/Gain/EEIT gain EB+01");

  cme = mui->collate2D("EEIT ChId EB+01", "EEIT ChId EB+01", "EcalEndcap/Sums/EEIntegrityTask/ChId");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/ChId/EEIT ChId EB+01");

  cme = mui->collate2D("EEIT TTId EB+01", "EEIT TTId EB+01", "EcalEndcap/Sums/EEIntegrityTask/TTId");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/TTId/EEIT TTId EB+01");

  cme = mui->collate2D("EEIT TTBlockSize EB+01", "EEIT TTBlockSize EB+01", "EcalEndcap/Sums/EEIntegrityTask/TTBlockSize");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize EB+01");

  cme = mui->collate1D("EEIT DCC size error", "DCC size error", "EcalEndcap/Sums/EEIntegrityTask");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/EEIT DCC size error");

  cme = mui->collate2D("EEIT MemChId EB+01", "EEIT MemChId EB+01", "EcalEndcap/Sums/EEIntegrityTask/MemChId");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/MemChId/EEIT MemChId EB+01");

  cme = mui->collate2D("EEIT MemGain EB+01", "EEIT MemGain EB+01", "EcalEndcap/Sums/EEIntegrityTask/MemGain");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/MemGain/EEIT MemGain EB+01");

  cme = mui->collate2D("EEIT MemTTId EB+01", "EEIT MemTTId EB+01", "EcalEndcap/Sums/EEIntegrityTask/MemTTId");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/MemTTId/EEIT MemTTId EB+01");

  cme = mui->collate2D("EEIT MemSize EB+01", "EEIT MemSize EB+01", "EcalEndcap/Sums/EEIntegrityTask/MemSize");
  mui->add(cme, "*/EcalEndcap/EEIntegrityTask/MemSize/EEIT MemSize EB+01");


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

