// Original Author:  Robert Ciesielski
// modified by Jan Kašpar

// system include files
#include <memory>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <istream>
#include <fstream>
#include <iomanip>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FWLite/interface/ChainEvent.h"

// dEdx
#include "DataFormats/TrackReco/interface/DeDxHit.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TLorentzVector.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

// PPS
#include "DataFormats/CTPPSDetId/interface/TotemRPDetId.h"
#include "DataFormats/CTPPSReco/interface/TotemRPRecHit.h"
#include "DataFormats/CTPPSReco/interface/TotemRPUVPattern.h"
#include "DataFormats/CTPPSReco/interface/CTPPSLocalTrackLite.h"

// PFCandidates
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

// optics
const double v_x_R_1_F = -2.24791387053766;  const double L_x_R_1_F = 0.125396407127792E3;
const double v_y_R_1_F = +0.025781593410852; const double L_y_R_1_F = 238.517247191010E3;
const double v_x_R_2_F = -1.92610996810677;  const double L_x_R_2_F = -3.00655323980445E3;
const double v_y_R_2_F = -0.000000021508565; const double L_y_R_2_F = 271.511335947517E3;

const double v_x_L_1_F = -2.24791387053766;  const double L_x_L_1_F = 0.125396407127792E3;
const double v_y_L_1_F = +0.025781593410852; const double L_y_L_1_F = 238.517247191010E3;
const double v_x_L_2_F = -1.92610996810677;  const double L_x_L_2_F = -3.00655323980445E3;
const double v_y_L_2_F = -0.000000021508565; const double L_y_L_2_F = 271.511335947517E3;

const double m_pi = 0.13957;
const double m_k = 0.493677;
const double m_p = 0.93827;

using namespace edm;
using namespace reco;
using namespace std;

//----------------------------------------------------------------------------------------------------

class PromptAnalyzer : public edm::one::EDFilter<>
{
  public:
    explicit PromptAnalyzer(const edm::ParameterSet&);
    ~PromptAnalyzer() {}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    virtual void beginJob() override;
    virtual bool filter(edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    void evaluateDEDxCurves(double p, double &piu3, double &kd3, double &ku3);
    bool isPionCurve(double p, double dEdx);
    bool isSafePionCurve(double p, double dEdx);
    bool isKaonCurve(double p, double dEdx);
    bool isProtonCurve(double p, double dEdx);

    edm::EDGetTokenT<reco::TrackCollection> trkToken_;
    edm::EDGetTokenT<edm::ValueMap<reco::DeDxData>> dedxToken_ ;
    edm::EDGetTokenT<edm::ValueMap<reco::DeDxData>> dedxPIXToken_ ;
    //edm::EDGetTokenT<reco::DeDxHitInfoAss> dedxpixelToken_ ;
    edm::EDGetTokenT<reco::VertexCollection> vtxToken_;
    //edm::EDGetTokenT<edm::TriggerResults>  trigToken_;
    //edm::EDGetTokenT<reco::PFCandidateCollection> pfToken_;
    //edm::EDGetTokenT<reco::MuonCollection> muToken_;
    //edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster> > clusterToken_;

    edm::EDGetTokenT<edm::DetSetVector<TotemRPRecHit>> rpRecHitToken_;
    edm::EDGetTokenT<edm::DetSetVector<TotemRPUVPattern>> rpPatternToken_;
    edm::EDGetTokenT<vector<CTPPSLocalTrackLite>> rpTrackToken_;

    map<string, TH1F*> histosTH1F;
    map<string, TH2F*> histosTH2F;

    std::string outputFileName;
};

//----------------------------------------------------------------------------------------------------

PromptAnalyzer::PromptAnalyzer(const edm::ParameterSet& iConfig) :
  trkToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"))),
  dedxToken_(consumes<edm::ValueMap<reco::DeDxData>>(iConfig.getParameter<edm::InputTag>("dedxs"))),
  dedxPIXToken_(consumes<edm::ValueMap<reco::DeDxData>>(iConfig.getParameter<edm::InputTag>("dedxPIXs"))),
  //dedxpixelToken_(consumes<reco::DeDxHitInfoAss>(iConfig.getParameter<edm::InputTag>("dedxpixels"))),
  vtxToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
  //pfToken_(consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("pflows"))),
  //muToken_(consumes<reco::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"))),

  rpRecHitToken_(consumes<edm::DetSetVector<TotemRPRecHit>>(iConfig.getParameter<edm::InputTag>("rpRecHitTag"))),
  rpPatternToken_(consumes<edm::DetSetVector<TotemRPUVPattern>>(iConfig.getParameter<edm::InputTag>("rpPatternTag"))),
  rpTrackToken_(consumes<vector<CTPPSLocalTrackLite>>(iConfig.getParameter<edm::InputTag>("rpTrackTag"))),

