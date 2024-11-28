#ifndef sensi_hh
#define sensi_hh

#include "G4VSensitiveDetector.hh"
#include "G4UImessenger.hh"

#include <vector>
#include <fstream>

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "tls.hh"
#include <unordered_map>
#include <map>
#include <functional>

class sensiHit;
class G4ParticleDefinition;
class G4VProcess;

typedef std::unordered_map<std::string, std::function<void(sensiHit *hit, std::ostream &out)>> OutputMap;

class sensiHit : public G4VHit
{
public:
	sensiHit();
	virtual ~sensiHit();

	inline void* operator new(size_t);
	inline void  operator delete(void*);

	virtual void Print();
	void PrintValue(std::string &val, std::ostream &out);

	// variables
	G4ThreeVector pos, pre_dir, post_dir, pv_pos, pv_dir;
	G4double post_ekin, pre_ekin, pv_ekin, len, ioni, nioni;
	G4String det, vol;
	G4VProcess *pre_proc, *post_proc, *trk_proc;
	G4int evid, trid, runid, detid, parent;
	G4ParticleDefinition *part, *pv_part;
	G4double pre_global_time, post_global_time, pre_local_time, post_local_time;
	
	static OutputMap outputMap;

	static G4ThreadLocal G4Allocator<sensiHit>* sensiHitAllocator;
};

typedef G4THitsCollection<sensiHit> sensiHitsCollection;

inline void* sensiHit::operator new(size_t)
{
	if(!sensiHitAllocator)
		sensiHitAllocator = new G4Allocator<sensiHit>;
	return (void *) sensiHitAllocator->MallocSingle();
}

inline void sensiHit::operator delete(void *hit)
{
	sensiHitAllocator->FreeSingle((sensiHit*) hit);
}


class G4Step;
class G4HCofThisEvent;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;

typedef std::map<int, sensiHit> HitTrackMap;
typedef std::map<std::string, HitTrackMap> HitMap;

class sensi : public G4VSensitiveDetector, public G4UImessenger
{
public:
	sensi(const G4String& name, G4bool zeroEnergy=true);
	sensi(const G4String& name, G4int detid, G4bool zeroEnergy=true);
	sensi(G4int detid, G4bool zeroEnergy=true);
	virtual ~sensi();

	// methods from base class
	virtual void   Initialize(G4HCofThisEvent* hitCollection);
	virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
	virtual void   EndOfEvent(G4HCofThisEvent* hitCollection);

	void InitMessenger();
	virtual void SetNewValue(G4UIcommand *cmd, G4String val);
	virtual G4String GetNewValue(G4UIcommand *cmd);
	virtual void PrintSteps(sensiHit* hit);
	virtual void PrintTracks(sensiHit* hit);
	virtual void PrintEvents(sensiHit* hit);
	virtual void SetVerboseLevel(G4int v) { verboseLevel = v; }
	virtual G4int GetVerboseLevel() { return verboseLevel; }
	G4int GetID() { return id; }

private:
	void OutputFile(G4String filename);
	void WriteFormatString();
	void WriteRandomState(const G4String &filename);
	void LoadRandomState(const G4String &filename);
	std::string FormatOutput(sensiHit *hit) const;
	static G4ThreadLocal std::vector<std::function<void(sensiHit *hit, std::ostream &out)>> format;

	sensiHitsCollection* fHitsCollection;
	static G4ThreadLocal G4String outputFileName;
	static G4ThreadLocal G4int gid;
	G4int id;
	G4bool zeroE;
protected:
	static std::string formatString;
	HitMap hitMap;
	static G4int verboseLevel;
	G4UIcmdWithAnInteger *verboseCmd;
	G4UIcmdWithAString *fileCmd, *formatCmd, *stateCmd, *stateLCmd;

	virtual void SetFormat();

	static G4ThreadLocal std::ofstream outstream, trkoutstream, stpoutstream;
};

class BeamDump : public G4VSensitiveDetector
{
public:
	BeamDump();
	virtual ~BeamDump();

	virtual void Initialize(G4HCofThisEvent*) {;};
	virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
	virtual void EndOfEvent(G4HCofThisEvent*) {;};
protected:
	static G4ThreadLocal G4int id;
};

#endif
