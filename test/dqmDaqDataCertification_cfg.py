import FWCore.ParameterSet.Config as cms

process = cms.Process("DataCert")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.load("DQMServices.Components.DQMOfflineCosmics_Certification_cff")

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/tmp/segoni/FileFromRun648148.root')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)
process.DQMStore = cms.Service("DQMStore",
    verbose = cms.untracked.int32(1)
)



process.PoolSource.fileNames = ['/store/data/Commissioning08/Cosmics/RECO/v1/000/064/257/04264E22-CD90-DD11-80EF-000423D9939C.root']
process.CondDBCommon.connect = 'oracle://cms_orcoff_prod/CMS_COND_21X_RUN_INFO'
process.CondDBCommon.DBParameters.authenticationPath = '/afs/cern.ch/cms/DB/conddb/'

process.rn = cms.ESSource("PoolDBESSource",
    process.CondDBCommon,
    timetype = cms.string('runnumber'),
    toGet = cms.VPSet(cms.PSet(
	record = cms.string('RunSummaryRcd'),
	tag = cms.string('runsummary_test')
    ))
)

process.asciiprint = cms.OutputModule("AsciiOutputModule")


process.p = cms.Path(process.DaqData)
process.ep = cms.EndPath(process.asciiprint)