  outputFileName(iConfig.getParameter<std::string>("outputFileName"))
{
}

//----------------------------------------------------------------------------------------------------

void PromptAnalyzer::beginJob()
{
  int nbins_eta = 80;
  int nbins_pt = 100;
  int nbins_phi = 64;

  histosTH1F["hlooper"] = new TH1F("hlooper","isLooper",5,0,5);
  histosTH1F["hpt"] = new TH1F("hpt","p_{T}",nbins_pt,0,5);
  histosTH1F["heta"] = new TH1F("heta","#eta",nbins_eta,-4,4);
  histosTH1F["hphi"] = new TH1F("hphi","#varphi",nbins_phi,-3.2,3.2);

  histosTH1F["halgo"] = new TH1F("halgo","Algo",15,0,15.);
  histosTH1F["hnhits"] = new TH1F("hnhits","nhits pix+strip",40,0,40.);
  histosTH1F["hchi2"] = new TH1F("hchi2","normalized #chi^{2}",1050,-50,1000.);
  histosTH1F["hdz"] = new TH1F("hdz","dz",1000,-200,200.);
  histosTH1F["hd0"] = new TH1F("hd0","d0",2000,-20,20.);

  histosTH1F["hlooperv"] = new TH1F("hlooperv","isLooper",5,0,5);
  histosTH1F["hptv"] = new TH1F("hptv","p_{T}",nbins_pt,0,5);
  histosTH1F["hetav"] = new TH1F("hetav","#eta",nbins_eta,-4,4);
  histosTH1F["hphiv"] = new TH1F("hphiv","#varphi",nbins_phi,-3.2,3.2);

  histosTH1F["halgov"] = new TH1F("halgov","Algo",15,0,15.);
  histosTH1F["hnhitsv"] = new TH1F("hnhitsv","nhits pixel",40,0,40.);
  histosTH1F["hchi2v"] = new TH1F("hchi2v","normalized #chi^{2} vtx-fitted",550,-50,500.);
  histosTH1F["hdzv"] = new TH1F("hdzv","dz vtx-fitted",500,-100,100.);
  histosTH1F["hd0v"] = new TH1F("hd0v","d0 vtx-fitted",2000,-20,20.);

  histosTH1F["hntrk0"] = new TH1F("hntrk0","Ntrk",150,0,150);
  histosTH1F["hntrk"] = new TH1F("hntrk","Ntrk for nPixelHits>0",150,0,150);
  histosTH1F["hntrkvtx"] = new TH1F("hntrkvtx","Ntrk vtx",150,0,150);

  histosTH1F["hntrk02"] = new TH1F("hntrk02","Ntrk",150,0,150);
  histosTH1F["hntrkvtx2"] = new TH1F("hntrkvtx2","Ntrk vtx",150,0,150);

  histosTH1F["hntrk04"] = new TH1F("hntrk04","Ntrk",150,0,150);
  histosTH1F["hntrkvtx4"] = new TH1F("hntrkvtx4","Ntrk vtx",150,0,150);

  histosTH1F["hvtx"] = new TH1F("hvtx","vtx.isFake()",2,0,2);
  histosTH1F["hvtx2"] = new TH1F("hvtx2","vtx.isFake() 2 tracks",2,0,2);
  histosTH1F["hvtx3"] = new TH1F("hvtx3","vtx.isFake() 2 tracks both |#eta|<2.5 and OS",2,0,2);

  histosTH1F["hnvtx"] = new TH1F("hnvtx","Nvtx",10,0,10);
  histosTH1F["hvtxx"] = new TH1F("hvtxx","X vtx",1000,-1.,1.);
  histosTH1F["hvtxy"] = new TH1F("hvtxy","Y vtx",1000,-1.,1.);
  histosTH1F["hvtxz"] = new TH1F("hvtxz","Z vtx",300,-30.,30.);
  histosTH1F["hvtxchi2"] = new TH1F("hvtxchi2","chi2 vtx",1100,-100.,1000.);
  //histosTH1F["hvtxndof"] = new TH1F("hvtxndof","ndof vtx",1020,-2.,100.);
  //histosTH1F["hvtxchi2fin"] = new TH1F("hvtxchi2fin","chi2 vtx",1100,-100.,1000.);

  //--------------------------------------
  // RPs

  histosTH1F["hnConf"] = new TH1F("", "Number of configurations (TB or BT or TT or BB)" , 5, 0., 5.);
  histosTH1F["hnConfClean"] = new TH1F("", "Number of clean configurations (TB or BT or TT or BB)" , 5, 0., 5.);

  vector<string> labRP = { "TB", "BT", "TT", "BB" };
  histosTH1F["hConf"]  = new TH1F("", "", labRP.size(), 0, labRP.size());
  for (size_t k = 0; k < labRP.size(); ++k)
    histosTH1F["hConf"]->GetXaxis()->SetBinLabel((k+1), labRP[k].c_str());

  histosTH1F["hthyEla"] = new TH1F("hthyEla"  ,"#theta_{Y}^{L}+#theta_{Y}^{R}", 2000 , -0.0004 , 0.0004);
  histosTH1F["hthxEla"] = new TH1F("hthxEla"  ,"#theta_{X}^{L}+#theta_{X}^{R}", 2000 , -0.0004 , 0.0004);

  histosTH2F["hthx2DIM"] = new TH2F("hthx2DIM"  , "#theta_{X}^{R} vs #theta_{X}^{L}" ,400,-0.0004,0.0004,400,-0.0004,0.0004);
  histosTH2F["hthythx2DIM"] = new TH2F("hthythx2DIM"  , "#theta_{Y} vs #theta_{X}" ,800,-0.0004,0.0004,800,-0.0004,0.0004);

  histosTH1F["hthyEla2"] = new TH1F("hthyEla2"  ,"#theta_{Y}^{L}+#theta_{Y}^{R}", 2000 , -0.0004 , 0.0004);
  histosTH1F["hthxEla2"] = new TH1F("hthxEla2"  ,"#theta_{X}^{L}+#theta_{X}^{R}", 2000 , -0.0004 , 0.0004);

  //--------------------------------------
  // CMS-TOTEM matching

  histosTH1F["hdpy2trk"] = new TH1F("hdpy2trk","2trk, p^{CMS}_{Y}+p^{TOTEM}_{Y}",500,-0.5,0.5);
  histosTH1F["hdpx2trk"] = new TH1F("hdpx2trk","2trk, p^{CMS}_{X}+p^{TOTEM}_{X}",500,-0.5,0.5);

  histosTH1F["hdpy2trkB"] = new TH1F("hdpy2trkB","2trk, p^{CMS}_{Y}+p^{TOTEM}_{Y}",500,-0.5,0.5);
  histosTH1F["hdpx2trkB"] = new TH1F("hdpx2trkB","2trk, p^{CMS}_{X}+p^{TOTEM}_{X}",500,-0.5,0.5);

  histosTH2F["h2DIMdpy2trk"] = new TH2F("h2DIMdpy2trk","2trk, p^{TOTEM}_{Y} vs p^{CMS}_{Y}",200,-2.,2.,200,-2.,2.);
  histosTH2F["h2DIMdpx2trk"] = new TH2F("h2DIMdpx2trk","p^{TOTEM}_{X} vs p^{CMS}_{X}",200,-2.,2.,200,-2.,2.);

  histosTH1F["hdpy4trk"] = new TH1F("hdpy4trk","p^{CMS}_{Y}+p^{TOTEM}_{Y}",500,-0.5,0.5);
  histosTH1F["hdpx4trk"] = new TH1F("hdpx4trk","p^{CMS}_{X}+p^{TOTEM}_{X}",500,-0.5,0.5);

  histosTH1F["hdpy4trkB"] = new TH1F("hdpy4trkB","p^{CMS}_{Y}+p^{TOTEM}_{Y}",500,-0.5,0.5);
  histosTH1F["hdpx4trkB"] = new TH1F("hdpx4trkB","p^{CMS}_{X}+p^{TOTEM}_{X}",500,-0.5,0.5);

  histosTH2F["h2DIMdpy4trk"] = new TH2F("h2DIMdpy4trk","4trk, p^{TOTEM}_{Y} vs p^{CMS}_{Y}",200,-2.,2.,200,-2.,2.);
  histosTH2F["h2DIMdpx4trk"] = new TH2F("h2DIMdpx4trk","4trk, p^{TOTEM}_{X} vs p^{CMS}_{X}",200,-2.,2.,200,-2.,2.);

  histosTH1F["hLS"] = new TH1F("hLS","LS",5000,0.,5000.);
  histosTH1F["hLS2"] = new TH1F("hLS2","2trk rate vs LS",5000,0.,5000.);
  histosTH1F["hLS2ela"] = new TH1F("hLS2ela","2trk, elastic background rate vs LS",5000,0.,5000.);

  // Mass spectra

  int massbins=500;
  int massbins4=2000;

  histosTH1F["hm2"]    = new TH1F("hm2", "M_{#pi^{+}#pi^{-}} (GeV)", massbins4,0.,10.);
  histosTH1F["hm4"]    = new TH1F("hm4", "M_{#pi^{+}#pi^{+}#pi^{-}#pi^{-}} (GeV)", massbins4,0.,10.);

  histosTH1F["hm4SIG1"] = new TH1F("hm4SIG1", "M_{#rho#rho} #sigma", massbins4,0.,10.);
  histosTH1F["hm4SIG2"] = new TH1F("hm4SIG2", "M_{#rho#rho} #sigma/2", massbins4,0.,10.);
  histosTH1F["hm4PHI"] = new TH1F("hm4PHI", "M_{4K}", massbins4,0.,10.);
  histosTH1F["hm4KST"] = new TH1F("hm4KST","M_{K^{*}(892)K^{*}(892)} ",massbins4,0.,10.);

  //--------------------------
  histosTH1F["hnOKpidKs"] = new TH1F("hnOKpidKs","Ncomb. K*K* ",5,0,5.);
  histosTH1F["hnOKpidKscurv"] = new TH1F("hnOKpidKscurv","Ncomb. K*K* ",5,0,5.);

  histosTH1F["hmALLkpi0"] = new TH1F("hmALLkpi0","M_{#pi#pi}",massbins,0,5.);
  histosTH1F["hmKSother0"] = new TH1F("hmKSother0","M_{K#pi} if K*(892)",massbins,0,5.);
  histosTH1F["hm4KST0"] = new TH1F("hm4KST0","M_{K^{*}(892)K^{*}(892)} ",massbins4,0.,10.);

  histosTH1F["hmALLkpi0curv"] = new TH1F("hmALLkpi0curv","M_{#pi#pi}",massbins,0,5.);
  histosTH1F["hmKSother0curv"] = new TH1F("hmKSother0curv","M_{K#pi} if K*(892)",massbins,0,5.);
  histosTH1F["hm4KST0curv"] = new TH1F("hm4KST0curv","M_{K^{*}(892)K^{*}(892)} ",massbins4,0.,10.);
  //--------------------------

  histosTH1F["hm4PHIscaled"] = new TH1F("hm4PHIscaled", "M_{4K}", massbins4,0.,10.);
  histosTH1F["hm4PHIscaled2"] = new TH1F("hm4PHIscaled2", "M_{4K}", massbins4,0.,10.);

  histosTH1F["hnKaons"] = new TH1F("hnKaons","N K curves in phiphi mass region",5,0,5.);
  histosTH1F["hnKaons_PhiCutStrict"] = new TH1F("hnKaons_PhiCutStrict","N K curves in phiphi mass region",5,0,5.);

  histosTH1F["hnPionsSIG1"] = new TH1F("hnPionsSIG1","N #pi curves in rhorho mass region",5,0,5.);
  histosTH1F["hnPionsSIG2"] = new TH1F("hnPionsSIG2","N #pi curves in rhorho mass region",5,0,5.);

  histosTH1F["hm4SIG1mass"] = new TH1F("hm4SIG1mass", "M_{#rho#rho} #sigma", massbins4,0.,10.);
  histosTH1F["hm4SIG1mass4curves"] = new TH1F("hm4SIG1mass4curves", "M_{#rho#rho} #sigma + 4#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG1mass34curves"] = new TH1F("hm4SIG1mass34curves", "M_{#rho#rho} #sigma + >=3#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG1mass234curves"] = new TH1F("hm4SIG1mass234curves", "M_{#rho#rho} #sigma + >=2#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG1mass1234curves"] = new TH1F("hm4SIG1mass1234curves", "M_{#rho#rho} #sigma + >1=#pi curves", massbins4,0.,10.);

  histosTH1F["hm4SIG2mass"] = new TH1F("hm4SIG2mass", "M_{#rho#rho} #sigma/2", massbins4,0.,10.);
  histosTH1F["hm4SIG2mass4curves"] = new TH1F("hm4SIG2mass4curves", "M_{#rho#rho} #sigma/2 + 4#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG2mass34curves"] = new TH1F("hm4SIG2mass34curves", "M_{#rho#rho} #sigma/2 + >=3#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG2mass234curves"] = new TH1F("hm4SIG2mass234curves", "M_{#rho#rho} #sigma/2 + >=2#pi curves", massbins4,0.,10.);
  histosTH1F["hm4SIG2mass1234curves"] = new TH1F("hm4SIG2mass1234curves", "M_{#rho#rho} #sigma/2 + >=1#pi curves", massbins4,0.,10.);

  histosTH1F["hm4PHImass"] = new TH1F("hm4PHImass", "M_{4K}, #varphi#varphi",massbins4,0.,10.);
  histosTH1F["hm4PHImass4curves"] = new TH1F("hm4PHImass4curves", "M_{4K}, #varphi#varphi + 4K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass3curves"] = new TH1F("hm4PHImass3curves", "M_{4K}, #varphi#varphi + 3K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass34curves"] = new TH1F("hm4PHImass34curves", "M_{4K}, #varphi#varphi + >=3K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass234curves"] = new TH1F("hm4PHImass234curves", "M_{4K}, #varphi#varphi + >=2K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass1234curves"] = new TH1F("hm4PHImass1234curves", "M_{4K}, #varphi#varphi + >=1K curves",massbins4,0.,10.);

  histosTH1F["hm4PHImassPionProtonVeto"] = new TH1F("", "M_{4K}, #varphi#varphi + Pion Veto",massbins4,0.,10.);

  histosTH1F["hm4PHImassSafePionProtonVeto"] = new TH1F("", "M_{4K}, #varphi#varphi + SafePionProton Veto",massbins4,0.,10.);
  histosTH1F["hm4PHImass1234curvesSafePionProtonVeto"] = new TH1F("", "M_{4K}, #varphi#varphi + >=1K curves + Pion Veto",massbins4,0.,10.);

  histosTH1F["hm4PHImass2SS"] = new TH1F("", "",massbins4,0.,10.);
  histosTH1F["hm4PHImass2SSSafePionProtonVeto"] = new TH1F("", "",massbins4,0.,10.);

  histosTH1F["hm4PHImass_PhiCutStrict"] = new TH1F("hm4PHImass_PhiCutStrict", "M_{4K}, #varphi#varphi",massbins4,0.,10.);
  histosTH1F["hm4PHImass4curves_PhiCutStrict"] = new TH1F("hm4PHImass4curves_PhiCutStrict", "M_{4K}, #varphi#varphi + 4K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass3curves_PhiCutStrict"] = new TH1F("hm4PHImass3curves_PhiCutStrict", "M_{4K}, #varphi#varphi + 3K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass34curves_PhiCutStrict"] = new TH1F("hm4PHImass34curves_PhiCutStrict", "M_{4K}, #varphi#varphi + >=3K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass234curves_PhiCutStrict"] = new TH1F("hm4PHImass234curves_PhiCutStrict", "M_{4K}, #varphi#varphi + >=2K curves",massbins4,0.,10.);
  histosTH1F["hm4PHImass1234curves_PhiCutStrict"] = new TH1F("hm4PHImass1234curves_PhiCutStrict", "M_{4K}, #varphi#varphi + >=1K curves",massbins4,0.,10.);

  histosTH1F["hm4PHImassPionProtonVeto_PhiCutStrict"] = new TH1F("", "M_{4K}, #varphi#varphi + PionProtonVeto",massbins4,0.,10.);

  histosTH1F["hm4PHImassSafePionProtonVeto_PhiCutStrict"] = new TH1F("", "M_{4K}, #varphi#varphi + SafePionProtonVeto",massbins4,0.,10.);
  histosTH1F["hm4PHImass1234curvesSafePionProtonVeto_PhiCutStrict"] = new TH1F("", "M_{4K}, #varphi#varphi + >=1K curves + SafePionProtonVeto",massbins4,0.,10.);

  histosTH1F["hm4PHImass2SS_PhiCutStrict"] = new TH1F("", "",massbins4,0.,10.);
  histosTH1F["hm4PHImass2SSSafePionProtonVeto_PhiCutStrict"] = new TH1F("", "",massbins4,0.,10.);

  histosTH1F["h_m_4K_PEAK"] = new TH1F("h_m_4K_PEAK", "M_{4K}", massbins4, 0., 10.);
  histosTH1F["h_m_4pi_PEAK"] = new TH1F("h_pi_4pi_PEAK", "M_{4#pi}", massbins4, 0., 10.);

  //-----

  histosTH1F["hm2BKG"] = new TH1F("hm2BKG", "TOTEM2, M_{#pi^{+}#pi^{-}} (GeV)", massbins4,0.,10.);
  histosTH1F["hm4BKG"] = new TH1F("hm4BKG", "M_{#pi^{+}#pi^{+}#pi^{-}#pi^{-}} (GeV)", massbins4,0.,10.);

  histosTH1F["hpt2"] = new TH1F("hpt2","p_{T}",40,0.,2.);
  histosTH1F["heta2"]= new TH1F("heta2","#eta",50,-5.,5.);

  histosTH1F["hpt2BKG"] = new TH1F("hpt2BKG","p_{T}",40,0.,2.);
  histosTH1F["heta2BKG"]= new TH1F("heta2BKG","#eta",50,-5.,5.);

  histosTH1F["hpt4"] = new TH1F("hpt4","p_{T}",40,0.,2.);
  histosTH1F["heta4"]= new TH1F("heta4","#eta",50,-5.,5.);

  histosTH1F["hpt4BKG"] = new TH1F("hpt4BKG","p_{T}",40,0.,2.);
  histosTH1F["heta4BKG"]= new TH1F("heta4BKG","#eta",50,-5.,5.);

  //-----

  histosTH1F["hdphi2"] = new TH1F("hdphi2","#Delta#varphi_{LR}",320,0,TMath::Pi());
  histosTH1F["hdphi4"] = new TH1F("hdphi4","#Delta#varphi_{LR}",320,0,TMath::Pi());

  //---------
  histosTH1F["hmALLpiKen0"] = new TH1F("hmALLpiKen0","M_{#pi#pi}",massbins,0,5.);

  histosTH2F["hmALLpiKen0_2D"] = new TH2F("hmALLpiKen0_2D","M_{22 or 21}(#pi#pi) vs. M_{11 or 12}(#pi#pi)", 80, 0.4, 1.2, 80, 0.4, 1.2);
  histosTH2F["hmALLpiKen0_2D_PEAK"] = new TH2F("hmALLpiKen0_2D_PEAK","M_{22 or 21}(#pi#pi) vs. M_{11 or 12}(#pi#pi)", 80, 0.4, 1.2, 80, 0.4, 1.2);

  histosTH1F["hmrhootherKen0"] = new TH1F("hmrhootherKen0","M_{#pi#pi} if #rho",massbins,0,5.);

  histosTH1F["hmALLkKen0"] = new TH1F("hmALLkKen0","M_{KK}",massbins,0,5.);

  histosTH1F["hmphiotherKen0"] = new TH1F("hmphiotherKen0","M_{KK} if #varphi",massbins,0,5.);
  histosTH1F["hmphiotherKen0_PhiCutStrict"] = new TH1F("hmphiotherKen0_PhiCutStrict","M_{KK} if #varphi",massbins,0,5.);

  histosTH2F["hmALLkKen0_2D"] = new TH2F("hmALLkKen0_2D","M_{22 or 21}(KK) vs. M_{11 or 12}(KK)", 80, 0.9, 1.3, 80, 0.9, 1.3);

  histosTH1F["hmKSother"] = new TH1F("hmKSother","M_{K#pi} if K*(892)",massbins,0,5.);

  //---------

  histosTH1F["heHFhad"] = new TH1F("heHFhad","E HFhad",500,0.,50);
  histosTH1F["heHFemc"] = new TH1F("heHFemc","E HFemc",500,0.,50);

  histosTH1F["hnHF"] = new TH1F("hnHF","n HF",100,0.,100);
  histosTH1F["hnHFhad"] = new TH1F("hnHFhad","n HF had",100,0.,100);
  histosTH1F["hnHFemc"] = new TH1F("hnHFemc","n HF emc",100,0.,100);

  histosTH2F["hHF_Memc_2DIM"] = new TH2F("hHF_Memc_2DIM","N HFhad",600,-5.2,-2.8,160,-3.2,3.2);
  histosTH2F["hHF_Mhad_2DIM"] = new TH2F("hHF_Mhad_2DIM","N HFhad",600,-5.2,-2.8,160,-3.2,3.2);

  histosTH2F["hHF_Pemc_2DIM"] = new TH2F("hHF_Pemc_2DIM","N HFhad",600,2.8,5.2,160,-3.2,3.2);
  histosTH2F["hHF_Phad_2DIM"] = new TH2F("hHF_Phad_2DIM","N HFhad",600,2.8,5.2,160,-3.2,3.2);

  histosTH2F["heHF_Memc_2DIM"] = new TH2F("heHF_Memc_2DIM","E HFhad",60,-5.2,-2.8,100,0.,20.);
  histosTH2F["heHF_Mhad_2DIM"] = new TH2F("heHF_Mhad_2DIM","E HFhad",60,-5.2,-2.8,100,0.,20.);

  histosTH2F["heHF_Pemc_2DIM"] = new TH2F("heHF_Pemc_2DIM","E HFhad",60,2.8,5.2,100,0.,20.);
  histosTH2F["heHF_Phad_2DIM"] = new TH2F("heHF_Phad_2DIM","E HFhad",60,2.8,5.2,100,0.,20.);

  //------------
  // dEdx

  histosTH1F["hdedxDetector"] = new TH1F("hdedxDetector","dedx Detector",4,0,4.);

  histosTH2F["hdedxAllPIX"] = new TH2F("hdedxAllPIX","dE/dx vs p", 500, 0.,5.,400, 0.,20.);
  histosTH2F["hdedxAllSTR"] = new TH2F("hdedxAllSTR","dE/dx vs p", 500, 0.,5.,400, 0.,20.);
  histosTH2F["hdedxSTRvsPIX"] = new TH2F("hdedxSTRvsPIX","dE/dx STR vs dE/dx PIX", 200, 0.,10.,200, 0.,10.);

  histosTH2F["hdedx2trk"] = new TH2F("","dE/dx vs p 2trk", 500, 0., 5., 400, 0., 20.);
  histosTH2F["hdedx4trk"] = new TH2F("","dE/dx vs p 4trk", 500, 0., 5., 400, 0., 20.);

  histosTH2F["hdedx4trk_PionCurve"] = new TH2F("","dE/dx vs p 4trk, within pion curves", 500, 0.,5.,400, 0.,20.);
  histosTH2F["hdedx4trk_SafePionCurve"] = new TH2F("","dE/dx vs p 4trk, within safe pion curves", 500, 0.,5.,400, 0.,20.);
  histosTH2F["hdedx4trk_KaonCurve"] = new TH2F("","dE/dx vs p 4trk, within kaon curves", 500, 0.,5.,400, 0.,20.);
  histosTH2F["hdedx4trk_ProtonCurve"] = new TH2F("","dE/dx vs p 4trk, within proton curves", 500, 0.,5.,400, 0.,20.);

  histosTH2F["hdedx2trklog"] = new TH2F("hdedx2trklog","log(dE/dx) vs p 2trk", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx4trklog"] = new TH2F("hdedx4trklog","log(dE/dx) vs p 4trk", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx2pionslog"] = new TH2F("hdedx2pionslog","log(dE/dx) vs p 2#pi", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx2kaonslog"] = new TH2F("hdedx2kaonslog","log(dE/dx) vs p 2K", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx2pionslog35"] = new TH2F("hdedx2pionslog35","log(dE/dx) vs p 2#pi", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx2kaonslog35"] = new TH2F("hdedx2kaonslog35","log(dE/dx) vs p 2K", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx2pionslog40"] = new TH2F("hdedx2pionslog40","log(dE/dx) vs p 2#pi", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx2kaonslog40"] = new TH2F("hdedx2kaonslog40","log(dE/dx) vs p 2K", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx2pionslog45"] = new TH2F("hdedx2pionslog45","log(dE/dx) vs p 2#pi", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx2kaonslog45"] = new TH2F("hdedx2kaonslog45","log(dE/dx) vs p 2K", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx2pionslog50"] = new TH2F("hdedx2pionslog50","log(dE/dx) vs p 2#pi", 500, 0.,5.,300, -1.,2.);
  histosTH2F["hdedx2kaonslog50"] = new TH2F("hdedx2kaonslog50","log(dE/dx) vs p 2K", 500, 0.,5.,300, -1.,2.);

  histosTH2F["hdedx4PHImass"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass4curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass3curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass34curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass234curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass1234curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass2SS"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImassPionProtonVeto"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImassSafePionProtonVeto"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass1234curvesSafePionProtonVeto"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4PHImass2SSSafePionProtonVeto"] = new TH2F(* histosTH2F["hdedx4trk"]);

  histosTH2F["hdedx4PHImass_PEAK"] = new TH2F(* histosTH2F["hdedx4trk"]);

  histosTH2F["hdedx4SIG1mass"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4SIG1mass4curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4SIG1mass34curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4SIG1mass234curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
  histosTH2F["hdedx4SIG1mass1234curves"] = new TH2F(* histosTH2F["hdedx4trk"]);
}

//----------------------------------------------------------------------------------------------------

void PromptAnalyzer::endJob()
{
  if (outputFileName == "")
    return;

  TFile * f_out = TFile::Open(outputFileName.c_str(), "RECREATE");

  for (const auto &p : histosTH1F)
    p.second->Write(p.first.c_str());

  for (const auto &p : histosTH2F)
    p.second->Write(p.first.c_str());

  delete f_out;
}

//----------------------------------------------------------------------------------------------------

void PromptAnalyzer::evaluateDEDxCurves(double p, double &piu3, double &kd3, double &ku3)
{
  // 3sigma curves based on 2018 data with pbins=2
  ku3 = 4.53163e+00-5.89828e-01*p + exp(3.16431e+00-5.38011e+00*p);
  kd3 = 1.46882e+00-9.31222e-04*p + exp(2.12237e+00-4.34300e+00*p);
  piu3 = 3.49965e+00-2.88850e-01*atan(p-1.51050e+00) + exp(3.03012e+00-1.53867e+01*p);
}

//----------------------------------------------------------------------------------------------------

bool PromptAnalyzer::isPionCurve(double p, double dEdx)
{
  double piu3, kd3, ku3;
  evaluateDEDxCurves(p, piu3, kd3, ku3);

  return (dEdx < piu3);
}

//----------------------------------------------------------------------------------------------------

bool PromptAnalyzer::isSafePionCurve(double p, double dEdx)
{
  const double max = 0.9 / p;
  return (dEdx < max);
}

//----------------------------------------------------------------------------------------------------

bool PromptAnalyzer::isKaonCurve(double p, double dEdx)
{
  double piu3, kd3, ku3;
  evaluateDEDxCurves(p, piu3, kd3, ku3);

  const double lb = max(kd3, piu3);
  const double ub = ku3;

  return (dEdx > lb && dEdx < ub);
}

//----------------------------------------------------------------------------------------------------

bool PromptAnalyzer::isProtonCurve(double p, double dEdx)
{
  double piu3, kd3, ku3;
  evaluateDEDxCurves(p, piu3, kd3, ku3);

  return (dEdx > ku3);
}

//----------------------------------------------------------------------------------------------------

void PromptAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions)
{
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//----------------------------------------------------------------------------------------------------

bool PromptAnalyzer::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // defaults
  bool returnStatus = false;

  // get input
  edm::Handle<TrackCollection> tracks;
  iEvent.getByToken(trkToken_, tracks);

  edm::Handle<edm::ValueMap<reco::DeDxData>> dedxs;
  iEvent.getByToken(dedxToken_, dedxs);

  edm::Handle<edm::ValueMap<reco::DeDxData>> dedxPIXs;
  iEvent.getByToken(dedxPIXToken_, dedxPIXs);

  //edm::Handle<reco::DeDxHitInfoAss> dedxpixels;
  //iEvent.getByToken(dedxpixelToken_, dedxpixels);

  edm::Handle<VertexCollection> vertices;
  iEvent.getByToken(vtxToken_, vertices);

  //edm::Handle<PFCandidateCollection> pfs;
  //iEvent.getByToken(pfToken_, pfs);

  //edm::Handle<MuonCollection> muons;
  //iEvent.getByToken(muToken_, muons);

  //edm::Handle<reco::DeDxHitInfoAss> dedxCollH;
  //iEvent.getByLabel("dedxHitInfo", dedxCollH);

  edm::Handle<edm::DetSetVector<TotemRPRecHit>> hRPRecHits;
  iEvent.getByToken(rpRecHitToken_, hRPRecHits);

  edm::Handle<edm::DetSetVector<TotemRPUVPattern>> hRPPatterns;
  iEvent.getByToken(rpPatternToken_, hRPPatterns);

  edm::Handle<vector<CTPPSLocalTrackLite>> hRPTracks;
  iEvent.getByToken(rpTrackToken_, hRPTracks);


  //if (!dedxpixels.isValid())
  //.  printf("Invalid dedxCollH\n");

  int LS = iEvent.luminosityBlock();
  histosTH1F["hLS"]->Fill(LS);

  int ntrk0 = 0;
  int ntrk  = 0;

  bool fiducialRegion = false;

  double etaCut = 2.5;

  int totcharge = 0;

  // tracks in 2-track-events (npixelhits>0)
  TLorentzVector pi1(0.,0.,0.,0.);
  TLorentzVector pi2(0.,0.,0.,0.);
  TLorentzVector pipiRec(0.,0.,0.,0.);

  TLorentzVector k1(0.,0.,0.,0.);
  TLorentzVector k2(0.,0.,0.,0.);
  TLorentzVector kkRec(0.,0.,0.,0.);

  // tracks in 4-track-events (npixelhits>0)
  TLorentzVector pi4pos1(0.,0.,0.,0.);
  TLorentzVector pi4pos2(0.,0.,0.,0.);
  TLorentzVector pi4neg1(0.,0.,0.,0.);
  TLorentzVector pi4neg2(0.,0.,0.,0.);
  TLorentzVector pi4Rec(0.,0.,0.,0.);

  TLorentzVector k4pos1(0.,0.,0.,0.);
  TLorentzVector k4pos2(0.,0.,0.,0.);
  TLorentzVector k4neg1(0.,0.,0.,0.);
  TLorentzVector k4neg2(0.,0.,0.,0.);
  TLorentzVector k4Rec(0.,0.,0.,0.);

  bool iskaon4pos[2] = {false,false};
  bool iskaon4neg[2] = {false,false};

  bool ispion4pos[2] = {false,false};
  bool ispion4neg[2] = {false,false};

  int ntrk4pos=0;
  int ntrk4neg=0;

  /*
  bool is2PIsample = false;
  bool is2Ksample = false;

  bool is2PIsample2 = false;
  bool is2Ksample2 = false;

  bool is2PIsample3 = false;
  bool is2Ksample3 = false;

  bool is2PIsample4 = false;
  bool is2Ksample4 = false;

  bool is2PIsample5 = false;
  bool is2Ksample5 = false;
  */

  //----------------------------------------------------------------------
  // process tracks

  int itref=0;
  for (TrackCollection::const_iterator itTrack = tracks->begin(); itTrack != tracks->end(); ++itTrack)
  {
    int looper = itTrack->isLooper();
    //double p = itTrack->p();
    double pt = itTrack->pt();
    double pz = itTrack->pz();
    double eta = itTrack->eta();
    double phi = itTrack->phi();
    int charge = itTrack->charge();
    int npixelhits = itTrack->hitPattern().numberOfValidPixelHits();
    //int nstriphits = itTrack->hitPattern().numberOfValidStripHits();
    int algo = itTrack->algo();
    double chi2 = itTrack->normalizedChi2();
    double d0 = itTrack->d0();
    double dz = itTrack->dz();

    // dEdx
    reco::TrackRef trackRef = reco::TrackRef(tracks, itref);
    double thisdedx = (*dedxs)[trackRef].dEdx();
    double thisdedxPIX = (*dedxPIXs)[trackRef].dEdx();
    itref++;

    if (npixelhits > 0)
    {
      int hdedxDetector = 0;

      if (thisdedxPIX > 0)
      {
        hdedxDetector = 2;
        histosTH2F["hdedxAllPIX"]->Fill(itTrack->p(),thisdedxPIX);
      } else if(thisdedx>0) {
        hdedxDetector = 1;
        histosTH2F["hdedxAllSTR"]->Fill(itTrack->p(),thisdedx);
      }

      histosTH1F["hdedxDetector"]->Fill(hdedxDetector);
      histosTH2F["hdedxSTRvsPIX"]->Fill(thisdedxPIX,thisdedx);

      /*
      if ( isPionCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.6 ) is2PIsample=true;
      if ( isKaonCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.5 ) is2Ksample=true;

      if ( isPionCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.35 ) is2PIsample2=true;
      if ( isKaonCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.35 ) is2Ksample2=true;

      if ( isPionCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.40 ) is2PIsample3=true;
      if ( isKaonCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.40 ) is2Ksample3=true;

      if ( isPionCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.45 ) is2PIsample4=true;
      if ( isKaonCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.45 ) is2Ksample4=true;

      if ( isPionCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.50 ) is2PIsample5=true;
      if ( isKaonCurve(itTrack->p(),thisdedxPIX) && itTrack->p()<0.50 ) is2Ksample5=true;
      */

      histosTH1F["hpt"]->Fill(pt);
      histosTH1F["heta"]->Fill(eta);
      histosTH1F["hphi"]->Fill(phi);
      histosTH1F["halgo"]->Fill(algo);
      //histosTH1F["hnhits"]->Fill(npixelhits+nstriphits);
      histosTH1F["hnhits"]->Fill(npixelhits);

      histosTH1F["hlooper"]->Fill(looper);
      histosTH1F["hchi2"]->Fill(chi2);
      histosTH1F["hd0"]->Fill(d0);
      histosTH1F["hdz"]->Fill(dz);

      totcharge += charge;

      // four-momenta with different mass hypotheses
      const double ene_pi = sqrt(pt*pt + pz*pz + m_pi*m_pi);
      const TLorentzVector trk_lorentz_pi(itTrack->px(), itTrack->py(), itTrack->pz(), ene_pi);

      const double ene_K = sqrt(pt*pt + pz*pz + m_k*m_k);
      const TLorentzVector trk_lorentz_K(itTrack->px(), itTrack->py(), itTrack->pz(), ene_K);

      //--------------------------------------
      // 2 trk

      if (ntrk==0) pi1 = trk_lorentz_pi;
      if (ntrk==1) pi2 = trk_lorentz_pi;

      if (ntrk==0) k1 = trk_lorentz_K;
      if (ntrk==1) k2 = trk_lorentz_K;

      pipiRec += trk_lorentz_pi;
      kkRec += trk_lorentz_K;

      //--------------------------------------
      // 4trk

      pi4Rec += trk_lorentz_pi;
      k4Rec  += trk_lorentz_K;

      if (charge > 0)
      {
        if (ntrk4pos == 0) pi4pos1 = trk_lorentz_pi;
        if (ntrk4pos == 1) pi4pos2 = trk_lorentz_pi;

        if (ntrk4pos == 0) k4pos1 = trk_lorentz_K;
        if (ntrk4pos == 1) k4pos2 = trk_lorentz_K;

        if (ntrk4pos < 2)
        {
          if( isPionCurve(itTrack->p(),thisdedxPIX) ) ispion4pos[ntrk4pos] = true;
          if( isKaonCurve(itTrack->p(),thisdedxPIX) ) iskaon4pos[ntrk4pos] = true;
        }

        ntrk4pos++;
      }

      if (charge < 0)
      {
        if (ntrk4neg == 0) pi4neg1 = trk_lorentz_pi;
        if (ntrk4neg == 1) pi4neg2 = trk_lorentz_pi;

        if (ntrk4neg == 0) k4neg1 = trk_lorentz_K;
        if (ntrk4neg == 1) k4neg2 = trk_lorentz_K;

        if (ntrk4neg < 2)
        {
          if ( isPionCurve(itTrack->p(),thisdedxPIX) ) ispion4neg[ntrk4pos] = true;
          if ( isKaonCurve(itTrack->p(),thisdedxPIX) ) iskaon4neg[ntrk4pos] = true;
        }

        ntrk4neg++;
      }

      ntrk++; // pixel tracks
    }

    ntrk0++; // all tracks
  }

  histosTH1F["hntrk0"]->Fill(ntrk0);
  histosTH1F["hntrk"]->Fill(ntrk);

  //----------------------------------------------------------------------
  // process vertex and vertex tracks

  int nvtx=0;
  int ntrkvtx=0;

  for (VertexCollection::const_iterator itVtx = vertices->begin(); itVtx != vertices->end(); ++itVtx)
  {
    const int vtxisfake = itVtx->isFake();
    if (vtxisfake == 0)
      nvtx++;
    else
      continue;

    if (nvtx == 1) // executed for the first vertex found
    {
      ntrkvtx = 0; // count the number of tracks associted with the vertex

      for (auto tv=itVtx->tracks_begin(); tv!=itVtx->tracks_end(); tv++)
      {
        const reco::TrackRef trackRef = tv->castTo<reco::TrackRef>();

        int looper = trackRef->isLooper();
        double pt = trackRef->pt();
        //double pz = trackRef->pz();
        double eta = trackRef->eta();
        double phi = trackRef->phi();
        //int charge = trackRef->charge();
        int npixelhits = trackRef->hitPattern().numberOfValidPixelHits();
        //int nstriphits = trackRef->hitPattern().numberOfValidStripHits();
        int algo = trackRef->algo();
        double chi2 = trackRef->normalizedChi2();
        double d0 = trackRef->d0();
        double dz = trackRef->dz();

        histosTH1F["hptv"]->Fill(pt);
        histosTH1F["hetav"]->Fill(eta);
        histosTH1F["hphiv"]->Fill(phi);
        histosTH1F["halgov"]->Fill(algo);
        //histosTH1F["hnhitsv"]->Fill(npixelhits+nstriphits);
        histosTH1F["hnhitsv"]->Fill(npixelhits);

        histosTH1F["hlooperv"]->Fill(looper);

        histosTH1F["hchi2v"]->Fill(chi2);
        histosTH1F["hd0v"]->Fill(d0);
        histosTH1F["hdzv"]->Fill(dz);

        ntrkvtx++;
      }
    }
  }

  histosTH1F["hnvtx"]->Fill(nvtx);
  histosTH1F["hntrkvtx"]->Fill(ntrkvtx);

  // not yet vertex cut, because checking vertex efficiency

  int isfake = vertices->begin()->isFake();
  double xvtx = vertices->begin()->x();
  double yvtx = vertices->begin()->y();
  double zvtx = vertices->begin()->z();
  double chi2vtx = vertices->begin()->normalizedChi2();

  fiducialRegion = (ntrk==2 && TMath::Abs(pi1.Eta())<etaCut && TMath::Abs(pi2.Eta())<etaCut);

  histosTH1F["hvtx"]->Fill( isfake );
  if (ntrk == 2)
  {
    histosTH1F["hvtx2"]->Fill( isfake );
    if (fiducialRegion && totcharge==0) histosTH1F["hvtx3"]->Fill( isfake );
  }

  //  fiducialRegion = fiducialRegion && nvtx==1;

  if (nvtx != 1)
    return returnStatus;

  histosTH1F["hvtxx"]->Fill(xvtx);
  histosTH1F["hvtxy"]->Fill(yvtx);
  histosTH1F["hvtxz"]->Fill(zvtx);
  histosTH1F["hvtxchi2"]->Fill(chi2vtx);

  // cut on vertex z position
  if (fabs(zvtx) > 24)
    return returnStatus;

  //--------------------------------------------------------------------------------
  // process RP data

  // 2018 setup
  //------------------------------------------------
  // -z                    IP               +z
  //         sec45                  sec56
  //top:  24       4           104         124
  //ver:     23  3                 103 123
  //bot:  25       5           105         125
  //

  bool rp_valid_004 = false;
  bool rp_valid_005 = false;
  bool rp_valid_024 = false;
  bool rp_valid_025 = false;

  bool rp_valid_104 = false;
  bool rp_valid_105 = false;
  bool rp_valid_124 = false;
  bool rp_valid_125 = false;

  double xLN=100, xLF=100, yLN=100, yLF=100;
  double xRN=100, xRF=100, yRN=100, yRF=100;

  // T, from L to R, as on Jan's slides
  const double mean_x24  = -0.465;
  const double mean_x4   = -0.210;
  const double mean_x104 =  0.167;
  const double mean_x124 = -0.450;

  // B, from L to R
  const double mean_x25  = -0.081;
  const double mean_x5   = -0.112;
  const double mean_x105 =  0.373;
  const double mean_x125 = -0.574;

  // T, from L to R
  const double mean_y24  = -0.689;
  const double mean_y4   = -1.479;
  const double mean_y104 = -0.916;
  const double mean_y124 =  0.044;

  // B, from L to R
  const double mean_y25  = 0.009;
  const double mean_y5   = 0.842;
  const double mean_y105 = 1.312;
  const double mean_y125 = 0.316;

  // buffer for RP data
  struct StripInfo
  {
      set<unsigned int> u_planes;
      set<unsigned int> v_planes;

      map<unsigned int, unsigned int> clustersPerPlane;

      bool u_tooFull = false;
      bool v_tooFull = false;

      unsigned int u_patterns = 0;
      unsigned int v_patterns = 0;

      bool hasActiv = false;
      bool hasTrack = false;
  };

  map<unsigned int, StripInfo> rpInfo;

  // process rec hits
  for (const auto &dsRecHits : *hRPRecHits)
  {
      TotemRPDetId planeId(dsRecHits.detId());
      unsigned int rpDecId = planeId.arm()*100 + planeId.station()*10 + planeId.rp();
      unsigned int planeIdx = planeId.plane();

      rpInfo[rpDecId].clustersPerPlane[planeIdx] = dsRecHits.size();

      if (dsRecHits.size() == 0)
          continue;

      if ((planeIdx % 2) == 0)
          rpInfo[rpDecId].v_planes.insert(planeIdx);
      else
          rpInfo[rpDecId].u_planes.insert(planeIdx);
  }

  // process patterns
  for (const auto &dsPatterns : *hRPPatterns)
  {
      TotemRPDetId rpId(dsPatterns.detId());
      unsigned int rpDecId = rpId.arm()*100 + rpId.station()*10 + rpId.rp();

      for (const auto &pat : dsPatterns)
      {
          if (! pat.getFittable())
              continue;

          if (pat.getProjection() == TotemRPUVPattern::projU)
              rpInfo[rpDecId].u_patterns++;
          if (pat.getProjection() == TotemRPUVPattern::projV)
              rpInfo[rpDecId].v_patterns++;
      }
  }

  // process track data
  for (const auto &tr : *hRPTracks)
  {
    CTPPSDetId rpId(tr.getRPId());
    unsigned int rpDecId = 100*rpId.arm() + 10*rpId.station() + 1*rpId.rp();

    rpInfo[rpDecId].hasTrack = true;

    if(rpDecId == 4) {rp_valid_004 = true; xLN = tr.getX() + mean_x4; yLN = tr.getY() + mean_y4;}
    if(rpDecId == 5) {rp_valid_005 = true; xLN = tr.getX() + mean_x5; yLN = tr.getY() + mean_y5;}

    if(rpDecId == 24) {rp_valid_024 = true; xLF = tr.getX() + mean_x24; yLF = tr.getY() + mean_y24;}
    if(rpDecId == 25) {rp_valid_025 = true; xLF = tr.getX() + mean_x25; yLF = tr.getY() + mean_y25;}

    if(rpDecId == 104) {rp_valid_104 = true; xRN = tr.getX() + mean_x104; yRN = tr.getY() + mean_y104;}
    if(rpDecId == 105) {rp_valid_105 = true; xRN = tr.getX() + mean_x105; yRN = tr.getY() + mean_y105;}

    if(rpDecId == 124) {rp_valid_124 = true; xRF = tr.getX() + mean_x124; yRF = tr.getY() + mean_y124;}
    if(rpDecId == 125) {rp_valid_125 = true; xRF = tr.getX() + mean_x125; yRF = tr.getY() + mean_y125;}
  }

  // process all RP inputs
  for (auto &p : rpInfo)
  {
      auto &info = p.second;

      unsigned int n_too_full_u = 0, n_too_full_v = 0;

      for (const auto &clP : info.clustersPerPlane)
      {
          if (clP.second <= 5)
              continue;

          if ((clP.first % 2) == 1)
              n_too_full_u++;
          else
              n_too_full_v++;
      }

      info.u_tooFull = (n_too_full_u >= 3);
      info.v_tooFull = (n_too_full_v >= 3);

      info.hasActiv = info.hasTrack || info.u_patterns > 0 || info.v_patterns > 0 || info.u_tooFull || info.v_tooFull;
  }

  bool diag_top45_bot56 = rp_valid_024 && rp_valid_004 && rp_valid_105 && rp_valid_125;
  bool diag_bot45_top56 = rp_valid_025 && rp_valid_005 && rp_valid_104 && rp_valid_124;

  bool top45_top56      = rp_valid_024 && rp_valid_004 && rp_valid_104 && rp_valid_124;
  bool bot45_bot56      = rp_valid_025 && rp_valid_005 && rp_valid_105 && rp_valid_125;

  bool clean_bot45_top56 = (!rpInfo[24].hasActiv && rpInfo[25].hasTrack) && (!rpInfo[4].hasActiv && rpInfo[5].hasTrack) && (rpInfo[104].hasTrack && !rpInfo[105].hasActiv) && (rpInfo[124].hasTrack && !rpInfo[125].hasActiv);
  bool clean_top45_bot56 = (rpInfo[24].hasTrack && !rpInfo[25].hasActiv) && (rpInfo[4].hasTrack && !rpInfo[5].hasActiv) && (!rpInfo[104].hasActiv && rpInfo[105].hasTrack) && (!rpInfo[124].hasActiv && rpInfo[125].hasTrack);
  bool clean_top45_top56 = (rpInfo[24].hasTrack && !rpInfo[25].hasActiv) && (rpInfo[4].hasTrack && !rpInfo[5].hasActiv) && (rpInfo[104].hasTrack && !rpInfo[105].hasActiv) && (rpInfo[124].hasTrack && !rpInfo[125].hasActiv);
  bool clean_bot45_bot56 = (!rpInfo[24].hasActiv && rpInfo[25].hasTrack) && (!rpInfo[4].hasActiv && rpInfo[5].hasTrack) && (!rpInfo[104].hasActiv && rpInfo[105].hasTrack) && (!rpInfo[124].hasActiv && rpInfo[125].hasTrack);

  int nconf=0;
  if (diag_top45_bot56) nconf++;
  if (diag_bot45_top56) nconf++;
  if (top45_top56) nconf++;
  if (bot45_bot56) nconf++;

  unsigned int nConfClean = 0;
  if (clean_top45_bot56) nConfClean++;
  if (clean_bot45_top56) nConfClean++;
  if (clean_top45_top56) nConfClean++;
  if (clean_bot45_bot56) nConfClean++;

  histosTH1F["hnConf"]->Fill(nconf);
  histosTH1F["hnConfClean"]->Fill(nConfClean);

  if (nConfClean != 1)
    return returnStatus;

  // topology: 1 - TB, 2 - BT, 3 - TT, 4 - BB
  int tb = -1;
  if (clean_top45_bot56) tb = 0;
  if (clean_bot45_top56) tb = 1;
  if (clean_top45_top56) tb = 2;
  if (clean_bot45_bot56) tb = 3;

  histosTH1F["hConf"]->Fill(tb);

  // single-arm kinematics reconstruction
  double ThxR, ThyR, ThxL, ThyL;//, xVtxL, xVtxR;

  double D_x_L = + v_x_L_1_F * L_x_L_2_F - v_x_L_2_F * L_x_L_1_F;
  ThxL = (v_x_L_1_F * xLF - v_x_L_2_F * xLN) / D_x_L;
  //xVtxL = (- xLN * L_x_L_2_F + xLF * L_x_L_1_F) / D_x_L;

  double D_x_R = + v_x_R_1_F * L_x_R_2_F - v_x_R_2_F * L_x_R_1_F;
  ThxR = (v_x_R_1_F * xRF - v_x_R_2_F * xRN) / D_x_R;
  //xVtxR = (+ xRN * L_x_R_2_F - xRF * L_x_R_1_F) / D_x_R;

  double th_y_L_1_F = + yLN / L_y_L_1_F;
  double th_y_L_2_F = + yLF / L_y_L_2_F;
  ThyL = (th_y_L_1_F + th_y_L_2_F) / 2.;

  double th_y_R_1_F = + yRN / L_y_R_1_F;
  double th_y_R_2_F = + yRF / L_y_R_2_F;
  ThyR = (th_y_R_1_F + th_y_R_2_F) / 2.;

  // Correct residual shifts in thx (only, not needed thy)
  // 2015
  //  if(specialreco) ThxL=rec_proton_left->thx-5.04e-5;
  // 2018
  // was on during the run for Express
  // Gauss fit: shift xL+xR = -1.80371e-04
  // ThxR += 1.815e-04;
  //
  // my calculations from shift in dpx/6500

  double a_off =  0.000002386 ; // TB
  double b_off = -0.000006593 ; // BT
  double c_off = -0.000007524 ; // TT
  double d_off =  0.000003268 ; // BB

  if (tb==0) {ThxL += 0. ;           ThxR += a_off ;} // TB
  if (tb==1) {ThxL += (b_off-c_off); ThxR += c_off ;} // BT
  if (tb==2) {ThxL += 0. ;           ThxR += c_off ;} // TT
  if (tb==3) {ThxL += (d_off-a_off); ThxR += a_off ;} // BB

  histosTH1F["hthxEla"]->Fill(ThxL+ThxR);
  histosTH1F["hthyEla"]->Fill(ThyL+ThyR);

  histosTH2F["hthx2DIM"]->Fill(ThxL,ThxR);
  histosTH2F["hthythx2DIM"]->Fill(ThxL+ThxR,ThyL+ThyR);

  // elastic cut
  bool isElastic = false;

  // 5 sigma
  if (TMath::Abs(ThyL+ThyR)< 15e-6 && TMath::Abs(ThxL+ThxR)<45e-6)
    isElastic=true;

  if (isElastic)
    return returnStatus;

  histosTH1F["hthxEla2"]->Fill(ThxL+ThxR);
  histosTH1F["hthyEla2"]->Fill(ThyL+ThyR);

  //--------------------------------------------------------------------------------
  // process HF data

  /*
  int nHFhad=0;
  int nHFemc=0;

  //double thresHFemc=2.;
  //double thresHFhad=5.5;

  double thresHFemc=5.;
  double thresHFhad=6.5;

  int nmuPF=0;

  for (PFCandidateCollection::const_iterator iPFCand = pfs->begin() ; iPFCand != pfs->end() ; ++iPFCand)
  {
    int PFtype = iPFCand->particleId();
    double PFene = iPFCand->energy();
    double PFeta = iPFCand->eta();
    double PFphi = iPFCand->phi();

    // id=3
    if (PFtype == reco::PFCandidate::mu) nmuPF++;

    if (PFtype == reco::PFCandidate::h_HF) // pfOut=6
    {
      histosTH1F["heHFhad"]->Fill(PFene);
      if(PFene > thresHFhad) nHFhad++;

      if(PFene > thresHFhad && PFeta<0) histosTH2F["hHF_Mhad_2DIM"]->Fill(PFeta,PFphi);
      if(PFene > thresHFhad && PFeta>0) histosTH2F["hHF_Phad_2DIM"]->Fill(PFeta,PFphi);

      if(PFene > thresHFhad && PFeta<0) histosTH2F["heHF_Mhad_2DIM"]->Fill(PFeta,PFene);
      if(PFene > thresHFhad && PFeta>0) histosTH2F["heHF_Phad_2DIM"]->Fill(PFeta,PFene);

    } else if(PFtype == reco::PFCandidate::egamma_HF) // pfOut=7
    {
      histosTH1F["heHFemc"]->Fill(PFene);
      if(PFene > thresHFemc) nHFemc++;

      if(PFene > thresHFemc && PFeta<0) histosTH2F["hHF_Memc_2DIM"]->Fill(PFeta,PFphi);
      if(PFene > thresHFemc && PFeta>0) histosTH2F["hHF_Pemc_2DIM"]->Fill(PFeta,PFphi);

      if(PFene > thresHFemc && PFeta<0) histosTH2F["heHF_Memc_2DIM"]->Fill(PFeta,PFene);
      if(PFene > thresHFemc && PFeta>0) histosTH2F["heHF_Pemc_2DIM"]->Fill(PFeta,PFene);

    }
  }

  histosTH1F["hnHF"]->Fill(nHFhad+nHFemc);
  histosTH1F["hnHFhad"]->Fill(nHFhad);
  histosTH1F["hnHFemc"]->Fill(nHFemc);

  //if(nHFhad+nHFemc > 1 ) return;
  */

  //--------------------------------------------------------------------------------
  // CMS-TOTEM matching

  double TOTEMpy =  6500.*(ThyL+ThyR);
  double TOTEMpx = -6500.*(ThxL+ThxR);
  double TOTEMphiL = TMath::ATan2(ThyL, ThxL);
  double TOTEMphiR = TMath::ATan2(ThyR, ThxR);

  double TOTEMdphi = TOTEMphiL - TOTEMphiR;
  if (TOTEMdphi < 0) TOTEMdphi = TOTEMdphi + 2.*TMath::Pi(); // from (-2pi,2pi) to (0,2pi)
  if (TOTEMdphi > TMath::Pi()) TOTEMdphi = 2.*TMath::Pi() - TOTEMdphi; // from (0,2pi) to (0,pi)

  //double CMSpx=pipiRec.Px();
  //double CMSpy=pipiRec.Py();

  //--------------------------------------------------------------------------------
  // plots for 2 tracks sample

  /*
  if (ntrk==2 && totcharge==0 && ntrkvtx==2)
  {
    bool CTpxcut = TMath::Abs(CMSpx+TOTEMpx)<0.15;
    bool CTpycut = TMath::Abs(CMSpy+TOTEMpy)<0.06;

    bool CTpycutBKG = TMath::Abs(CMSpy+TOTEMpy)>0.06 && TMath::Abs(CMSpy+TOTEMpy)<0.3;

    bool allCuts = CTpxcut && CTpycut;

    histosTH1F["hdpy2trk"]->Fill(CMSpy+TOTEMpy);
    histosTH1F["hdpx2trk"]->Fill(CMSpx+TOTEMpx);
    if(CTpxcut) histosTH1F["hdpy2trkB"]->Fill(CMSpy+TOTEMpy);
    if(CTpycut) histosTH1F["hdpx2trkB"]->Fill(CMSpx+TOTEMpx);

    histosTH2F["h2DIMdpy2trk"]->Fill(CMSpy,TOTEMpy);
    histosTH2F["h2DIMdpx2trk"]->Fill(CMSpx,TOTEMpx);

    histosTH1F["hLS2"]->Fill(LS);
    if(TMath::Abs(TOTEMpy)<0.1 && TMath::Abs(CMSpy)>0.1) histosTH1F["hLS2ela"]->Fill(LS);

    // Mass 2 tracks
    double mrec = pipiRec.M();

    if (allCuts)
    {
      int nKaons=0;
      int nPions=0;

      int itref2=0;
      for (TrackCollection::const_iterator itTrack = tracks->begin();itTrack != tracks->end();++itTrack)
      {
        int npixelhits = itTrack->hitPattern().numberOfValidPixelHits();
        reco::TrackRef trackRef = reco::TrackRef(tracks, itref2);
        double thisdedxPIX = (*dedxPIXs)[trackRef].dEdx(); //dedxs.get(trackRef.key()).dEdx();

        itref2++;

        if(npixelhits>0)
        {
          histosTH2F["hdedx2trk"]->Fill(itTrack->p(),thisdedxPIX);
          histosTH2F["hdedx2trklog"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

          if( isKaonCurve(itTrack->p(),thisdedxPIX) ) nKaons++;
          if( isPionCurve(itTrack->p(),thisdedxPIX) ) nPions++;

          if(is2PIsample) histosTH2F["hdedx2pionslog"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));
          if(is2Ksample)  histosTH2F["hdedx2kaonslog"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

          if(is2PIsample2) histosTH2F["hdedx2pionslog35"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));
          if(is2Ksample2)  histosTH2F["hdedx2kaonslog35"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

          if(is2PIsample3) histosTH2F["hdedx2pionslog40"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));
          if(is2Ksample3)  histosTH2F["hdedx2kaonslog40"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

          if(is2PIsample4) histosTH2F["hdedx2pionslog45"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));
          if(is2Ksample4)  histosTH2F["hdedx2kaonslog45"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

          if(is2PIsample5) histosTH2F["hdedx2pionslog50"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));
          if(is2Ksample5)  histosTH2F["hdedx2kaonslog50"]->Fill(itTrack->p(),TMath::Log10(thisdedxPIX));

        }
      }

      histosTH1F["hntrk02"]->Fill(ntrk0);
      histosTH1F["hntrkvtx2"]->Fill(ntrkvtx);

      histosTH1F["hm2"]->Fill(mrec);

      histosTH1F["hdphi2"]->Fill(TOTEMdphi);

      histosTH1F["hpt2"]->Fill(pi4pos1.Pt());
      histosTH1F["hpt2"]->Fill(pi4neg1.Pt());

      histosTH1F["heta2"]->Fill(pi4pos1.Eta());
      histosTH1F["heta2"]->Fill(pi4neg1.Eta());
    }

    if(CTpycutBKG)
    {
      histosTH1F["hm2BKG"]->Fill(mrec);

      histosTH1F["hpt2BKG"]->Fill(pi1.Pt());
      histosTH1F["hpt2BKG"]->Fill(pi2.Pt());

      histosTH1F["heta2BKG"]->Fill(pi1.Eta());
      histosTH1F["heta2BKG"]->Fill(pi2.Eta());
    }
  }
  */

  //--------------------------------------------------------------------------------
  // plots for 4 tracks sample

  const double CMSpx4 = pi4Rec.Px();
  const double CMSpy4 = pi4Rec.Py();

  if (ntrk == 4 && totcharge == 0 && ntrkvtx == 4)
  {
    bool CTpxcut4 = TMath::Abs(CMSpx4+TOTEMpx) < 0.13;
    bool CTpycut4 = TMath::Abs(CMSpy4+TOTEMpy) < 0.06;

    bool CTpycut4BKG = fabs(CMSpy4+TOTEMpy) > 0.06 && fabs(CMSpy4+TOTEMpy) < 0.3;

    bool allCuts4 = CTpxcut4 && CTpycut4;

    histosTH1F["hdpy4trk"]->Fill(CMSpy4+TOTEMpy);
    histosTH1F["hdpx4trk"]->Fill(CMSpx4+TOTEMpx);
    if(CTpxcut4) histosTH1F["hdpy4trkB"]->Fill(CMSpy4+TOTEMpy);
    if(CTpycut4) histosTH1F["hdpx4trkB"]->Fill(CMSpx4+TOTEMpx);

    histosTH2F["h2DIMdpy4trk"]->Fill(CMSpy4, TOTEMpy);
    histosTH2F["h2DIMdpx4trk"]->Fill(CMSpx4, TOTEMpx);

    // TODO
    const double mrec4 = pi4Rec.M();

    if (allCuts4)
    {
      returnStatus = true;

      int nPions=0, nSafePions=0;
      int nKaons=0, nKaonsPos=0, nKaonsNeg=0;
      int nProtons=0;

      struct DEDxInfo
      {
        int nPixelHits;
        double p;
        double dEdx;
      };

      vector<DEDxInfo> dEdxInfo;

      int itref4=0;

      bool etaCutSatisfied = true;
      bool allTracksHavePixelHit = true;

      for (TrackCollection::const_iterator itTrack = tracks->begin(); itTrack != tracks->end(); ++itTrack)
      {
        int npixelhits = itTrack->hitPattern().numberOfValidPixelHits();
        reco::TrackRef trackRef = reco::TrackRef(tracks, itref4);
        double thisdedxPIX = (*dedxPIXs)[trackRef].dEdx();

        itref4++;

        dEdxInfo.push_back({npixelhits, itTrack->p(), thisdedxPIX});

        if (fabs(itTrack->eta()) > 2.5)
          etaCutSatisfied = false;

        if (npixelhits == 0)
          allTracksHavePixelHit = false;

        if (npixelhits > 0)
        {
          histosTH2F["hdedx4trk"]->Fill(itTrack->p(), thisdedxPIX);
          histosTH2F["hdedx4trklog"]->Fill(itTrack->p(), TMath::Log10(thisdedxPIX));

          if (isPionCurve(itTrack->p(), thisdedxPIX))
          {
            nPions++;

            histosTH2F["hdedx4trk_PionCurve"]->Fill(itTrack->p(), thisdedxPIX);
          }

          if (isSafePionCurve(itTrack->p(), thisdedxPIX))
          {
            nSafePions++;

            histosTH2F["hdedx4trk_SafePionCurve"]->Fill(itTrack->p(), thisdedxPIX);
          }

          if (isKaonCurve(itTrack->p(), thisdedxPIX))
          {
            nKaons++;

            histosTH2F["hdedx4trk_KaonCurve"]->Fill(itTrack->p(), thisdedxPIX);

            if (itTrack->charge() > 0)
              nKaonsPos++;
            if (itTrack->charge() < 0)
              nKaonsNeg++;
          }

          if (isProtonCurve(itTrack->p(), thisdedxPIX))
          {
            nProtons++;

            histosTH2F["hdedx4trk_ProtonCurve"]->Fill(itTrack->p(), thisdedxPIX);
          }
        }
      }

      histosTH1F["hntrk04"]->Fill(ntrk0);
      histosTH1F["hntrkvtx4"]->Fill(ntrkvtx);

      histosTH1F["hm4"]->Fill(mrec4);

      histosTH1F["hdphi4"]->Fill(TOTEMdphi);

      histosTH1F["hpt4"]->Fill(pi4pos1.Pt());
      histosTH1F["hpt4"]->Fill(pi4neg1.Pt());
      histosTH1F["hpt4"]->Fill(pi4pos2.Pt());
      histosTH1F["hpt4"]->Fill(pi4neg2.Pt());

      histosTH1F["heta4"]->Fill(pi4pos1.Eta());
      histosTH1F["heta4"]->Fill(pi4neg1.Eta());
      histosTH1F["heta4"]->Fill(pi4pos2.Eta());
      histosTH1F["heta4"]->Fill(pi4neg2.Eta());

      if (!etaCutSatisfied)
        return returnStatus;

      if (!allTracksHavePixelHit)
        return returnStatus;

      //------------------------
      // rho-rho

      const TLorentzVector pi4m11 = pi4pos1 + pi4neg1;
      const TLorentzVector pi4m22 = pi4pos2 + pi4neg2;

      const TLorentzVector pi4m12 = pi4pos1 + pi4neg2;
      const TLorentzVector pi4m21 = pi4pos2 + pi4neg1;

      const double m11 = pi4m11.M();
      const double m22 = pi4m22.M();

      const double m12 = pi4m12.M();
      const double m21 = pi4m21.M();

      const double rhoM = 0.77;
      const double rhoCut = 0.124;
      const double rhoCut2 = rhoCut/2.;
      const double rhoCut4 = rhoCut/4.;

      histosTH1F["hmALLpiKen0"]->Fill(m11);
      histosTH1F["hmALLpiKen0"]->Fill(m12);

      histosTH2F["hmALLpiKen0_2D"]->Fill(m11, m22);
      histosTH2F["hmALLpiKen0_2D"]->Fill(m12, m21);

      if (fabs(m11-rhoM) < rhoCut) histosTH1F["hmrhootherKen0"]->Fill(m22);
      if (fabs(m12-rhoM) < rhoCut) histosTH1F["hmrhootherKen0"]->Fill(m21);

      int nOK2mass = 0;
      if (fabs(m11-rhoM) < rhoCut2 && fabs(m22-rhoM) < rhoCut2) nOK2mass++;
      if (fabs(m12-rhoM) < rhoCut2 && fabs(m21-rhoM) < rhoCut2) nOK2mass++;

      int nOK4mass=0;
      if(fabs(m11-rhoM) < rhoCut4 && fabs(m22-rhoM) < rhoCut4) nOK4mass++;
      if(fabs(m12-rhoM) < rhoCut4 && fabs(m21-rhoM) < rhoCut4) nOK4mass++;

      if (nOK2mass>0)
      {
        histosTH1F["hnPionsSIG1"]->Fill(nPions);

        {
          histosTH1F["hm4SIG1mass"]->Fill(mrec4);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4SIG1mass"]->Fill(in.p, in.dEdx);
        }

        if (nPions == 4)
        {
          histosTH1F["hm4SIG1mass4curves"]->Fill(mrec4);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4SIG1mass4curves"]->Fill(in.p, in.dEdx);
        }

        if (nPions >= 3)
        {
          histosTH1F["hm4SIG1mass34curves"]->Fill(mrec4);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4SIG1mass34curves"]->Fill(in.p, in.dEdx);
        }

        if (nPions >= 2)
        {
          histosTH1F["hm4SIG1mass234curves"]->Fill(mrec4);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4SIG1mass234curves"]->Fill(in.p, in.dEdx);
        }

        if (nPions >= 1)
        {
          histosTH1F["hm4SIG1mass1234curves"]->Fill(mrec4);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4SIG1mass1234curves"]->Fill(in.p, in.dEdx);
        }
      }

      if (nOK4mass>0)
      {
        histosTH1F["hm4SIG2mass"]->Fill(mrec4);

        histosTH1F["hnPionsSIG2"]->Fill(nPions);
        if (nPions == 4) histosTH1F["hm4SIG2mass4curves"]->Fill(mrec4);
        if (nPions >= 3) histosTH1F["hm4SIG2mass34curves"]->Fill(mrec4);
        if (nPions >= 2) histosTH1F["hm4SIG2mass234curves"]->Fill(mrec4);
        if (nPions >= 1) histosTH1F["hm4SIG2mass1234curves"]->Fill(mrec4);
      }

      //------------------------
      // phi-phi

      const double m11k = (k4pos1 + k4neg1).M();
      const double m22k = (k4pos2 + k4neg2).M();

      const double m12k = (k4pos1 + k4neg2).M();
      const double m21k = (k4pos2 + k4neg1).M();

      const double m4k = (k4pos1 + k4pos2 + k4neg1 + k4neg2).M();

      const double phiCen = 1.02;
      const double phiCut = 0.02;
      const double phiCutStrict = 0.01;

      histosTH1F["hmALLkKen0"]->Fill(m11k);
      histosTH1F["hmALLkKen0"]->Fill(m12k);

      histosTH2F["hmALLkKen0_2D"]->Fill(m11k, m22k);
      histosTH2F["hmALLkKen0_2D"]->Fill(m12k, m21k);

      if (fabs(m11k - phiCen) < phiCut) histosTH1F["hmphiotherKen0"]->Fill(m22k);
      if (fabs(m12k - phiCen) < phiCut) histosTH1F["hmphiotherKen0"]->Fill(m21k);

      if (fabs(m11k - phiCen) < phiCutStrict) histosTH1F["hmphiotherKen0_PhiCutStrict"]->Fill(m22k);
      if (fabs(m12k - phiCen) < phiCutStrict) histosTH1F["hmphiotherKen0_PhiCutStrict"]->Fill(m21k);

      unsigned int nOKPhiCut = 0;
      if (fabs(m11k - phiCen) < phiCut && fabs(m22k - phiCen) < phiCut) nOKPhiCut++;
      if (fabs(m12k - phiCen) < phiCut && fabs(m21k - phiCen) < phiCut) nOKPhiCut++;

      unsigned int nOKPhiCutStrict = 0;
      if (fabs(m11k - phiCen) < phiCutStrict && fabs(m22k - phiCen) < phiCutStrict) nOKPhiCutStrict++;
      if (fabs(m12k - phiCen) < phiCutStrict && fabs(m21k - phiCen) < phiCutStrict) nOKPhiCutStrict++;

      bool twoKaonsOfTheSameSign = (nKaonsPos == 2 || nKaonsNeg == 2);

      if (nOKPhiCut > 0)
      {
        histosTH1F["hnKaons"]->Fill(nKaons);

        {
          histosTH1F["hm4PHImass"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass"]->Fill(in.p, in.dEdx);
        }

        if (nKaons == 4)
        {
          histosTH1F["hm4PHImass4curves"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass4curves"]->Fill(in.p, in.dEdx);
        }

        if (nKaons == 3)
        {
          histosTH1F["hm4PHImass3curves"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass3curves"]->Fill(in.p, in.dEdx);
        }

        if (nKaons >= 3)
        {
          histosTH1F["hm4PHImass34curves"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass34curves"]->Fill(in.p, in.dEdx);
        }

        if (nKaons >= 2)
        {
          histosTH1F["hm4PHImass234curves"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass234curves"]->Fill(in.p, in.dEdx);
        }

        if (nKaons >= 1)
        {
          histosTH1F["hm4PHImass1234curves"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass1234curves"]->Fill(in.p, in.dEdx);
        }

        if (twoKaonsOfTheSameSign)
        {
          histosTH1F["hm4PHImass2SS"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass2SS"]->Fill(in.p, in.dEdx);
        }

        if (nPions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImassPionProtonVeto"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImassPionProtonVeto"]->Fill(in.p, in.dEdx);
        }

        if (nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImassSafePionProtonVeto"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImassSafePionProtonVeto"]->Fill(in.p, in.dEdx);
        }

        if (nKaons >= 1 && nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImass1234curvesSafePionProtonVeto"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass1234curvesSafePionProtonVeto"]->Fill(in.p, in.dEdx);
        }

        if (twoKaonsOfTheSameSign && nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImass2SSSafePionProtonVeto"]->Fill(m4k);
          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass2SSSafePionProtonVeto"]->Fill(in.p, in.dEdx);
        }
      }

      if (nOKPhiCutStrict > 0)
      {
        histosTH1F["hnKaons_PhiCutStrict"]->Fill(nKaons);

        histosTH1F["hm4PHImass_PhiCutStrict"]->Fill(m4k);

        if (nKaons == 4) histosTH1F["hm4PHImass4curves_PhiCutStrict"]->Fill(m4k);
        if (nKaons == 3) histosTH1F["hm4PHImass3curves_PhiCutStrict"]->Fill(m4k);

        if (nKaons >= 3) histosTH1F["hm4PHImass34curves_PhiCutStrict"]->Fill(m4k);
        if (nKaons >= 2) histosTH1F["hm4PHImass234curves_PhiCutStrict"]->Fill(m4k);
        if (nKaons >= 1) histosTH1F["hm4PHImass1234curves_PhiCutStrict"]->Fill(m4k);

        if (twoKaonsOfTheSameSign)
        {
          histosTH1F["hm4PHImass2SS_PhiCutStrict"]->Fill(m4k);
        }

        if (nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImassSafePionProtonVeto_PhiCutStrict"]->Fill(m4k);
        }

        if (nKaons >= 1 && nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImass1234curvesSafePionProtonVeto_PhiCutStrict"]->Fill(m4k);
        }

        if (twoKaonsOfTheSameSign && nSafePions == 0 && nProtons == 0)
        {
          histosTH1F["hm4PHImass2SSSafePionProtonVeto_PhiCutStrict"]->Fill(m4k);
        }
      }

      // begin: test code
      if (nOKPhiCut > 0 && m4k > 2.210 && m4k < 2.225)
      {
          histosTH1F["h_m_4K_PEAK"]->Fill(m4k);
          histosTH1F["h_m_4pi_PEAK"]->Fill(mrec4);

          histosTH2F["hmALLpiKen0_2D_PEAK"]->Fill(m11, m22);
          histosTH2F["hmALLpiKen0_2D_PEAK"]->Fill(m12, m21);

          for (const auto &in : dEdxInfo)
            histosTH2F["hdedx4PHImass_PEAK"]->Fill(in.p, in.dEdx);
      }
      // end: test code

      //------------------------------------------
      // K*-K*

      double kstarCut=0.042;

      TLorentzVector KpPI2Rec0(0.,0.,0.,0.);
      TLorentzVector KmPI2Rec0(0.,0.,0.,0.);
      TLorentzVector KPI4Rec0(0.,0.,0.,0.);

      double mKpPI0=0;
      double mKmPI0=0;
      double mKPI40=0;

      int nOKpidKs=0;

      bool passcurves = false;

      // 2combinations for phi phi, but 4 for KsKs
      //    pos1pos2  +K +pi
      //    neg1neg2  -K -pi

      KpPI2Rec0 =  k4pos1 + pi4neg2;
      KmPI2Rec0 = pi4pos2 +  k4neg1;
      mKpPI0 = KpPI2Rec0.M();
      mKmPI0 = KmPI2Rec0.M();

      KPI4Rec0 = KpPI2Rec0 + KmPI2Rec0;
      mKPI40 = KPI4Rec0.M();

      if(iskaon4pos[0] && ispion4neg[1] && ispion4pos[1] && iskaon4neg[0]) passcurves = true;

      if(fabs(mKpPI0-0.892)<kstarCut && fabs(mKmPI0-0.892)<kstarCut)  nOKpidKs++;

      KpPI2Rec0 =  k4pos1 + pi4neg1;
      KmPI2Rec0 = pi4pos2 +  k4neg2;
      mKpPI0 = KpPI2Rec0.M();
      mKmPI0 = KmPI2Rec0.M();

      KPI4Rec0 = KpPI2Rec0 + KmPI2Rec0;
      mKPI40 = KPI4Rec0.M();

      if(iskaon4pos[0] && ispion4neg[0] && ispion4pos[1] && iskaon4neg[1]) passcurves = true;

      if(fabs(mKpPI0-0.892)<kstarCut && fabs(mKmPI0-0.892)<kstarCut)  nOKpidKs++;

      KpPI2Rec0 =  k4pos2 + pi4neg1;
      KmPI2Rec0 = pi4pos1 +  k4neg2;
      mKpPI0 = KpPI2Rec0.M();
      mKmPI0 = KmPI2Rec0.M();

      KPI4Rec0 = KpPI2Rec0 + KmPI2Rec0;
      mKPI40 = KPI4Rec0.M();

      if(iskaon4pos[1] && ispion4neg[0] && ispion4pos[0] && iskaon4neg[1]) passcurves = true;

      if(fabs(mKpPI0-0.892)<kstarCut && fabs(mKmPI0-0.892)<kstarCut)  nOKpidKs++;

      KpPI2Rec0 =  k4pos2 + pi4neg2;
      KmPI2Rec0 = pi4pos1 +  k4neg1;
      mKpPI0 = KpPI2Rec0.M();
      mKmPI0 = KmPI2Rec0.M();

      KPI4Rec0=KpPI2Rec0+KmPI2Rec0;
      mKPI40=KPI4Rec0.M();

      if(iskaon4pos[1] && ispion4neg[1] && ispion4pos[0] && iskaon4neg[0]) passcurves = true;

      if(fabs(mKpPI0-0.892)<kstarCut && fabs(mKmPI0-0.892)<kstarCut)  nOKpidKs++;

      histosTH1F["hmALLkpi0"]->Fill(mKpPI0);
      if(fabs(mKpPI0-0.892)<kstarCut) histosTH1F["hmKSother0"]->Fill(mKmPI0);

      if (passcurves)
      {
        histosTH1F["hmALLkpi0curv"]->Fill(mKpPI0);
        if(fabs(mKpPI0-0.892)<kstarCut) histosTH1F["hmKSother0curv"]->Fill(mKmPI0);
      }

      if(nOKpidKs>0)
      {
        histosTH1F["hnOKpidKs"]->Fill(nOKpidKs);
        histosTH1F["hm4KST0"]->Fill(mKPI40);

        if(passcurves)
        {
          histosTH1F["hnOKpidKscurv"]->Fill(nOKpidKs);
          histosTH1F["hm4KST0curv"]->Fill(mKPI40);
        }
      }
    }

    if (CTpycut4BKG)
    {
      histosTH1F["hm4BKG"]->Fill(mrec4);

      histosTH1F["hpt4BKG"]->Fill(pi4pos1.Pt());
      histosTH1F["hpt4BKG"]->Fill(pi4neg1.Pt());
      histosTH1F["hpt4BKG"]->Fill(pi4pos2.Pt());
      histosTH1F["hpt4BKG"]->Fill(pi4neg2.Pt());

      histosTH1F["heta4BKG"]->Fill(pi4pos1.Eta());
      histosTH1F["heta4BKG"]->Fill(pi4neg1.Eta());
      histosTH1F["heta4BKG"]->Fill(pi4pos2.Eta());
      histosTH1F["heta4BKG"]->Fill(pi4neg2.Eta());
    }
  }

  return returnStatus;
}

//----------------------------------------------------------------------------------------------------

DEFINE_FWK_MODULE(PromptAnalyzer);
