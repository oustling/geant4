//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4HadronPhysicsFTFP_BERT.cc 105736 2017-08-16 13:01:11Z gcosmo $
//
//---------------------------------------------------------------------------
//
// ClassName:   
//
// Author: 2007 Gunter Folger
//   created from G4HadronPhysicsFTFP
//
// Modified:
//
//----------------------------------------------------------------------------
//
#include <iomanip>   

#include "G4HadronPhysicsFTFP_BERT.hh"

#include "globals.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4PionBuilder.hh"
#include "G4BertiniPionBuilder.hh"
#include "G4FTFPPionBuilder.hh"

#include "G4KaonBuilder.hh"
#include "G4BertiniKaonBuilder.hh"
#include "G4FTFPKaonBuilder.hh"

#include "G4ProtonBuilder.hh"
#include "G4BertiniProtonBuilder.hh"
#include "G4FTFPNeutronBuilder.hh"
#include "G4FTFPProtonBuilder.hh"

#include "G4NeutronBuilder.hh"
#include "G4BertiniNeutronBuilder.hh"
#include "G4FTFPNeutronBuilder.hh"

#include "G4HyperonFTFPBuilder.hh"
#include "G4AntiBarionBuilder.hh"
#include "G4FTFPAntiBarionBuilder.hh"

#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4ComponentGGHadronNucleusXsc.hh"
#include "G4CrossSectionInelastic.hh"
#include "G4HadronCaptureProcess.hh"
#include "G4NeutronRadCapture.hh"
#include "G4NeutronInelasticXS.hh"
#include "G4NeutronCaptureXS.hh"

#include "G4CrossSectionDataSetRegistry.hh"

#include "G4PhysListUtil.hh"
#include "G4Threading.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4HadronPhysicsFTFP_BERT);


G4HadronPhysicsFTFP_BERT::G4HadronPhysicsFTFP_BERT(G4int) :
    G4HadronPhysicsFTFP_BERT("hInelastic FTFP_BERT",false) {}

G4HadronPhysicsFTFP_BERT::G4HadronPhysicsFTFP_BERT(const G4String& name, G4bool quasiElastic)
    :  G4VPhysicsConstructor(name) 
    , QuasiElastic(quasiElastic)
{
  minFTFP_pion = 3.0 * GeV;
  maxBERT_pion = 12.0 * GeV;
  minFTFP_kaon = 3.0 * GeV;
  maxBERT_kaon = 12.0 * GeV;
  minFTFP_proton = 3.0 * GeV;
  maxBERT_proton = 12.0 * GeV;
  minFTFP_neutron = 3.0 * GeV;
  maxBERT_neutron = 12.0 * GeV;
}

G4HadronPhysicsFTFP_BERT::~G4HadronPhysicsFTFP_BERT()
{
  //Detele master-owned stuff
  delete xs_k.Get();
  std::for_each( xs_ds.Begin(), xs_ds.End(),[](G4VCrossSectionDataSet* el){ delete el;});
}

void G4HadronPhysicsFTFP_BERT::ConstructParticle()
{
  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();
}

void G4HadronPhysicsFTFP_BERT::TerminateWorker()
{
  delete xs_k.Get();
  std::for_each( xs_ds.Begin(), xs_ds.End(),[](G4VCrossSectionDataSet* el){ delete el;});
  xs_ds.Clear();
  G4VPhysicsConstructor::TerminateWorker();
}

void G4HadronPhysicsFTFP_BERT::DumpBanner()
{
  G4cout << G4endl
       << " FTFP_BERT : new threshold between BERT and FTFP is over the interval " << G4endl
       << " for pions :   " << minFTFP_pion/GeV << " to " << maxBERT_pion/GeV  << " GeV" << G4endl
       << " for kaons :   " << minFTFP_kaon/GeV << " to " << maxBERT_kaon/GeV  << " GeV" << G4endl
       << " for proton :  " << minFTFP_proton/GeV << " to " << maxBERT_proton/GeV  << " GeV" << G4endl
       << " for neutron : " << minFTFP_neutron/GeV << " to " << maxBERT_neutron/GeV  << " GeV" << G4endl
       << G4endl;
}

void G4HadronPhysicsFTFP_BERT::CreateModels()
{
  Neutron();
  Proton();
  Pion();
  Kaon();
  Others();
}

void G4HadronPhysicsFTFP_BERT::Neutron()
{
  //General schema:
  // 1) Create a builder
  // 2) Call AddBuilder
  // 3) Configure the builder, possibly with sub-builders
  // 4) Call builder->Build()
  auto neu = new G4NeutronBuilder;
  AddBuilder(neu);
  auto ftfpn = new G4FTFPNeutronBuilder(QuasiElastic);
  AddBuilder( ftfpn );
  neu->RegisterMe(ftfpn);
  ftfpn->SetMinEnergy(minFTFP_neutron);
  auto bertn = new G4BertiniNeutronBuilder;
  AddBuilder(bertn);
  neu->RegisterMe(bertn);
  bertn->SetMinEnergy(0.*GeV);
  bertn->SetMaxEnergy(maxBERT_neutron);
  neu->Build();
}

