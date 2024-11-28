#include "sensi.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4SystemOfUnits.hh"
#include "CLHEP/Random/Random.h"
#include "G4TrackStatus.hh"
#include "G4VProcess.hh"

// ThreadLocal static variables
G4ThreadLocal G4Allocator<sensiHit>* sensiHit::sensiHitAllocator=0;
G4ThreadLocal std::ofstream sensi::outstream;
G4ThreadLocal std::ofstream sensi::trkoutstream;
G4ThreadLocal std::ofstream sensi::stpoutstream;
G4ThreadLocal G4String sensi::outputFileName;
G4ThreadLocal G4int BeamDump::id=0;
G4ThreadLocal std::vector<std::function<void(sensiHit *hit, std::ostream &out)>> sensi::format;
G4ThreadLocal G4int sensi::gid=0;

// use this macro in the lambdas below
#define outmapout [](sensiHit *hit, std::ostream &out)

OutputMap sensiHit::outputMap = {
	{"x", outmapout { out << (hit->pos).x()/mm; } },
	{"y", outmapout { out << (hit->pos).y()/mm; } },
	{"z", outmapout { out << (hit->pos).z()/mm; } },
	{"pos", outmapout { out << (hit->pos).x()/mm << " " << (hit->pos).y()/mm  << " " << (hit->pos).z()/mm; } },
	{"dx", outmapout { out << (hit->post_dir).x(); } },
	{"dy", outmapout { out << (hit->post_dir).y(); } },
	{"dz", outmapout { out << (hit->post_dir).z(); } },
	{"dir", outmapout { out << (hit->post_dir).x() << " " << (hit->post_dir).y() << " " << (hit->post_dir).z(); } },

	{"post_dx", outmapout { out << (hit->post_dir).x(); } },
	{"post_dy", outmapout { out << (hit->post_dir).y(); } },
	{"post_dz", outmapout { out << (hit->post_dir).z(); } },
	{"post_dir", outmapout { out << (hit->post_dir).x() << " " << (hit->post_dir).y() << " " << (hit->post_dir).z(); } },

	{"pre_dx", outmapout { out << (hit->pre_dir).x(); } },
	{"pre_dy", outmapout { out << (hit->pre_dir).y(); } },
	{"pre_dz", outmapout { out << (hit->pre_dir).z(); } },
	{"pre_dir", outmapout { out << (hit->pre_dir).x() << " " << (hit->pre_dir).y() << " " << (hit->pre_dir).z(); } },

	{"post_ekin", outmapout { out << hit->post_ekin / MeV; } },
	{"pre_ekin", outmapout { out << hit->pre_ekin / MeV; } },
	{"ekin", outmapout { out << hit->pre_ekin / MeV; } },

	{"ioni", outmapout { out << hit->ioni / MeV; } },
	{"nioni", outmapout { out << hit->nioni / MeV; } },
	{"len", outmapout { out << hit->len / mm; } },
	{"pvx", outmapout { out << (hit->pv_pos).x()/mm; } },
	{"pvy", outmapout { out << (hit->pv_pos).y()/mm; } },
	{"pvz", outmapout { out << (hit->pv_pos).z()/mm; } },
	{"pvpos", outmapout { out << (hit->pv_pos).x()/mm << " " << (hit->pv_pos).y()/mm << " " << (hit->pv_pos).z()/mm; } },
	{"pvdx", outmapout { out << (hit->pv_dir).x(); } },
	{"pvdy", outmapout { out << (hit->pv_dir).y(); } },
	{"pvdz", outmapout { out << (hit->pv_dir).z(); } },
	{"pvdir", outmapout { out << (hit->pv_dir).x() << " " << (hit->pv_dir).y() << " " << (hit->pv_dir).z(); } },
	{"pvekin", outmapout { out << hit->pv_ekin / MeV; } },
	{"runid", outmapout { out << hit->runid; } },
	{"evid", outmapout { out << hit->evid; } },
	{"trid", outmapout { out << hit->trid; } },
	{"parent", outmapout { out << hit->parent; } },
	{"vol", outmapout { out << hit->vol; } },
	{"det", outmapout { out << hit->det; } },
	{"part", outmapout { out << (hit->part)->GetParticleName(); } },
	{"part_A", outmapout { out << (hit->part)->GetAtomicNumber(); } },
	{"part_M", outmapout { out << (hit->part)->GetAtomicMass(); } },
	{"pdg", outmapout { out << (hit->part)->GetPDGEncoding(); } },

	{"pre_proc", outmapout { out << (hit->pre_proc ? hit->pre_proc->GetProcessName() : "creation"); } },
	{"post_proc", outmapout { out << (hit->post_proc ? hit->post_proc->GetProcessName() : "creation"); } },
	{"trk_proc", outmapout { out << (hit->trk_proc ? hit->trk_proc->GetProcessName() : "creation"); } },
	{"proc", outmapout { out << (hit->post_proc ? hit->post_proc->GetProcessName() : "creation"); } },

	{"pre_global_time", outmapout { out << hit->pre_global_time; } },
	{"post_global_time", outmapout { out << hit->post_global_time; } },
	{"pre_local_time", outmapout { out << hit->pre_local_time; } },
	{"post_local_time", outmapout { out << hit->post_local_time; } },
	{"global_time", outmapout { out << hit->post_global_time; } },
	{"local_time", outmapout { out << hit->post_local_time; } },
	{"time", outmapout { out << hit->post_global_time; } },

	{"phi", outmapout { out << (hit->pos).phi(); } },
	{"theta", outmapout { out << (hit->pos).theta(); } },
	{"pvphi", outmapout { out << (hit->pv_pos).phi(); } },
	{"pvtheta", outmapout { out << (hit->pv_pos).theta(); } },

	/* Directions */
	{"dphi", outmapout { out << (hit->post_dir).phi(); } },
	{"dtheta", outmapout { out << (hit->post_dir).theta(); } },

	{"post_dphi", outmapout { out << (hit->post_dir).phi(); } },
	{"post_dtheta", outmapout { out << (hit->post_dir).theta(); } },

	{"pre_dphi", outmapout { out << (hit->pre_dir).phi(); } },
	{"pre_dtheta", outmapout { out << (hit->pre_dir).theta(); } },

	{"pvdphi", outmapout { out << (hit->pv_dir).phi(); } },
	{"pvdtheta", outmapout { out << (hit->pv_dir).theta(); } },

	/* primary particle information */
	{"pvpart", outmapout { out << (hit->pv_part)->GetParticleName(); } },
	{"pvpart_A", outmapout { out << (hit->pv_part)->GetAtomicNumber(); } },
	{"pvpart_M", outmapout { out << (hit->pv_part)->GetAtomicMass(); } },
	{"pvpdg", outmapout { out << (hit->pv_part)->GetPDGEncoding(); } },

	/* detector id */
	{"detid", outmapout { out << hit->detid; } }
};

