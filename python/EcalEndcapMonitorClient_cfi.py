import FWCore.ParameterSet.Config as cms

ecalEndcapMonitorClient = cms.EDAnalyzer("EcalEndcapMonitorClient",
    inputFile = cms.untracked.string(''),
    dbName = cms.untracked.string(''),
    dbHostName = cms.untracked.string(''),
    dbHostPort = cms.untracked.int32(1521),
    dbUserName = cms.untracked.string(''),
    dbPassword = cms.untracked.string(''),
    dbTagName = cms.untracked.string('CMSSW'),
    mergeRuns = cms.untracked.bool(False),
    location = cms.untracked.string(''),
    cloneME = cms.untracked.bool(False),
    updateTime = cms.untracked.int32(0),
    dbUpdateTime = cms.untracked.int32(0),
    resetFile = cms.untracked.string(''),
    enabledClients = cms.untracked.vstring('Integrity', 
        'Occupancy', 
        'StatusFlags', 
        'PedestalOnline', 
        'Pedestal', 
        'TestPulse', 
        'Laser', 
        'Led', 
        'Timing', 
        'Cosmic', 
        'Summary'),
    prefixME = cms.untracked.string('EcalEndcap'),
    enableCleanup = cms.untracked.bool(False),
    superModules = cms.untracked.vint32(1, 2, 3, 4, 5, 
        6, 7, 8, 9, 10, 
        11, 12, 13, 14, 15, 
        16, 17, 18),
    laserWavelengths = cms.untracked.vint32(1, 2, 3, 4),
    ledWavelengths = cms.untracked.vint32(1, 2),
    MGPAGains = cms.untracked.vint32(1, 6, 12),
    MGPAGainsPN = cms.untracked.vint32(1, 16),
    timingNHitThreshold = cms.untracked.int32(4),
    synchErrorThreshold = cms.untracked.int32(5),
    verbose = cms.untracked.bool(True),
    debug = cms.untracked.bool(False),
    prescaleFactor = cms.untracked.int32(1)
)

