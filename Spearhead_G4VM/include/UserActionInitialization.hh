#ifndef UserActionInitialization_hh
#define UserActionInitialization_hh

#include "G4VUserActionInitialization.hh"

class UserActionInitialization : public G4VUserActionInitialization
{
public:
    UserActionInitialization();
    virtual ~UserActionInitialization();

    void Build() const;
	void BuildForMaster() const;
};

#endif