void G4HadronPhysicsFTFP_BERT::Proton()
{
  auto pro = new G4ProtonBuilder;
  AddBuilder(pro);
  auto ftfpp = new G4FTFPProtonBuilder(QuasiElastic);
  AddBuilder(ftfpp);
  pro->RegisterMe(ftfpp);
  ftfpp->SetMinEnergy(minFTFP_proton);
  auto bertp = new G4BertiniProtonBuilder;
  AddBuilder(bertp);
  pro->RegisterMe(bertp);
  bertp->SetMaxEnergy(maxBERT_proton);
  pro->Build();
}

void G4HadronPhysicsFTFP_BERT::Pion()
{
  auto pi = new G4PionBuilder;
  AddBuilder(pi);
  auto ftfppi = new G4FTFPPionBuilder(QuasiElastic);
  AddBuilder(ftfppi);
  pi->RegisterMe(ftfppi);
  ftfppi->SetMinEnergy(minFTFP_pion);
  auto bertpi = new G4BertiniPionBuilder;
  AddBuilder(bertpi);
  pi->RegisterMe(bertpi);
  bertpi->SetMaxEnergy(maxBERT_pion);
  pi->Build();
}

void G4HadronPhysicsFTFP_BERT::Kaon()
{
  auto k = new G4KaonBuilder;
  AddBuilder(k);
  auto ftfpk = new G4FTFPKaonBuilder(QuasiElastic);
  AddBuilder(ftfpk);
  k->RegisterMe(ftfpk);
  ftfpk->SetMinEnergy(minFTFP_kaon);
  auto bertk  = new G4BertiniKaonBuilder;
  AddBuilder(bertk);
  k->RegisterMe(bertk);
  bertk->SetMaxEnergy(maxBERT_kaon);
  k->Build();
}

void G4HadronPhysicsFTFP_BERT::Others()
{
  //===== Hyperons ====== //
  auto hyp = new G4HyperonFTFPBuilder;
  AddBuilder( hyp );
  hyp->Build();

  ///===== Anti-barions==== //
  auto abar = new G4AntiBarionBuilder;
  AddBuilder(abar);
  auto ftfpabar = new G4FTFPAntiBarionBuilder(QuasiElastic);
  AddBuilder(ftfpabar);
  abar->RegisterMe(ftfpabar);
  abar->Build();
}

void G4HadronPhysicsFTFP_BERT::ConstructProcess()
{
  if(G4Threading::IsMasterThread()) {
      DumpBanner();
  }
  CreateModels();
  ExtraConfiguration();
}

#include "G4ProcessManager.hh"
void G4HadronPhysicsFTFP_BERT::ExtraConfiguration()
{
  //Modify XS for kaons
  auto xsk = new G4ComponentGGHadronNucleusXsc();
  xs_k.Put(xsk);
  G4VCrossSectionDataSet * kaonxs = new G4CrossSectionInelastic(xsk);
  xs_ds.Push_back(kaonxs);
  G4PhysListUtil::FindInelasticProcess(G4KaonMinus::KaonMinus())->AddDataSet(kaonxs);
  G4PhysListUtil::FindInelasticProcess(G4KaonPlus::KaonPlus())->AddDataSet(kaonxs);
  G4PhysListUtil::FindInelasticProcess(G4KaonZeroShort::KaonZeroShort())->AddDataSet(kaonxs);
  G4PhysListUtil::FindInelasticProcess(G4KaonZeroLong::KaonZeroLong())->AddDataSet(kaonxs);

  //Modify Neutrons
  auto xs_n_in = (G4NeutronInelasticXS*)G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4NeutronInelasticXS::Default_Name());
  xs_ds.Push_back(xs_n_in);//TODO: Is this needed? Who owns the pointer?
  G4PhysListUtil::FindInelasticProcess(G4Neutron::Neutron())->AddDataSet( xs_n_in );
  G4HadronicProcess* capture = 0;
  G4ProcessManager* pmanager = G4Neutron::Neutron()->GetProcessManager();
  G4ProcessVector*  pv = pmanager->GetProcessList();
  for ( size_t i=0; i < static_cast<size_t>(pv->size()); ++i ) {
    if ( fCapture == ((*pv)[i])->GetProcessSubType() ) {
      capture = static_cast<G4HadronicProcess*>((*pv)[i]);
    }
  }
  if ( ! capture ) {
    capture = new G4HadronCaptureProcess("nCapture");
    pmanager->AddDiscreteProcess(capture);
  }
  auto xs_n_c = (G4NeutronCaptureXS*)G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4NeutronCaptureXS::Default_Name());
  xs_ds.Push_back(xs_n_c);
  capture->AddDataSet( xs_n_c );
  capture->RegisterMe( new G4NeutronRadCapture() );
}
