#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "DetectorConstruction.hh"
#include "UserActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"

int main(int argc,char** argv)
{
	G4UIExecutive* ui = 0;
	if ( argc == 1 ) {
		ui = new G4UIExecutive(argc, argv);
	}

#ifdef G4MULTITHREADED
	G4MTRunManager* runManager = new G4MTRunManager;
#else
	G4RunManager *runManager = new G4RunManager;
#endif

	DetectorConstruction* detector = new DetectorConstruction;
	runManager->SetUserInitialization(detector);

	PhysicsList* physlist = new PhysicsList();
	runManager->SetUserInitialization(physlist->GetPhysicsList());

#ifdef G4MULTITHREADED
	UserActionInitialization *userActionInit = new UserActionInitialization;
	runManager->SetUserInitialization(userActionInit);
#else
	PrimaryGeneratorAction *pga = new PrimaryGeneratorAction;
	runManager->SetUserAction(pga);
#endif
	
	G4String tempString;
	G4String tempArgv;
	G4VisManager* visManager			= new G4VisExecutive;
	visManager->Initialize();
	
	G4UImanager * UImanager				= G4UImanager::GetUIpointer();
	
	if(argc>1)
	{
		tempString="/control/execute ";
		tempString+=argv[1];
		UImanager->ApplyCommand(tempString);
	}
	else { 
		// interactive mode
		UImanager->ApplyCommand("/control/execute run/vis.mac");
		ui->SessionStart();
	}
	delete ui;
	delete visManager;
	delete runManager;
	
	return 0;
}