// global static variables
std::string sensi::formatString;
G4int sensi::verboseLevel;

sensiHit::sensiHit()
 : G4VHit()
{}

sensiHit::~sensiHit() {}

void sensiHit::Print()
{
	;
}

void sensiHit::PrintValue(std::string &val, std::ostream &out)
{
	if( this->outputMap.count(val) == 0) {
		G4cerr << "ERROR: Invalid output formatting key: \"" << val << "\". Ignoring key."  << '\n';
	}
	else {
		outputMap.at(val)(this, out);
	}
}

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4Run.hh"

sensi::sensi(const G4String& name, G4bool zeroEnergy)
 : G4VSensitiveDetector(name),
   fHitsCollection(NULL),
   zeroE(zeroEnergy)
{
	collectionName.insert(name);
	id = gid++;
	InitMessenger();
}

sensi::sensi(const G4String& name, G4int detid, G4bool zeroEnergy)
   : G4VSensitiveDetector(name), 
   fHitsCollection(NULL),
   zeroE(zeroEnergy)
{
   collectionName.insert(name);
   id = detid;
   InitMessenger();
}

sensi::sensi(G4int detid, G4bool zeroEnergy)
   : G4VSensitiveDetector(std::to_string(detid)),
   fHitsCollection(NULL),
   zeroE(zeroEnergy)
{
   collectionName.insert(std::to_string(detid));
   id = detid;
   InitMessenger();
}

sensi::~sensi()
{}

void sensi::Initialize(G4HCofThisEvent* hce)
{
	// Create hits collection
	fHitsCollection	= new sensiHitsCollection(SensitiveDetectorName, collectionName[0]);
	
	// Add this collection in hce
	G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
	hce->AddHitsCollection( hcID, fHitsCollection );
}

