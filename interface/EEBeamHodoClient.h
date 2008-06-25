#ifndef EEBeamHodoClient_H
#define EEBeamHodoClient_H

/*
 * \file EEBeamHodoClient.h
 *
 * $Date: 2008/06/25 14:16:16 $
 * $Revision: 1.21 $
 * \author G. Della Ricca
 * \author G. Franzoni
 *
*/

#include <vector>
#include <string>

#include "TROOT.h"
#include "TProfile2D.h"
#include "TH1F.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQM/EcalEndcapMonitorClient/interface/EEClient.h"

class MonitorElement;
class DQMStore;
class EcalCondDBInterface;
class RunIOV;
class MonRunIOV;

class EEBeamHodoClient : public EEClient {

public:

/// Constructor
EEBeamHodoClient(const edm::ParameterSet& ps);

/// Destructor
virtual ~EEBeamHodoClient();

/// Analyze
void analyze(void);

/// BeginJob
void beginJob(DQMStore* dqmStore);

/// EndJob
void endJob(void);

/// BeginRun
void beginRun(void);

/// EndRun
void endRun(void);

/// Setup
void setup(void);

/// Cleanup
void cleanup(void);

/// SoftReset
void softReset(bool flag);

/// HtmlOutput
void htmlOutput(int run, std::string& htmlDir, std::string& htmlName);

/// WriteDB
bool writeDb(EcalCondDBInterface* econn, RunIOV* runiov, MonRunIOV* moniov, bool& status, bool flag);

/// Get Functions
inline int getEvtPerJob() { return ievt_; }
inline int getEvtPerRun() { return jevt_; }

private:

int ievt_;
int jevt_;

bool cloneME_;

bool verbose_;
bool debug_;

std::string prefixME_;

bool enableCleanup_;

std::vector<int> superModules_;

DQMStore* dqmStore_;

TH1F* ho01_[4];
TH1F* hr01_[4];

TH1F* hp01_[2];
TH2F* hp02_;

TH1F* hs01_[2];

TH1F* hq01_[2];

TH1F* ht01_;

TH1F* hc01_[3];

TH1F* hm01_;

TProfile* he01_[2];
TH2F* he02_[2];

TH1F* he03_[3];

};

#endif
