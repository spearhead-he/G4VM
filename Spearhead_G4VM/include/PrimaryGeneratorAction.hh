#include "G4VUserPrimaryGeneratorAction.hh"

class G4GeneralParticleSource;
class G4Event;
 
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    virtual~PrimaryGeneratorAction();

    void GeneratePrimaries(G4Event*);

private:
	G4GeneralParticleSource*	ParticleGenerator;
};