G4bool sensi::ProcessHits(G4Step* step, G4TouchableHistory*)
{
	// energy deposit
	G4Track *trk = step->GetTrack();
	G4ParticleDefinition *part = trk->GetDefinition();
	G4String partname = part->GetParticleName();

	const G4Event *evt = G4RunManager::GetRunManager()->GetCurrentEvent();
	const G4Run *run = G4RunManager::GetRunManager()->GetCurrentRun();
	const G4StepPoint *pre = step->GetPreStepPoint();
	const G4StepPoint *post = step->GetPostStepPoint();

	sensiHit *newHit = new sensiHit();

	newHit->vol = pre->GetPhysicalVolume()->GetName();
	newHit->det = this->GetName();
	newHit->part = part;

	newHit->pos = post->GetPosition();
	newHit->post_dir = post->GetMomentumDirection();
	newHit->pre_dir = pre->GetMomentumDirection();
	newHit->post_ekin = post->GetKineticEnergy();
	newHit->pre_ekin = pre->GetKineticEnergy();

	newHit->pre_proc = const_cast<G4VProcess*>(pre->GetProcessDefinedStep());
	newHit->post_proc = const_cast<G4VProcess*>(post->GetProcessDefinedStep());
	newHit->trk_proc = const_cast<G4VProcess*>(trk->GetCreatorProcess());

	newHit->pre_global_time = pre->GetGlobalTime();
	newHit->post_global_time = post->GetGlobalTime();
	newHit->pre_local_time = pre->GetLocalTime();
	newHit->post_local_time = post->GetLocalTime();

	newHit->pv_pos = evt->GetPrimaryVertex()->GetPosition();
	newHit->pv_dir = evt->GetPrimaryVertex()->GetPrimary()->GetMomentumDirection();
	newHit->pv_ekin = evt->GetPrimaryVertex()->GetPrimary()->GetKineticEnergy();
	newHit->pv_part = evt->GetPrimaryVertex()->GetPrimary()->GetG4code();

	newHit->nioni = step->GetNonIonizingEnergyDeposit();
	newHit->ioni = step->GetTotalEnergyDeposit() - newHit->nioni;

	newHit->evid = evt->GetEventID();
	newHit->trid = trk->GetTrackID();
	newHit->runid = run->GetRunID();
	newHit->parent = trk->GetParentID();

	newHit->len = step->GetStepLength();

	newHit->detid = this->GetID();

	fHitsCollection->insert( newHit );

	return true;
}

void sensi::PrintEvents(sensiHit *hit)
{
        if(!zeroE && hit->ioni == 0)
	   return;
	hit->Print();
	if (outstream.is_open())
		outstream << FormatOutput(hit) << '\n';
	else
		G4cout << "SD: "<< FormatOutput(hit) << '\n';
}

void sensi::PrintTracks(sensiHit *hit)
{
        if(!zeroE && hit->ioni == 0)
	   return;
	hit->Print();
	if (trkoutstream.is_open())
		trkoutstream << FormatOutput(hit) << '\n';
	else
		G4cout << "SDST: " << FormatOutput(hit) << '\n';
}
void sensi::PrintSteps(sensiHit *hit)
{
        if(!zeroE && hit->ioni == 0)
	   return;
	hit->Print();
	if (stpoutstream.is_open())
		stpoutstream << FormatOutput(hit) << '\n';
	else
		G4cout << "SDST: " << FormatOutput(hit) << '\n';
}

void sensi::EndOfEvent(G4HCofThisEvent*)
{
	if ( verboseLevel > 0 && !formatString.empty() ) {
		hitMap.clear();
		G4int nofHits = fHitsCollection->entries();
		for ( G4int i=0; i<nofHits; i++ ) {
			if( verboseLevel & 4 ) {
				this->PrintSteps((*fHitsCollection)[i]);
			}
			if ( verboseLevel & 1 || verboseLevel & 2 ) {
				sensiHit* hit = (*fHitsCollection)[i];
				if( !(hitMap.count(hit->det)) )
					hitMap[hit->det] = HitTrackMap();
				HitTrackMap &hitTrackMap = hitMap[hit->det];
				if( !(hitTrackMap.count(hit->trid)) ) {
					hitTrackMap[hit->trid] = *hit;
				}
				else {
					hitTrackMap[hit->trid].ioni += hit->ioni;
					hitTrackMap[hit->trid].nioni += hit->nioni;
					hitTrackMap[hit->trid].post_proc = hit->post_proc;
					hitTrackMap[hit->trid].post_dir = hit->post_dir;
				}
			}
		}
		if ( (verboseLevel & 1 || verboseLevel & 2) && nofHits ) {
			for( auto &h : hitMap) {
				HitTrackMap &hitTrackMap = h.second;
				sensiHit hit = (*hitTrackMap.begin()).second;
				hit.ioni = 0;
				hit.nioni = 0;
				hit.len = 0;
				for( auto &hh : hitTrackMap ) {
					if( verboseLevel & 2 ) {
						PrintTracks( &(hh.second) );
					}
					hit.ioni += hh.second.ioni;
					hit.nioni += hh.second.nioni;
				}
				if(verboseLevel & 1) {
					PrintEvents(&hit);
				}
			}
		}
	}
}

