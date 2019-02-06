import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

# minimum of logs
process.MessageLogger = cms.Service("MessageLogger",
  statistics = cms.untracked.vstring(),
  destinations = cms.untracked.vstring('cout'),
  cout = cms.untracked.PSet(
    threshold = cms.untracked.string('WARNING')
  )
)

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
# process.GlobalTag.globaltag = "101X_dataRun2_Express_v8"
process.GlobalTag.globaltag = "101X_dataRun2_Prompt_v11"

process.maxEvents = cms.untracked.PSet(
  input = cms.untracked.int32(-1)
)

process.source = cms.Source("PoolSource",
  fileNames = cms.untracked.vstring($INPUT)
)

process.analyzer = cms.EDAnalyzer("PromptAnalyzer",
  tracks = cms.InputTag('generalTracks'),
  dedxs = cms.InputTag('dedxHarmonic2'),
  dedxPIXs = cms.InputTag('dedxPixelHarmonic2'),
  dedxpixels = cms.InputTag('dedxHitInfo'),
  RPtracks = cms.InputTag('ctppsLocalTrackLiteProducer'),
  vertices = cms.InputTag('offlinePrimaryVertices'),
  triggers = cms.InputTag('TriggerResults','','HLT'),
  pflows = cms.InputTag('particleFlow'),
  muons = cms.InputTag('muons'),

  outputFileName = cms.string("output.root")
)

process.p = cms.Path(process.analyzer)
