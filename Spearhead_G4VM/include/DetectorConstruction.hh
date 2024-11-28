#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh

#include "G4VUserDetectorConstruction.hh"
#include "G4UImessenger.hh"
#include "G4Cache.hh"

class G4VSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4NistManager;
class G4GDMLParser;
class G4UIcmdWithAString;


class DetectorConstruction : public G4VUserDetectorConstruction, public G4UImessenger {
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

	void ConstructSDandField();

	G4VPhysicalVolume* Construct();

	void SetNewValue(G4UIcommand *cmd, G4String val);
	G4String GetNewValue(G4UIcommand *cmd);

private:
	void InitMessenger();

	G4VPhysicalVolume* GDMLGetWorldVolume(G4String const &name);

	G4GDMLParser *parser;
	G4UIcmdWithAString *gdmlFileCmd;
	G4String gdmlFile;
};

#endif