void sensi::OutputFile(G4String fn)
{
	size_t dpos = fn.find_last_of('/');
	G4String dir;

	if(dpos == G4String::npos) {
		dir = "";
	}
	else {
		dir = fn.substr(0,dpos+1);
		fn = fn.substr(dpos+1);
	}
	if ( verboseLevel & 1) {
		if ( G4Threading::IsWorkerThread() ) {
			G4int tid = G4Threading::G4GetThreadId();
			outputFileName = dir + fn + "." + std::to_string(tid) + ".hits";
		}
		else if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM)
			outputFileName = dir + fn + ".hits";
		outstream.close();
		outstream.open(outputFileName, std::ofstream::out);
	}
	if ( verboseLevel & 2) {
		if ( G4Threading::IsWorkerThread() ) {
			G4int tid = G4Threading::G4GetThreadId();
			outputFileName = dir + fn + "." + std::to_string(tid) + ".tracks";
		}
		else if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM)
			outputFileName = dir + fn + ".tracks";
		trkoutstream.close();
		trkoutstream.open(outputFileName, std::ofstream::out);
	}
	if ( verboseLevel & 4) {
		if ( G4Threading::IsWorkerThread() ) {
			G4int tid = G4Threading::G4GetThreadId();
			outputFileName = dir + fn + "." + std::to_string(tid) + ".steps";
		}
		else if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM)
			outputFileName = dir + fn + ".steps";
		stpoutstream.close();
		stpoutstream.open(outputFileName, std::ofstream::out);
	}

}

void sensi::InitMessenger()
{
	verboseCmd = new G4UIcmdWithAnInteger("/sd/verbose", this);
	verboseCmd->SetGuidance("Set Sensitive Detector verboseLevel");
	verboseCmd->SetParameterName("Verbose", false, false);
	verboseCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

	fileCmd = new G4UIcmdWithAString("/sd/file", this);
	fileCmd->SetGuidance("Set Sensitive Detector Output file name");
	fileCmd->SetParameterName("Filename", false, false);
	fileCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

	formatCmd = new G4UIcmdWithAString("/sd/format", this);
	formatCmd->SetGuidance("Set Sensitive Detector Output format string");
	formatCmd->SetParameterName("Format", false, false);
	formatCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

	stateCmd = new G4UIcmdWithAString("/rng/save", this);
	stateCmd->SetGuidance("Save PRNG state to a file");
	stateCmd->SetParameterName("Filename", false, false);
	stateCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

	stateLCmd = new G4UIcmdWithAString("/rng/load", this);
	stateLCmd->SetGuidance("Load PRNG state from a file");
	stateLCmd->SetParameterName("Filename", false, false);
	stateLCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

void sensi::WriteFormatString()
{
	if(formatString.empty())
		return;
	if(outstream.is_open())
		outstream << "# " << formatString << '\n';
	if(trkoutstream.is_open())
		trkoutstream << "# " << formatString << '\n';
	if(stpoutstream.is_open())
		stpoutstream << "# " << formatString << '\n';
}

void sensi::WriteRandomState(const G4String &filename)
{
	CLHEP::HepRandom::saveEngineStatus(filename.c_str());
}

void sensi::LoadRandomState(const G4String &filename)
{
	CLHEP::HepRandom::restoreEngineStatus(filename.c_str());
}

void sensi::SetNewValue(G4UIcommand *cmd, G4String val)
{
	if (cmd == verboseCmd)
		SetVerboseLevel(verboseCmd->GetNewIntValue(val));
	else if (cmd == fileCmd) {
		this->OutputFile(val);
	        this->WriteFormatString();
	}
	else if (cmd == formatCmd) {
		this->formatString = val;
		this->SetFormat();
	}
	else if (cmd == stateCmd)
		this->WriteRandomState(val);
	else if (cmd == stateLCmd)
		this->LoadRandomState(val);
}

G4String sensi::GetNewValue(G4UIcommand *cmd)
{
	if (cmd == verboseCmd)
		return std::to_string(verboseLevel);
	else if (cmd == fileCmd)
		return outputFileName;
	else if (cmd == formatCmd)
		return formatString;
	return "";
}

void sensi::SetFormat()
{
	std::istringstream fss(this->formatString);
	std::string tmp;
	format.clear();

	while(fss.good()) {
		fss >> tmp;
		if( sensiHit::outputMap.count(tmp) == 0) {
			G4cerr << "ERROR: Invalid output formatting key: \"" << tmp << "\". Ignoring key."  << '\n';
		}
		else {
			format.push_back(sensiHit::outputMap.at(tmp));
		}
	}
}

std::string sensi::FormatOutput(sensiHit *hit) const
{
	std::stringstream oss;
	std::string tmp;

	for(auto &fmt : this->format) {
		fmt(hit, oss);
		oss << " ";
	}

	tmp = oss.str();
	// remove last extra whitespace
	tmp.pop_back();
	return tmp;
}

BeamDump::BeamDump() : G4VSensitiveDetector("dump_" + std::to_string(id))
{
	id++;
}

BeamDump::~BeamDump()
{;}

G4bool BeamDump::ProcessHits(G4Step *st, G4TouchableHistory*)
{
	G4Track *tr = st->GetTrack();
	tr->SetTrackStatus(G4TrackStatus::fStopAndKill);
	return true;
}
