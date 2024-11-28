#include "DetectorConstruction.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "sensi.hh"
#include "G4SDManager.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4GDMLParser.hh"

#include "G4UIcmdWithAString.hh"

DetectorConstruction::DetectorConstruction() : gdmlFile() {
	parser = new G4GDMLParser;
	InitMessenger();
}

DetectorConstruction::~DetectorConstruction() {
	;
}

G4VPhysicalVolume* DetectorConstruction::GDMLGetWorldVolume(G4String const &name)
{
	parser->SetStripFlag(false);
	parser->Read(name);
	return parser->GetWorldVolume();
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
	return GDMLGetWorldVolume(this->gdmlFile);
}

void DetectorConstruction::ConstructSDandField()
{
	G4LogicalVolumeStore *lvs = G4LogicalVolumeStore::GetInstance();
	G4SDManager *sdm = G4SDManager::GetSDMpointer();

	for(auto it = lvs->begin(); it != lvs->end(); it++) {

		G4GDMLAuxListType auxInfo = parser->GetVolumeAuxiliaryInformation(*it);

		for(auto jt = auxInfo.begin(); jt != auxInfo.end(); jt++) {

			G4String type = jt->type;
			if(type != "SD") continue;

			G4String value = jt->value;
			G4String unit = jt->unit;

			sensi *sd = static_cast<sensi*>(sdm->FindSensitiveDetector(value));
			if(!sd) {
				sd = new sensi(value);
				sdm->AddNewDetector(sd);
			}
			(*it)->SetSensitiveDetector(sd);
		}

	}
}

void DetectorConstruction::InitMessenger()
{
	gdmlFileCmd = new G4UIcmdWithAString("/gdml/file", this);
	gdmlFileCmd->SetGuidance("GDML geometry file to load");
	gdmlFileCmd->SetParameterName("Filename", false, false);
	gdmlFileCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

void DetectorConstruction::SetNewValue(G4UIcommand *cmd, G4String val)
{
	if(cmd == gdmlFileCmd) {
		this->gdmlFile = val;
	}
}

G4String DetectorConstruction::GetNewValue(G4UIcommand *cmd)
{
	if(cmd == gdmlFileCmd) {
		return this->gdmlFile;
	}
	return "";
}

