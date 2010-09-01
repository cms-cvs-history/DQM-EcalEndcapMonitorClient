
/*
 * \file EEIntegrityClient.cc
 *
 * $Date: 2010/08/04 19:05:29 $
 * $Revision: 1.106.4.1 $
 * \author G. Della Ricca
 * \author G. Franzoni
 *
 */

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/DQMStore.h"

#ifdef WITH_ECAL_COND_DB
#include "OnlineDB/EcalCondDB/interface/RunTag.h"
#include "OnlineDB/EcalCondDB/interface/RunIOV.h"
#include "OnlineDB/EcalCondDB/interface/MonCrystalConsistencyDat.h"
#include "OnlineDB/EcalCondDB/interface/MonTTConsistencyDat.h"
#include "OnlineDB/EcalCondDB/interface/MonMemChConsistencyDat.h"
#include "OnlineDB/EcalCondDB/interface/MonMemTTConsistencyDat.h"
#include "OnlineDB/EcalCondDB/interface/RunCrystalErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunTTErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunPNErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunMemChErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/RunMemTTErrorsDat.h"
#include "OnlineDB/EcalCondDB/interface/EcalCondDBInterface.h"
#include "CondTools/Ecal/interface/EcalErrorDictionary.h"
#include "DQM/EcalCommon/interface/EcalErrorMask.h"
#include "DQM/EcalCommon/interface/LogicID.h"
#endif

#include "DQM/EcalCommon/interface/UtilsClient.h"
#include "DQM/EcalCommon/interface/Numbers.h"

#include "DataFormats/EcalDetId/interface/EEDetId.h"

#include "DQM/EcalEndcapMonitorClient/interface/EEIntegrityClient.h"

EEIntegrityClient::EEIntegrityClient(const edm::ParameterSet& ps) {

  // cloneME switch
  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);

  // verbose switch
  verbose_ = ps.getUntrackedParameter<bool>("verbose", true);

  // debug switch
  debug_ = ps.getUntrackedParameter<bool>("debug", false);

  // prefixME path
  prefixME_ = ps.getUntrackedParameter<std::string>("prefixME", "");

  // enableCleanup_ switch
  enableCleanup_ = ps.getUntrackedParameter<bool>("enableCleanup", false);

  // vector of selected Super Modules (Defaults to all 18).
  superModules_.reserve(18);
  for ( unsigned int i = 1; i <= 18; i++ ) superModules_.push_back(i);
  superModules_ = ps.getUntrackedParameter<std::vector<int> >("superModules", superModules_);

  h00_ = 0;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    h_[ism-1] = 0;
    hmem_[ism-1] = 0;

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;
    h04_[ism-1] = 0;
    h05_[ism-1] = 0;
    h06_[ism-1] = 0;
    h07_[ism-1] = 0;
    h08_[ism-1] = 0;
    h09_[ism-1] = 0;

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    // integrity summary histograms
    meg01_[ism-1] = 0;
    meg02_[ism-1] = 0;

  }

  threshCry_ = 0.01;

}

EEIntegrityClient::~EEIntegrityClient() {

}

void EEIntegrityClient::beginJob(void) {

  dqmStore_ = edm::Service<DQMStore>().operator->();

  if ( debug_ ) std::cout << "EEIntegrityClient: beginJob" << std::endl;

  ievt_ = 0;
  jevt_ = 0;

}

void EEIntegrityClient::beginRun(void) {

  if ( debug_ ) std::cout << "EEIntegrityClient: beginRun" << std::endl;

  jevt_ = 0;

  this->setup();

}

void EEIntegrityClient::endJob(void) {

  if ( debug_ ) std::cout << "EEIntegrityClient: endJob, ievt = " << ievt_ << std::endl;

  this->cleanup();

}

void EEIntegrityClient::endRun(void) {

  if ( debug_ ) std::cout << "EEIntegrityClient: endRun, jevt = " << jevt_ << std::endl;

  this->cleanup();

}

