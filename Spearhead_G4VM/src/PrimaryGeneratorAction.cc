#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
	ParticleGenerator	= new G4GeneralParticleSource;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
	delete ParticleGenerator;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	ParticleGenerator->GeneratePrimaryVertex(anEvent);
}

