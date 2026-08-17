#ifndef sensi_hh
#define sensi_hh

#include "G4VSensitiveDetector.hh"
#include "G4UImessenger.hh"

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"
#include "G4Types.hh"

#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <functional>
#include <string>

class sensiHit;
class G4ParticleDefinition;
class G4VProcess;
class G4Step;
class G4HCofThisEvent;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;

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
    G4ThreeVector pos, pre_pos, post_pos, pre_dir, post_dir, pv_pos, pv_dir;
    G4double ekin = 0., pv_ekin = 0., len = 0., ioni = 0., nioni = 0.;
    G4double pdg_m = 0., pdg_c = 0.;
    G4double optPhot = 0.;    // analytic Cherenkov photons (see /sd/optics/*)
    G4double scintPhot = 0.;  // analytic scintillation photons (see /sd/scint/*)
    G4double beta = 0.;
    G4String det, vol;
    const G4VProcess *pre_proc  = nullptr;
    const G4VProcess *post_proc = nullptr;
    const G4VProcess *trk_proc  = nullptr;
    G4int evid = 0, trid = 0, runid = 0, detid = 0, parent = 0;
    const G4ParticleDefinition *part    = nullptr;
    const G4ParticleDefinition *pv_part = nullptr;
    G4double pre_global_time = 0., post_global_time = 0.;
    G4double pre_local_time  = 0., post_local_time  = 0.;

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

typedef std::map<int, sensiHit> HitTrackMap;
typedef std::map<G4String, HitTrackMap> HitMap;

class sensi : public G4VSensitiveDetector, public G4UImessenger
{
public:
    sensi(const G4String& name, G4bool zeroEnergy=true);
    sensi(const G4String& name, G4int detid, G4bool zeroEnergy=true);
    sensi(G4int detid, G4bool zeroEnergy=true);
    virtual ~sensi();

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
    virtual void SetFormat();


    // ---------- per-detector optics configuration (Cherenkov) ----------
public:
    enum class OpticsType { None, ConstN, Dispersive };

    struct ConstNOptics {
      double n = 1.0;                // refractive index (no dispersion)
      double lamMin_nm = 300.0;      // integration band
      double lamMax_nm = 600.0;
      G4String qePath;               // optional QE(λ) file; empty => QE=1
      // cached QE grid per thread
      std::vector<double> lam_nm_qe;
      std::vector<double> qe_of_lam;
      bool qeLoaded = false;
    };

    struct DispOptics {
      G4String refrPath;             // λ[µm] n(λ)
      G4String qePath;               // λ[nm] QE(λ)
      double dLam_nm = 1.0;
      // cached dispersion grid per thread
      std::vector<double> lam_nm_grid;
      std::vector<double> n_of_lam;
      std::vector<double> qe_of_lam;
      bool loaded = false;
    };

    struct OpticsConfig {
      OpticsType type = OpticsType::None;
      ConstNOptics cn;
      DispOptics   disp;
    };

    // ---------- per-detector scintillation configuration ----------
    //
    // Analogue of the optics configuration above: Const <-> ConstN and
    // Spectral <-> Dispersive (in both cases the second variant is the one
    // that folds a wavelength-dependent quantum efficiency into the yield).
    enum class ScintType { None, Const, Spectral };

    struct ConstScint {
      double S_perMeV = 0.0;         // absolute light yield [photons/MeV]
      double kB_mm_per_MeV = 0.0;    // Birks constant [mm/MeV]
      double eff = 1.0;              // flat collection x QE factor
    };

    struct SpecScint {
      double S_perMeV = 0.0;
      double kB_mm_per_MeV = 0.0;
      G4String emisPath;             // λ[nm] I(λ), emission spectrum (arb. units)
      G4String qePath;               // λ[nm] QE(λ)
      double dLam_nm = 1.0;
      // cached spectral overlap ∫I·QE dλ / ∫I dλ, per thread
      double effCached = 0.0;
      bool loaded = false;
    };

    struct ScintConfig {
      ScintType type = ScintType::None;
      ConstScint cs;
      SpecScint  sp;
      double chou_C = 0.0;           // second-order Birks/Chou term [mm^2/MeV^2]
    };

private:
    static G4ThreadLocal std::map<G4String, OpticsConfig> opticsMap;
    static G4ThreadLocal std::map<G4String, ScintConfig>  scintMap;

    // Generic whitespace-separated two-column reader: skips blank / # / //
    // lines, sorts by x and drops duplicate x (which would divide by zero
    // when interpolating). Used by every optics/scintillation table loader.
    static void   LoadTwoColumn(const G4String& path, const char* what,
                                std::vector<double>& x, std::vector<double>& y);

    static void   LoadQEFile(const G4String& path,
                             std::vector<double>& lam_nm,
                             std::vector<double>& qe_frac); // auto %->fraction

    static void   LoadSpectrum(const G4String& path,
                               std::vector<double>& lam_nm,
                               std::vector<double>& intensity);

    static void   LoadDispersion(const G4String& refrPath, const G4String& qePath, double dLam_nm,
                                 std::vector<double>& lam_nm_grid,
                                 std::vector<double>& n_of_lam,
                                 std::vector<double>& qe_of_lam);

    static double CherenkovPerStep_ConstN(double beta, double z_abs_e, double step_len_mm,
                                          const ConstNOptics& cfg);
    static double CherenkovPerStep_Disp(double beta, double z_abs_e, double step_len_mm,
                                        const DispOptics& cfg);
    static double BetaFromTandM_MeV(double T, double m);

    static double ScintPerStep(double dE_MeV, double step_len_mm, const ScintConfig& cfg);
    static double SpectralEfficiency(SpecScint& cfg);

    // data
    sensiHitsCollection* fHitsCollection;
    static G4ThreadLocal G4String outputFileName;
    static G4ThreadLocal G4int gid;
    G4int id;
    G4bool zeroE;
    G4bool ownsCmds = false;   

protected:
    static std::string formatString;
    HitMap hitMap;
    static G4int verboseLevel;
    static G4ThreadLocal G4bool cmdsCreated;

    // UI commands
    G4UIcmdWithAnInteger *verboseCmd = nullptr;
    G4UIcmdWithAString *fileCmd = nullptr;
    G4UIcmdWithAString *formatCmd = nullptr;
    G4UIcmdWithAString *stateCmd = nullptr;
    G4UIcmdWithAString *stateLCmd = nullptr;

    // Cherenkov optics UI
    G4UIcmdWithAString *optConstCmd = nullptr;  // /sd/optics/const
    G4UIcmdWithAString *optDispCmd  = nullptr;  // /sd/optics/disp
    G4UIcmdWithAString *optClearCmd = nullptr;  // /sd/optics/clear

    // scintillation UI
    G4UIcmdWithAString *scintConstCmd = nullptr;  // /sd/scint/const
    G4UIcmdWithAString *scintSpecCmd  = nullptr;  // /sd/scint/spec
    G4UIcmdWithAString *scintChouCmd  = nullptr;  // /sd/scint/chou
    G4UIcmdWithAString *scintClearCmd = nullptr;  // /sd/scint/clear

    static G4ThreadLocal std::ofstream outstream, trkoutstream, stpoutstream;
};

class BeamDump : public G4VSensitiveDetector
{
public:
    BeamDump();
    virtual ~BeamDump();

    virtual void Initialize(G4HCofThisEvent*) {;}
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
    virtual void EndOfEvent(G4HCofThisEvent*) {;}
protected:
    static G4ThreadLocal G4int id;
};

#endif
