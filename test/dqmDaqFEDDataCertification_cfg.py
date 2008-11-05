import FWCore.ParameterSet.Config as cms

process = cms.Process("DataCert")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.load("DQMServices.Components.DQMOfflineCosmics_Certification_cff")

process.DaqData.FedGranularityInput = True
process.DaqData.saveDCFile = False

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/tmp/segoni/FileFromRun648148.root')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)
process.DQMStore = cms.Service("DQMStore",
    verbose = cms.untracked.int32(0)
)




process.CondDBCommon.connect = 'oracle://cms_orcoff_prod/CMS_COND_21X_RUN_INFO'
process.CondDBCommon.DBParameters.authenticationPath = '/afs/cern.ch/cms/DB/conddb/'

process.rn = cms.ESSource("PoolDBESSource",
    process.CondDBCommon,
    timetype = cms.string('runnumber'),
    toGet = cms.VPSet(cms.PSet(
	record = cms.string('RunInfoRcd'),
	tag = cms.string('runinfo_test')
    ))
)


process.PoolSource.fileNames = ['/store/data/Commissioning08/Cosmics/RECO/v1/000/066/394/229A4AEE-359B-DD11-B144-000423D99B3E.root']



process.asciiprint = cms.OutputModule("AsciiOutputModule")


process.p = cms.Path(process.DaqData)
process.ep = cms.EndPath(process.asciiprint)

