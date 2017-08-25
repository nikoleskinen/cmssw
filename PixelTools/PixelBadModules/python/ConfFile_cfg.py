import FWCore.ParameterSet.Config as cms
from Configuration.AlCa.GlobalTag import GlobalTag 

process = cms.Process("PixelBadModules")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(threshold = cms.untracked.string('WARNING')),
    destinations = cms.untracked.vstring('cout')
) 

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")               
process.load("Configuration.StandardSequences.GeometryRecoDB_cff") 

process.GlobalTag = GlobalTag(process.GlobalTag,'auto:phase1_2017_realistic', '')  
process.load("CondCore.CondDB.CondDB_cfi") 
cond = process.CondDB.clone(connect ='frontier://FrontierProd/CMS_CONDITIONS')
process.SiPixelQualityDBReader = cms.ESSource(
    "PoolDBESSource",
    cond,
    toGet = cms.VPSet(
        cms.PSet(record = cms.string('SiPixelQualityFromDbRcd'),
            tag = cms.string('SiPixelQuality_phase1_2017_v1'),
            )
        )
)
process.es_prefer_Quality = cms.ESPrefer("PoolDBESSource","SiPixelQualityDBReader") 

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1) )
process.source = cms.Source("EmptySource",)
process.PixelBadModules = cms.EDAnalyzer('PixelBadModules')
process.p = cms.Path(process.PixelBadModules)