void EEIntegrityClient::setup(void) {

  char histo[200];

  dqmStore_->setCurrentFolder( prefixME_ + "/EEIntegrityClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) dqmStore_->removeElement( meg01_[ism-1]->getName() );
    sprintf(histo, "EEIT data integrity quality %s", Numbers::sEE(ism).c_str());
    meg01_[ism-1] = dqmStore_->book2D(histo, histo, 50, Numbers::ix0EE(ism)+0., Numbers::ix0EE(ism)+50., 50, Numbers::iy0EE(ism)+0., Numbers::iy0EE(ism)+50.);
    meg01_[ism-1]->setAxisTitle("ix", 1);
    if ( ism >= 1 && ism <= 9 ) meg01_[ism-1]->setAxisTitle("101-ix", 1);
    meg01_[ism-1]->setAxisTitle("iy", 2);

    if ( meg02_[ism-1] ) dqmStore_->removeElement( meg02_[ism-1]->getName() );
    sprintf(histo, "EEIT data integrity quality MEM %s", Numbers::sEE(ism).c_str());
    meg02_[ism-1] = dqmStore_->book2D(histo, histo, 10, 0., 10., 5, 0.,5.);
    meg02_[ism-1]->setAxisTitle("pseudo-strip", 1);
    meg02_[ism-1]->setAxisTitle("channel", 2);

  }

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) meg01_[ism-1]->Reset();
    if ( meg02_[ism-1] ) meg02_[ism-1]->Reset();

    for ( int ix = 1; ix <= 50; ix++ ) {
      for ( int iy = 1; iy <= 50; iy++ ) {

        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent( ix, iy, 6. );

        int jx = ix + Numbers::ix0EE(ism);
        int jy = iy + Numbers::iy0EE(ism);

        if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;

        if ( Numbers::validEE(ism, jx, jy) ) {
          if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent( ix, iy, 2. );
        }

      }
    }

    for ( int ie = 1; ie <= 10; ie++ ) {
      for ( int ip = 1; ip <= 5; ip++ ) {

        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent( ie, ip, 6. );

        // non-existing mem
        if ( (ism >=  3 && ism <=  4) || (ism >=  7 && ism <=  9) ) continue;
        if ( (ism >= 12 && ism <=  3) || (ism >= 16 && ism <= 18) ) continue;

        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent( ie, ip, 2. );

      }
    }

  }

}

void EEIntegrityClient::cleanup(void) {

  if ( ! enableCleanup_ ) return;

  if ( cloneME_ ) {
    if ( h00_ ) delete h00_;
  }

  h00_ = 0;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( cloneME_ ) {
      if ( h_[ism-1] )    delete h_[ism-1];
      if ( hmem_[ism-1] ) delete hmem_[ism-1];

      if ( h01_[ism-1] ) delete h01_[ism-1];
      if ( h02_[ism-1] ) delete h02_[ism-1];
      if ( h03_[ism-1] ) delete h03_[ism-1];
      if ( h04_[ism-1] ) delete h04_[ism-1];
      if ( h05_[ism-1] ) delete h05_[ism-1];
      if ( h06_[ism-1] ) delete h06_[ism-1];
      if ( h07_[ism-1] ) delete h07_[ism-1];
      if ( h08_[ism-1] ) delete h08_[ism-1];
      if ( h09_[ism-1] ) delete h09_[ism-1];
    }

    h_[ism-1] = 0;
    hmem_[ism-1] = 0;

    h01_[ism-1] = 0;
    h02_[ism-1] = 0;
    h03_[ism-1] = 0;
    h04_[ism-1] = 0;
    h05_[ism-1] = 0;
    h06_[ism-1] = 0;
    h07_[ism-1] = 0;
    h08_[ism-1] = 0;
    h09_[ism-1] = 0;

  }

  dqmStore_->setCurrentFolder( prefixME_ + "/EEIntegrityClient" );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( meg01_[ism-1] ) dqmStore_->removeElement( meg01_[ism-1]->getName() );
    meg01_[ism-1] = 0;

    if ( meg02_[ism-1] ) dqmStore_->removeElement( meg02_[ism-1]->getName() );
    meg02_[ism-1] = 0;

  }

}

