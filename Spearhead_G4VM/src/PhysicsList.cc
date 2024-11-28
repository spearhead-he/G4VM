#include "PhysicsList.hh"

#include "G4PhysListFactory.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4NuclideTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4EmParameters.hh"
#include "G4ParticleHPManager.hh"

PhysicsList::PhysicsList() {
	G4PhysListFactory factory;
	physlist = factory.GetReferencePhysList("QGSP_BERT_HP_LIV");

#ifndef NO_DECAY_PHYSICS
	// Mandatory for G4NuclideTable
	// Half-life threshold must be set small or many short-lived isomers
	// will not be assigned life times (default to 0)
	// G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.1*picosecond);
	// G4NuclideTable::GetInstance()->SetLevelTolerance(1.0*eV);

	// physlist->RegisterPhysics(new G4DecayPhysics());
	// physlist->RegisterPhysics(new G4RadioactiveDecayPhysics());
#endif

#ifndef NO_INNER_EM
	// G4EmParameters* param = G4EmParameters::Instance();
	// param->SetAuger(true);
	// param->SetFluo(true);
	// param->SetAugerCascade(true);
	// param->SetPixe(true);
#endif

#ifdef PHOTON_EVAPORATION
	// G4ParticleHPManager::GetInstance()->SetUseOnlyPhotoEvaporation(true);
#endif

#ifdef OPTICAL_PHYSICS
	// G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
	
	// opticalPhysics->SetVerboseLevel(1);
	
	// opticalPhysics->SetWLSTimeProfile("delta");
	// opticalPhysics->SetScintillationYieldFactor(1.0);
	// opticalPhysics->SetScintillationExcitationRatio(0.0);
	
	// opticalPhysics->SetMaxNumPhotonsPerStep(100);
	// opticalPhysics->SetMaxBetaChangePerStep(10.0);
		
	// opticalPhysics->SetTrackSecondariesFirst(true);
	
	// physlist->RegisterPhysics(opticalPhysics);
#endif
}

PhysicsList::~PhysicsList() {
	delete physlist;
}

G4VModularPhysicsList* PhysicsList::GetPhysicsList() const {
	return physlist;
}

