#ifndef PhysicsList_hh
#define PhysicsList_hh

#include "G4VModularPhysicsList.hh"

class PhysicsList {
public:
    PhysicsList();
    virtual ~PhysicsList();

	G4VModularPhysicsList* GetPhysicsList() const;

private:
	G4VModularPhysicsList *physlist;
};

#endif