#ifdef WITH_ECAL_COND_DB
bool EEIntegrityClient::writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, bool& status) {

  status = true;

  EcalLogicID ecid;

  MonCrystalConsistencyDat c1;
  map<EcalLogicID, MonCrystalConsistencyDat> dataset1;
  MonTTConsistencyDat c2;
  map<EcalLogicID, MonTTConsistencyDat> dataset2;
  MonMemChConsistencyDat c3;
  map<EcalLogicID, MonMemChConsistencyDat> dataset3;
  MonMemTTConsistencyDat c4;
  map<EcalLogicID, MonMemTTConsistencyDat> dataset4;

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    if ( h00_ && h00_->GetBinContent(ism) != 0 ) {
      cerr << std::endl;
      cerr << " DCC failed " << h00_->GetBinContent(ism) << " times" << std::endl;
      cerr << std::endl;
    }

    if ( verbose_ ) {
      std::cout << " " << Numbers::sEE(ism) << " (ism=" << ism << ")" << std::endl;
      std::cout << std::endl;
      UtilsClient::printBadChannels(meg01_[ism-1], h01_[ism-1], true);
      UtilsClient::printBadChannels(meg01_[ism-1], h02_[ism-1], true);
      UtilsClient::printBadChannels(meg01_[ism-1], h03_[ism-1], true);
      UtilsClient::printBadChannels(meg01_[ism-1], h04_[ism-1], true);
      UtilsClient::printBadChannels(meg01_[ism-1], h05_[ism-1], true);

      UtilsClient::printBadChannels(meg02_[ism-1], h06_[ism-1], true);
      UtilsClient::printBadChannels(meg02_[ism-1], h07_[ism-1], true);
      UtilsClient::printBadChannels(meg02_[ism-1], h08_[ism-1], true);
      UtilsClient::printBadChannels(meg02_[ism-1], h09_[ism-1], true);
    }

    float num00;

    num00 = 0.;

    bool update0 = false;

    if ( h00_ ) {
      num00 = h00_->GetBinContent(ism);
      if ( num00 > 0 ) update0 = true;
    }

    float num01, num02, num03;

    for ( int ix = 1; ix <= 50; ix++ ) {
      for ( int iy = 1; iy <= 50; iy++ ) {

        int jx = ix + Numbers::ix0EE(ism);
        int jy = iy + Numbers::iy0EE(ism);

        if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;

        if ( ! Numbers::validEE(ism, jx, jy) ) continue;

        num01 = num02 = num03 = 0.;

        bool update1 = false;

        float numTot = -1.;

        if ( h_[ism-1] ) numTot = h_[ism-1]->GetBinContent(ix, iy);

        if ( h01_[ism-1] ) {
          num01  = h01_[ism-1]->GetBinContent(ix, iy);
          if ( num01 > 0 ) update1 = true;
        }

        if ( h02_[ism-1] ) {
          num02  = h02_[ism-1]->GetBinContent(ix, iy);
          if ( num02 > 0 ) update1 = true;
        }

        if ( h03_[ism-1] ) {
          num03  = h03_[ism-1]->GetBinContent(ix, iy);
          if ( num03 > 0 ) update1 = true;
        }

        if ( update0 || update1 ) {

          if ( Numbers::icEE(ism, jx, jy) == 1 ) {

            if ( verbose_ ) {
              std::cout << "Preparing dataset for " << Numbers::sEE(ism) << " (ism=" << ism << ")" << std::endl;
              std::cout << "(" << Numbers::ix0EE(i+1)+ix << "," << Numbers::iy0EE(i+1)+iy << ") " << num00 << " " << num01 << " " << num02 << " " << num03 << std::endl;
              std::cout << std::endl;
            }

          }

          c1.setProcessedEvents(int(numTot));
          c1.setProblematicEvents(int(num01+num02+num03));
          c1.setProblemsGainZero(int(num01));
          c1.setProblemsID(int(num02));
          c1.setProblemsGainSwitch(int(num03));

          bool val;

          val = false;
          if ( numTot > 0 ) {
            float errorRate1 = num00 / ( numTot + num01 + num02 + num03 );
            if ( errorRate1 > threshCry_ )
              val = true;
            errorRate1 = ( num01 + num02 + num03 ) / ( numTot + num01 + num02 + num03 ) / 3.;
            if ( errorRate1 > threshCry_ )
              val = true;
          } else {
            if ( num00 > 0 )
              val = true;
            if ( ( num01 + num02 + num03 ) > 0 )
              val = true;
          }
          c1.setTaskStatus(val);

          int ic = Numbers::indexEE(ism, jx, jy);

          if ( ic == -1 ) continue;

          if ( econn ) {
            ecid = LogicID::getEcalLogicID("EE_crystal_number", Numbers::iSM(ism, EcalEndcap), ic);
            dataset1[ecid] = c1;
          }

          status = status && !val;

        }

      }
    }

    float num04, num05;

    for ( int ixt = 1; ixt <= 10; ixt++ ) {
      for ( int iyt = 1; iyt <= 10; iyt++ ) {

        int jxt = Numbers::ix0EE(ism) + 1 + 5*(ixt-1);
        int jyt = Numbers::iy0EE(ism) + 1 + 5*(iyt-1);

        if ( ism >= 1 && ism <= 9 ) jxt = 101 - jxt;

        num04 = num05 = 0.;

        bool update1 = false;

        float numTot = -1.;

        if ( h_[ism-1] ) {
          numTot = 0.;
          for ( int ix = 1 + 5*(ixt-1); ix <= 5*ixt; ix++ ) {
            for ( int iy = 1 + 5*(iyt-1); iy <= 5*iyt; iy++ ) {
              int jx = ix + Numbers::ix0EE(ism);
              int jy = iy + Numbers::iy0EE(ism);
              if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;
              if ( ! Numbers::validEE(ism, jx, jy) ) continue;
              numTot += h_[ism-1]->GetBinContent(ix, iy);
            }
          }
        }

        if ( h04_[ism-1] ) {
          for ( int ix = 1 + 5*(ixt-1); ix <= 5*ixt; ix++ ) {
            for ( int iy = 1 + 5*(iyt-1); iy <= 5*iyt; iy++ ) {
              int jx = ix + Numbers::ix0EE(ism);
              int jy = iy + Numbers::iy0EE(ism);
              if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;
              if ( ! Numbers::validEE(ism, jx, jy) ) continue;
              num04  = h04_[ism-1]->GetBinContent(ix, iy);
              if ( num04 > 0 ) update1 = true;
            }
          }
        }

        if ( h05_[ism-1] ) {
          for ( int ix = 1 + 5*(ixt-1); ix <= 5*ixt; ix++ ) {
            for ( int iy = 1 + 5*(iyt-1); iy <= 5*iyt; iy++ ) {
              int jx = ix + Numbers::ix0EE(ism);
              int jy = iy + Numbers::iy0EE(ism);
              if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;
              if ( ! Numbers::validEE(ism, jx, jy) ) continue;
              num05  = h05_[ism-1]->GetBinContent(ix, iy);
              if ( num05 > 0 ) update1 = true;
            }
          }
        }

        if ( update0 || update1 ) {

          if ( Numbers::iSC(ism, EcalEndcap, jxt, jyt) == 1 ) {

            if ( verbose_ ) {
              std::cout << "Preparing dataset for " << Numbers::sEE(ism) << " (ism=" << ism << ")" << std::endl;
              std::cout << "(" << 1+(Numbers::ix0EE(ism)+1+5*(ixt-1))/5 << "," << 1+(Numbers::iy0EE(ism)+1+5*(iyt-1))/5 << ") " << num00 << " " << num04 << " " << num05 << std::endl;
              std::cout << std::endl;
            }

          }

          c2.setProcessedEvents(int(numTot));
          c2.setProblematicEvents(int(num04+num05));
          c2.setProblemsID(int(num04));
          c2.setProblemsSize(int(num05));
          c2.setProblemsLV1(int(-1.));
          c2.setProblemsBunchX(int(-1.));

          bool val;

          val = false;
          if ( numTot > 0 ) {
            float errorRate2 = num00 / ( numTot + num04 + num05 );
            if ( errorRate2 > threshCry_ )
              val = true;
            errorRate2 = ( num04 + num05 ) / ( numTot + num04 + num05 ) / 2.;
            if ( errorRate2 > threshCry_ )
              val = true;
          } else {
            if ( num00 > 0 )
              val = true;
            if ( ( num04 + num05 ) > 0 )
              val = true;
          }
          c2.setTaskStatus(val);

          int itt = Numbers::iSC(ism, EcalEndcap, jxt, jyt);

          if ( itt == -1 ) continue;

          if ( econn ) {
            ecid = LogicID::getEcalLogicID("EE_readout_tower", Numbers::iSM(ism, EcalEndcap), itt);
            dataset2[ecid] = c2;
          }

          status = status && !val;

        }

      }
    }

    float num06, num07;

    for ( int ix = 1; ix <= 10; ix++ ) {
      for ( int iy = 1; iy <= 5; iy++ ) {

        num06 = num07 = 0.;

        bool update1 = false;

        float numTot = -1.;

        if ( hmem_[ism-1] ) numTot = hmem_[ism-1]->GetBinContent(ix, iy);

        if ( h06_[ism-1] ) {
          num06  = h06_[ism-1]->GetBinContent(ix, iy);
          if ( num06 > 0 ) update1 = true;
        }

        if ( h07_[ism-1] ) {
          num07  = h07_[ism-1]->GetBinContent(ix, iy);
          if ( num07 > 0 ) update1 = true;
        }

        if ( update0 || update1 ) {

          if ( ix ==1 && iy == 1 ) {

            if ( verbose_ ) {
              std::cout << "Preparing dataset for mem of SM=" << ism << std::endl;
              std::cout << "(" << ix << "," << iy << ") " << num06 << " " << num07 << std::endl;
              std::cout << std::endl;
            }

          }

          c3.setProcessedEvents( int (numTot));
          c3.setProblematicEvents(int (num06+num07));
          c3.setProblemsID(int (num06) );
          c3.setProblemsGainZero(int (num07));
          // c3.setProblemsGainSwitch(int prob);

          bool val;

          val = false;
          if ( numTot > 0 ) {
            float errorRate1 = num00 / ( numTot + num06 + num07 );
            if ( errorRate1 > threshCry_ )
              val = true;
            errorRate1 = ( num06 + num07 ) / ( numTot + num06 + num07 ) / 2.;
            if ( errorRate1 > threshCry_ )
              val = true;
          } else {
            if ( num00 > 0 )
             val = true;
            if ( ( num06 + num07 ) > 0 )
              val = true;
          }
          c3. setTaskStatus(val);

          int ic = EEIntegrityClient::chNum[ (ix-1)%5 ][ (iy-1) ] + (ix-1)/5 * 25;

          if ( econn ) {
            ecid = LogicID::getEcalLogicID("EE_mem_channel", Numbers::iSM(ism, EcalEndcap), ic);
            dataset3[ecid] = c3;
          }

          status = status && !val;

        }

      }
    }

    float num08, num09;

    for ( int ixt = 1; ixt <= 2; ixt++ ) {

      num08 = num09 = 0.;

      bool update1 = false;

      float numTot = -1.;

      if ( hmem_[ism-1] ) {
        numTot = 0.;
        for ( int ix = 1 + 5*(ixt-1); ix <= 5*ixt; ix++ ) {
          for ( int iy = 1 ; iy <= 5; iy++ ) {
            numTot += hmem_[ism-1]->GetBinContent(ix, iy);
          }
        }
      }

      if ( h08_[ism-1] ) {
        num08  = h08_[ism-1]->GetBinContent(ixt, 1);
        if ( num08 > 0 ) update1 = true;
      }

      if ( h09_[ism-1] ) {
        num09  = h09_[ism-1]->GetBinContent(ixt, 1);
        if ( num09 > 0 ) update1 = true;
      }

      if ( update0 || update1 ) {

        if ( ixt == 1 ) {

          if ( verbose_ ) {
            std::cout << "Preparing dataset for " << Numbers::sEE(ism) << " (ism=" << ism << ")" << std::endl;
            std::cout << "(" << ixt <<  ") " << num08 << " " << num09 << std::endl;
            std::cout << std::endl;
          }

        }

        c4.setProcessedEvents( int(numTot) );
        c4.setProblematicEvents( int(num08 + num09) );
        c4.setProblemsID( int(num08) );
        c4.setProblemsSize(int (num09) );
        // setProblemsLV1(int LV1);
        // setProblemsBunchX(int bunchX);

        bool val;

        val = false;
        if ( numTot > 0 ) {
          float errorRate2 = num00 / ( numTot + num08 + num09 );
          if ( errorRate2 > threshCry_ )
            val = true;
          errorRate2 = ( num08 + num09 ) / ( numTot/25. + num08 + num09 ) / 2.;
          if ( errorRate2 > threshCry_ )
            val = true;
        } else {
          if ( num00 > 0 )
            val = true;
          if ( ( num08 + num09 ) > 0 )
            val = true;
        }
        c4.setTaskStatus(val);

        int itt = 68 + ixt;

        if ( econn ) {
          ecid = LogicID::getEcalLogicID("EE_mem_TT", Numbers::iSM(ism, EcalEndcap), itt);
          dataset4[ecid] = c4;
        }

        status = status && !val;

      }

    }

  }

  if ( econn ) {
    try {
      if ( verbose_ ) std::cout << "Inserting MonConsistencyDat ..." << std::endl;
      if ( dataset1.size() != 0 ) econn->insertDataArraySet(&dataset1, moniov);
      if ( dataset2.size() != 0 ) econn->insertDataArraySet(&dataset2, moniov);
      if ( dataset3.size() != 0 ) econn->insertDataArraySet(&dataset3, moniov);
      if ( dataset4.size() != 0 ) econn->insertDataArraySet(&dataset4, moniov);
      if ( verbose_ ) std::cout << "done." << std::endl;
    } catch (runtime_error &e) {
      cerr << e.what() << std::endl;
    }
  }

  return true;

}
#endif

void EEIntegrityClient::analyze(void) {

  ievt_++;
  jevt_++;
  if ( ievt_ % 10 == 0 ) {
    if ( debug_ ) std::cout << "EEIntegrityClient: ievt/jevt = " << ievt_ << "/" << jevt_ << std::endl;
  }

#ifdef WITH_ECAL_COND_DB
  uint64_t bits01 = 0;
  bits01 |= EcalErrorDictionary::getMask("CH_ID_WARNING");
  bits01 |= EcalErrorDictionary::getMask("CH_GAIN_ZERO_WARNING");
  bits01 |= EcalErrorDictionary::getMask("CH_GAIN_SWITCH_WARNING");
  bits01 |= EcalErrorDictionary::getMask("CH_ID_ERROR");
  bits01 |= EcalErrorDictionary::getMask("CH_GAIN_ZERO_ERROR");
  bits01 |= EcalErrorDictionary::getMask("CH_GAIN_SWITCH_ERROR");

  uint64_t bits02 = 0;
  bits02 |= EcalErrorDictionary::getMask("TT_ID_WARNING");
  bits02 |= EcalErrorDictionary::getMask("TT_SIZE_WARNING");
  bits02 |= EcalErrorDictionary::getMask("TT_LV1_WARNING");
  bits02 |= EcalErrorDictionary::getMask("TT_BUNCH_X_WARNING");
  bits02 |= EcalErrorDictionary::getMask("TT_ID_ERROR");
  bits02 |= EcalErrorDictionary::getMask("TT_SIZE_ERROR");
  bits02 |= EcalErrorDictionary::getMask("TT_LV1_ERROR");
  bits02 |= EcalErrorDictionary::getMask("TT_BUNCH_X_ERROR");
#endif

  char histo[200];

  MonitorElement* me;

  sprintf(histo, (prefixME_ + "/EEIntegrityTask/EEIT DCC size error").c_str());
  me = dqmStore_->get(histo);
  h00_ = UtilsClient::getHisto<TH1F*>( me, cloneME_, h00_ );

  for ( unsigned int i=0; i<superModules_.size(); i++ ) {

    int ism = superModules_[i];

    sprintf(histo, (prefixME_ + "/EEOccupancyTask/EEOT digi occupancy %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEOccupancyTask/EEOT MEM digi occupancy %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    hmem_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, hmem_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/Gain/EEIT gain %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h01_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h01_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/ChId/EEIT ChId %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h02_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h02_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/GainSwitch/EEIT gain switch %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h03_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h03_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/TTId/EEIT TTId %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h04_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h04_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/TTBlockSize/EEIT TTBlockSize %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h05_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h05_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/MemChId/EEIT MemChId %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h06_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h06_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/MemGain/EEIT MemGain %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h07_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h07_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/MemTTId/EEIT MemTTId %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h08_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h08_[ism-1] );

    sprintf(histo, (prefixME_ + "/EEIntegrityTask/MemSize/EEIT MemSize %s").c_str(), Numbers::sEE(ism).c_str());
    me = dqmStore_->get(histo);
    h09_[ism-1] = UtilsClient::getHisto<TH2F*>( me, cloneME_, h09_[ism-1] );

    float num00;

    // integrity summary histograms
    if ( meg01_[ism-1] ) meg01_[ism-1]->Reset();
    if ( meg01_[ism-1] ) meg02_[ism-1]->Reset();

    num00 = 0.;

    bool update0 = false;

    // dcc size errors
    if ( h00_ ) {
      num00  = h00_->GetBinContent(ism);
      update0 = true;
    }

    float num01, num02, num03, num04, num05;

    for ( int ix = 1; ix <= 50; ix++ ) {
      for ( int iy = 1; iy <= 50; iy++ ) {

        num01 = num02 = num03 = num04 = num05 = 0.;

        if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent( ix, iy, 6. );

        bool update1 = false;
        bool update2 = false;

        float numTot = -1.;

        if ( h_[ism-1] ) numTot = h_[ism-1]->GetBinContent(ix, iy);

        if ( h01_[ism-1] ) {
          num01  = h01_[ism-1]->GetBinContent(ix, iy);
          update1 = true;
        }

        if ( h02_[ism-1] ) {
          num02  = h02_[ism-1]->GetBinContent(ix, iy);
          update1 = true;
        }

        if ( h03_[ism-1] ) {
          num03  = h03_[ism-1]->GetBinContent(ix, iy);
          update1 = true;
        }

        if ( h04_[ism-1] ) {
          num04  = h04_[ism-1]->GetBinContent(ix, iy);
          update2 = true;
        }

        if ( h05_[ism-1] ) {
          num05  = h05_[ism-1]->GetBinContent(ix, iy);
          update2 = true;
        }

        if ( update0 || update1 || update2 ) {

          float val;

          val = 1.;
          // number of events on a channel
          if ( numTot > 0 ) {
            float errorRate1 =  num00 / ( numTot + num01 + num02 + num03 );
            if ( errorRate1 > threshCry_ )
              val = 0.;
            errorRate1 = ( num01 + num02 + num03 ) / ( numTot + num01 + num02 + num03 ) / 3.;
            if ( errorRate1 > threshCry_ )
              val = 0.;
            float errorRate2 = ( num04 + num05 ) / ( numTot + num04 + num05 ) / 2.;
            if ( errorRate2 > threshCry_ )
              val = 0.;
          } else {
            val = 2.;
            if ( num00 > 0 )
              val = 0.;
            if ( ( num01 + num02 + num03 ) > 0 )
              val = 0.;
            if ( ( num04 + num05 ) > 0 )
              val = 0.;
          }

          int jx = ix + Numbers::ix0EE(ism);
          int jy = iy + Numbers::iy0EE(ism);

          if ( ism >= 1 && ism <= 9 ) jx = 101 - jx;

          // filling the summary for SM channels
          if ( Numbers::validEE(ism, jx, jy) ) {
            if ( meg01_[ism-1] ) meg01_[ism-1]->setBinContent( ix, iy, val );
          }

        }

      }
    } // end of loop on crystals to fill summary plot

    // summaries for mem channels
    float num06, num07, num08, num09;

    for ( int ie = 1; ie <= 10; ie++ ) {
      for ( int ip = 1; ip <= 5; ip++ ) {

        num06 = num07 = num08 = num09 = 0.;

        // initialize summary histo for mem
        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent( ie, ip, 6. );

        // non-existing mem
        if ( (ism >=  3 && ism <=  4) || (ism >=  7 && ism <=  9) ) continue;
        if ( (ism >= 12 && ism <= 13) || (ism >= 16 && ism <= 18) ) continue;

        if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent( ie, ip, 2. );

        bool update1 = false;
        bool update2 = false;

        float numTotmem = -1.;

        if ( hmem_[ism-1] ) numTotmem = hmem_[ism-1]->GetBinContent(ie, ip);

        if ( h06_[ism-1] ) {
          num06  = h06_[ism-1]->GetBinContent(ie, ip);
          update1 = true;
        }

        if ( h07_[ism-1] ) {
          num07  = h07_[ism-1]->GetBinContent(ie, ip);
          update1 = true;
        }

        int iet = 1 + ((ie-1)/5);
        int ipt = 1;

        if ( h08_[ism-1] ) {
          num08  = h08_[ism-1]->GetBinContent(iet, ipt);
          update2 = true;
        }

        if ( h09_[ism-1] ) {
          num09  = h09_[ism-1]->GetBinContent(iet, ipt);
          update2 = true;
        }

        if ( update0 || update1 || update2 ) {

          float val;

          val = 1.;
          // number of events on a channel
          if ( numTotmem > 0 ) {
            float errorRate1 = ( num06 + num07 ) / ( numTotmem + num06 + num07 )/ 2.;
            if ( errorRate1 > threshCry_ )
              val = 0.;
            float errorRate2 = ( num08 + num09 ) / ( numTotmem/25. + num08 + num09 ) / 2.;
            if ( errorRate2 > threshCry_ )
              val = 0.;
          } else {
            val = 2.;
            if ( ( num06 + num07 ) > 0 )
              val = 0.;
            if ( ( num08 + num09 ) > 0 )
              val = 0.;
          }

          // filling summary for mem channels
          if ( meg02_[ism-1] ) meg02_[ism-1]->setBinContent( ie, ip, val );

        }

      }
    }  // end loop on mem channels

  } // end loop on supermodules

#ifdef WITH_ECAL_COND_DB
  if ( EcalErrorMask::mapCrystalErrors_.size() != 0 ) {
    map<EcalLogicID, RunCrystalErrorsDat>::const_iterator m;
    for (m = EcalErrorMask::mapCrystalErrors_.begin(); m != EcalErrorMask::mapCrystalErrors_.end(); m++) {

      if ( (m->second).getErrorBits() & bits01 ) {
        EcalLogicID ecid = m->first;

        if ( strcmp(ecid.getMapsTo().c_str(), "EE_crystal_number") != 0 ) continue;

        int iz = ecid.getID1();
        int ix = ecid.getID2();
        int iy = ecid.getID3();

        for ( unsigned int i=0; i<superModules_.size(); i++ ) {
          int ism = superModules_[i];
          std::vector<int>::iterator iter = find(superModules_.begin(), superModules_.end(), ism);
          if (iter == superModules_.end()) continue;
          if ( iz == -1 && ( ism >=  1 && ism <=  9 ) ) {
            int jx = 101 - ix - Numbers::ix0EE(ism);
            int jy = iy - Numbers::iy0EE(ism);
            if ( Numbers::validEE(ism, ix, iy) ) UtilsClient::maskBinContent( meg01_[ism-1], jx, jy );
          }
          if ( iz == +1 && ( ism >= 10 && ism <= 18 ) ) {
            int jx = ix - Numbers::ix0EE(ism);
            int jy = iy - Numbers::iy0EE(ism);
            if ( Numbers::validEE(ism, ix, iy) ) UtilsClient::maskBinContent( meg01_[ism-1], jx, jy );
          }
        }

      }

    }
  }

  if ( EcalErrorMask::mapTTErrors_.size() != 0 ) {
    map<EcalLogicID, RunTTErrorsDat>::const_iterator m;
    for (m = EcalErrorMask::mapTTErrors_.begin(); m != EcalErrorMask::mapTTErrors_.end(); m++) {

      if ( (m->second).getErrorBits() & bits02 ) {
        EcalLogicID ecid = m->first;

        if ( strcmp(ecid.getMapsTo().c_str(), "EE_readout_tower") != 0 ) continue;

        int idcc = ecid.getID1() - 600;

        int ism = -1;
        if ( idcc >=   1 && idcc <=   9 ) ism = idcc;
        if ( idcc >=  46 && idcc <=  54 ) ism = idcc - 45 + 9;
        std::vector<int>::iterator iter = find(superModules_.begin(), superModules_.end(), ism);
        if (iter == superModules_.end()) continue;

        int itt = ecid.getID2();

        if ( itt > 70 ) continue;

        if ( itt >= 42 && itt <= 68 ) continue;

        if ( ( ism == 8 || ism == 17 ) && ( itt >= 18 && itt <= 24 ) ) continue;

        if ( itt >= 1 && itt <= 68 ) {
          vector<DetId>* crystals = Numbers::crystals( idcc, itt );
          for ( unsigned int i=0; i<crystals->size(); i++ ) {
            EEDetId id = (*crystals)[i];
            int ix = id.ix();
            int iy = id.iy();
            if ( ism >= 1 && ism <= 9 ) ix = 101 - ix;
            int jx = ix - Numbers::ix0EE(ism);
            int jy = iy - Numbers::iy0EE(ism);
            UtilsClient::maskBinContent( meg01_[ism-1], jx, jy );
          }
        }

      }

    }
  }

  if ( EcalErrorMask::mapMemChErrors_.size() != 0 ) {
    map<EcalLogicID, RunMemChErrorsDat>::const_iterator m;
    for (m = EcalErrorMask::mapMemChErrors_.begin(); m != EcalErrorMask::mapMemChErrors_.end(); m++) {

      if ( (m->second).getErrorBits() & bits01 ) {
        EcalLogicID ecid = m->first;

        if ( strcmp(ecid.getMapsTo().c_str(), "EE_mem_channel") != 0 ) continue;

        int idcc = ecid.getID1() - 600;

        int ism = -1;
        if ( idcc >=   1 && idcc <=   9 ) ism = idcc;
        if ( idcc >=  46 && idcc <=  54 ) ism = idcc - 45 + 9;
        std::vector<int>::iterator iter = find(superModules_.begin(), superModules_.end(), ism);
        if (iter == superModules_.end()) continue;

        int ic = ecid.getID2();
        int ie = (ic-1)/5+1;
        int ip = (ic-1)%5+1;

        UtilsClient::maskBinContent( meg02_[ism-1], ie, ip );

      }

    }
  }

  if ( EcalErrorMask::mapMemTTErrors_.size() != 0 ) {
    map<EcalLogicID, RunMemTTErrorsDat>::const_iterator m;
    for (m = EcalErrorMask::mapMemTTErrors_.begin(); m != EcalErrorMask::mapMemTTErrors_.end(); m++) {

      if ( (m->second).getErrorBits() & bits02 ) {
        EcalLogicID ecid = m->first;

        if ( strcmp(ecid.getMapsTo().c_str(), "EE_mem_TT") != 0 ) continue;

        int idcc = ecid.getID1() - 600;

        int ism = -1;
        if ( idcc >=   1 && idcc <=   9 ) ism = idcc;
        if ( idcc >=  46 && idcc <=  54 ) ism = idcc - 45 + 9;
        std::vector<int>::iterator iter = find(superModules_.begin(), superModules_.end(), ism);
        if (iter == superModules_.end()) continue;

        int itt = ecid.getID2();

        for ( int ie = 5*(itt-68-1)+1; ie <= 5*(itt-68); ie++ ) {
          for ( int ip = 1; ip <= 5; ip++ ) {
            UtilsClient::maskBinContent( meg02_[ism-1], ie, ip );
          }
        }

      }

    }
  }
#endif

}

const int EEIntegrityClient::chNum[5][5] = {
  { 1,  2,  3,  4,  5},
  {10,  9,  8,  7,  6},
  {11, 12, 13, 14, 15},
  {20, 19, 18, 17, 16},
  {21, 22, 23, 24, 25}
};

