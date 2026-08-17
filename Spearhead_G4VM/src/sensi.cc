#include "sensi.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "CLHEP/Random/Random.h"
#include "G4TrackStatus.hh"
#include "G4VProcess.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4Run.hh"

#include <sstream>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

// ThreadLocal static variables
G4ThreadLocal G4Allocator<sensiHit>* sensiHit::sensiHitAllocator = nullptr;
G4ThreadLocal std::ofstream sensi::outstream;
G4ThreadLocal std::ofstream sensi::trkoutstream;
G4ThreadLocal std::ofstream sensi::stpoutstream;
G4ThreadLocal G4String sensi::outputFileName;
G4ThreadLocal G4int BeamDump::id=0;
G4ThreadLocal std::vector<std::function<void(sensiHit *hit, std::ostream &out)>> sensi::format;
G4ThreadLocal G4int sensi::gid=0;
G4ThreadLocal G4bool sensi::cmdsCreated = false;

// per-detector optics / scintillation maps
G4ThreadLocal std::map<G4String, sensi::OpticsConfig> sensi::opticsMap;
G4ThreadLocal std::map<G4String, sensi::ScintConfig>  sensi::scintMap;

// global static variables
std::string sensi::formatString;
G4int sensi::verboseLevel = 0;

// ---------------- sensiHit mapping ----------------

#define outmapout [](sensiHit *hit, std::ostream &out)

OutputMap sensiHit::outputMap = {
    {"x",         outmapout { out << (hit->pos).x()/mm; } },
    {"y",         outmapout { out << (hit->pos).y()/mm; } },
    {"z",         outmapout { out << (hit->pos).z()/mm; } },
    {"pos",       outmapout { out << (hit->pos).x()/mm << " " << (hit->pos).y()/mm  << " " << (hit->pos).z()/mm; } },
    {"dx",        outmapout { out << (hit->post_dir).x(); } },
    {"pre_pos",   outmapout { out << (hit->pre_pos).x()/mm << " " << (hit->pre_pos).y()/mm  << " " << (hit->pre_pos).z()/mm; } },
    {"post_pos",  outmapout { out << (hit->post_pos).x()/mm << " " << (hit->post_pos).y()/mm  << " " << (hit->post_pos).z()/mm; } },
    {"dy",        outmapout { out << (hit->post_dir).y(); } },
    {"dz",        outmapout { out << (hit->post_dir).z(); } },
    {"dir",       outmapout { out << (hit->post_dir).x() << " " << (hit->post_dir).y() << " " << (hit->post_dir).z(); } },

    {"post_dx",   outmapout { out << (hit->post_dir).x(); } },
    {"post_dy",   outmapout { out << (hit->post_dir).y(); } },
    {"post_dz",   outmapout { out << (hit->post_dir).z(); } },
    {"post_dir",  outmapout { out << (hit->post_dir).x() << " " << (hit->post_dir).y() << " " << (hit->post_dir).z(); } },

    {"pre_dx",    outmapout { out << (hit->pre_dir).x(); } },
    {"pre_dy",    outmapout { out << (hit->pre_dir).y(); } },
    {"pre_dz",    outmapout { out << (hit->pre_dir).z(); } },
    {"pre_dir",   outmapout { out << (hit->pre_dir).x() << " " << (hit->pre_dir).y() << " " << (hit->pre_dir).z(); } },

    {"ekin",      outmapout { out << hit->ekin / MeV; } },
    {"ioni",      outmapout { out << hit->ioni / MeV; } },
    {"nioni",     outmapout { out << hit->nioni / MeV; } },
    {"len",       outmapout { out << hit->len / mm; } },
    {"pvx",       outmapout { out << (hit->pv_pos).x()/mm; } },
    {"pvy",       outmapout { out << (hit->pv_pos).y()/mm; } },
    {"pvz",       outmapout { out << (hit->pv_pos).z()/mm; } },
    {"pvpos",     outmapout { out << (hit->pv_pos).x()/mm << " " << (hit->pv_pos).y()/mm << " " << (hit->pv_pos).z()/mm; } },
    {"pvdx",      outmapout { out << (hit->pv_dir).x(); } },
    {"pvdy",      outmapout { out << (hit->pv_dir).y(); } },
    {"pvdz",      outmapout { out << (hit->pv_dir).z(); } },
    {"pvdir",     outmapout { out << (hit->pv_dir).x() << " " << (hit->pv_dir).y() << " " << (hit->pv_dir).z(); } },
    {"pvekin",    outmapout { out << hit->pv_ekin / MeV; } },
    {"runid",     outmapout { out << hit->runid; } },
    {"evid",      outmapout { out << hit->evid; } },
    {"trid",      outmapout { out << hit->trid; } },
    {"parent",    outmapout { out << hit->parent; } },
    {"vol",       outmapout { out << hit->vol; } },
    {"det",       outmapout { out << hit->det; } },
    {"part",      outmapout { out << (hit->part)->GetParticleName(); } },
    {"part_A",    outmapout { out << (hit->part)->GetAtomicNumber(); } },
    {"part_M",    outmapout { out << (hit->part)->GetAtomicMass(); } },
    {"pdg",       outmapout { out << (hit->part)->GetPDGEncoding(); } },
    {"pdg_m",     outmapout { out << (hit->part)->GetPDGMass(); } },
    {"pdg_c",     outmapout { out << (hit->part)->GetPDGCharge(); } },

    {"pre_proc",  outmapout { out << (hit->pre_proc ? hit->pre_proc->GetProcessName() : "creation"); } },
    {"post_proc", outmapout { out << (hit->post_proc ? hit->post_proc->GetProcessName() : "creation"); } },
    {"trk_proc",  outmapout { out << (hit->trk_proc ? hit->trk_proc->GetProcessName() : "creation"); } },
    {"proc",      outmapout { out << (hit->post_proc ? hit->post_proc->GetProcessName() : "creation"); } },

    {"pre_global_time",  outmapout { out << hit->pre_global_time; } },
    {"post_global_time", outmapout { out << hit->post_global_time; } },
    {"pre_local_time",   outmapout { out << hit->pre_local_time; } },
    {"post_local_time",  outmapout { out << hit->post_local_time; } },
    {"global_time",      outmapout { out << hit->post_global_time; } },
    {"local_time",       outmapout { out << hit->post_local_time; } },
    {"time",             outmapout { out << hit->post_global_time; } },

    {"phi",       outmapout { out << (hit->pos).phi(); } },
    {"theta",     outmapout { out << (hit->pos).theta(); } },
    {"pvphi",     outmapout { out << (hit->pv_pos).phi(); } },
    {"pvtheta",   outmapout { out << (hit->pv_pos).theta(); } },

    {"dphi",           outmapout { out << (hit->post_dir).phi(); } },
    {"dtheta",         outmapout { out << (hit->post_dir).theta(); } },
    {"post_dphi",      outmapout { out << (hit->post_dir).phi(); } },
    {"post_dtheta",    outmapout { out << (hit->post_dir).theta(); } },
    {"pre_dphi",       outmapout { out << (hit->pre_dir).phi(); } },
    {"pre_dtheta",     outmapout { out << (hit->pre_dir).theta(); } },
    {"pvdphi",         outmapout { out << (hit->pv_dir).phi(); } },
    {"pvdtheta",       outmapout { out << (hit->pv_dir).theta(); } },

    {"pvpart",   outmapout { out << (hit->pv_part)->GetParticleName(); } },
    {"pvpart_A", outmapout { out << (hit->pv_part)->GetAtomicNumber(); } },
    {"pvpart_M", outmapout { out << (hit->pv_part)->GetAtomicMass(); } },
    {"pvpdg",    outmapout { out << (hit->pv_part)->GetPDGEncoding(); } },

    {"optPhot",   outmapout { out << (hit->optPhot); } },
    {"scintPhot", outmapout { out << (hit->scintPhot); } },
    {"beta",      outmapout { out << (hit->beta); } },
    {"detid",     outmapout { out << hit->detid; } }
};

sensiHit::sensiHit() : G4VHit() {}
sensiHit::~sensiHit() {}
void sensiHit::Print() {}
void sensiHit::PrintValue(std::string &val, std::ostream &out)
{
    if (this->outputMap.count(val) == 0) {
        G4cerr << "ERROR: Invalid output formatting key: \"" << val << "\". Ignoring key."  << '\n';
    } else {
        outputMap.at(val)(this, out);
    }
}

// --------- helpers for optics files & integration ----------

// Linear interpolation with edge clamping. x must be sorted; 
static double interp1(const std::vector<double>& x, const std::vector<double>& y, double xq)
{
  if (x.empty()) return 0.0;
  if (xq <= x.front()) return y.front();
  if (xq >= x.back())  return y.back();
  auto it = std::upper_bound(x.begin(), x.end(), xq);
  size_t j = it - x.begin();
  size_t i = j - 1;
  const double dx = x[j] - x[i];
  if (dx <= 0.0) return y[i];
  return y[i] + (y[j] - y[i]) * ((xq - x[i]) / dx);
}

// As interp1(), but returns 0 outside the tabulated range instead of clamping.
// Used where "not tabulated" means "not sensitive" rather than "constant".
static double interp1_zero(const std::vector<double>& x, const std::vector<double>& y, double xq)
{
  if (x.empty() || xq < x.front() || xq > x.back()) return 0.0;
  return interp1(x, y, xq);
}

static inline double trap_weight(size_t i, size_t N) {
  return (i==0 || i==N-1) ? 0.5 : 1.0;
}

// Reject unreadable optics files at /sd/optics/* and /sd/scint/* command time;

static bool opticsFileReadable(const G4String& path, const char* what)
{
  std::ifstream in(path.c_str());
  if (in) return true;
  G4Exception("sensi","OpticsFile",JustWarning,
              (std::string("Cannot open ")+what+" file: "+std::string(path)
               +" - config rejected").c_str());
  return false;
}

static bool cfgReject(const char* code, const std::string& msg)
{
  G4Exception("sensi", code, JustWarning, msg.c_str());
  return false;
}


static bool optionalDouble(std::istringstream& iss, double& out)
{
  std::string tok;
  if (!(iss >> tok)) return true;              // absent: keep the default
  try {
    size_t used = 0;
    const double v = std::stod(tok, &used);
    if (used != tok.size()) return false;      // trailing garbage
    out = v;
    return true;
  }
  catch (const std::exception&) { return false; }
}

void sensi::LoadTwoColumn(const G4String& path, const char* what,
                          std::vector<double>& x, std::vector<double>& y)
{
  x.clear(); y.clear();
  std::ifstream in(path.c_str());
  if (!in) G4Exception("sensi","Table1",FatalException,
                       (std::string("Cannot open ")+what+" file: "+std::string(path)).c_str());
  std::string line;
  while (std::getline(in,line)) {
    if (line.empty() || line[0]=='#' || (line.size()>1 && line[0]=='/' && line[1]=='/')) continue;
    std::istringstream iss(line);
    double a,b; if (!(iss>>a>>b)) continue;
    x.push_back(a); y.push_back(b);
  }
  if (x.empty()) G4Exception("sensi","Table2",FatalException,
                             (std::string("Empty ")+what+" data: "+std::string(path)).c_str());

  std::vector<size_t> idx(x.size()); std::iota(idx.begin(),idx.end(),0);
  std::stable_sort(idx.begin(),idx.end(),[&](size_t a,size_t b){return x[a]<x[b];});

  // sort and drop duplicate abscissae - a repeated wavelength is ambiguous
  // (which of the two values applies?) and would be the one input that could
  // make the interpolation divide by zero
  std::vector<double> xs, ys;
  xs.reserve(x.size()); ys.reserve(y.size());
  for (size_t k=0;k<idx.size();++k) {
    const double xv = x[idx[k]];
    if (!xs.empty() && xv == xs.back()) continue;
    xs.push_back(xv); ys.push_back(y[idx[k]]);
  }
  x.swap(xs); y.swap(ys);
}

void sensi::LoadQEFile(const G4String& path,
                       std::vector<double>& lam_nm,
                       std::vector<double>& qe_frac)
{
  LoadTwoColumn(path, "QE", lam_nm, qe_frac);

  // Files are found both in percent and as a fraction; anything above 1 can
  // only be percent. Say so, so that a silently misread file is noticed.
  const double qmax = *std::max_element(qe_frac.begin(),qe_frac.end());
  if (qmax > 1.0) {
    G4cout << "sensi: QE file " << path << " has max " << qmax
           << " > 1, interpreting it as percent." << G4endl;
    for (auto& q : qe_frac) q *= 0.01;
  }
  for (auto& q : qe_frac) if (q < 0.0) q = 0.0;
}

void sensi::LoadSpectrum(const G4String& path,
                         std::vector<double>& lam_nm,
                         std::vector<double>& intensity)
{
  // emission spectrum in arbitrary units - only its shape matters
  LoadTwoColumn(path, "emission spectrum", lam_nm, intensity);
  for (auto& v : intensity) if (v < 0.0) v = 0.0;
}

void sensi::LoadDispersion(const G4String& refrPath, const G4String& qePath, double dLam_nm,
                           std::vector<double>& lam_nm_grid,
                           std::vector<double>& n_of_lam,
                           std::vector<double>& qe_of_lam)
{
  // refr: [µm, n]
  std::vector<double> wl_nm, n_sorted;
  LoadTwoColumn(refrPath, "refractive", wl_nm, n_sorted);
  for (auto& w : wl_nm) w *= 1000.0;   // µm -> nm

  std::vector<double> wlq_nm, qe;
  LoadQEFile(qePath, wlq_nm, qe);

  double lam_min = std::max(wl_nm.front(), wlq_nm.front());
  double lam_max = std::min(wl_nm.back(),  wlq_nm.back());
  if (lam_max<=lam_min) G4Exception("sensi","Disp3",FatalException,"No wavelength overlap");

  // The grid ends at the last full dLam step, so up to dLam of the band is
  // dropped 
  int N = int(std::floor((lam_max - lam_min)/dLam_nm))+1;
  lam_nm_grid.resize(N);
  for (int i=0;i<N;++i) lam_nm_grid[i]=lam_min+i*dLam_nm;

  n_of_lam.resize(N); qe_of_lam.resize(N);
  for (int i=0;i<N;++i){
    double L=lam_nm_grid[i];
    n_of_lam[i]  = interp1(wl_nm, n_sorted, L);
    qe_of_lam[i] = interp1(wlq_nm, qe, L);
  }
}

double sensi::CherenkovPerStep_ConstN(double beta, double z_abs_e, double step_len_mm,
                                      const ConstNOptics& cfg)
{
  if (beta<=0 || z_abs_e<=0 || step_len_mm<=0) return 0.0;
  const double step_cm = step_len_mm * 0.1;
  const double coeff = 2.0*CLHEP::pi*CLHEP::fine_structure_const*(z_abs_e*z_abs_e);
  const double betaTerm = 1.0 - 1.0/(beta*beta*cfg.n*cfg.n);
  if (betaTerm<=0) return 0.0;

  if (!cfg.qePath.empty()) {
    ConstNOptics& m = const_cast<ConstNOptics&>(cfg);
    if (!m.qeLoaded) {
      LoadQEFile(cfg.qePath, m.lam_nm_qe, m.qe_of_lam);
      m.qeLoaded = true;
    }
    double lam_min = std::max(cfg.lamMin_nm, m.lam_nm_qe.front());
    double lam_max = std::min(cfg.lamMax_nm, m.lam_nm_qe.back());
    if (lam_max<=lam_min) return 0.0;

    const double dLam_nm = 1.0;
    int N = int(std::floor((lam_max - lam_min)/dLam_nm))+1;
    double sum=0.0;
    for (int i=0;i<N;++i) {
      double Lnm = lam_min + i*dLam_nm;
      double qe  = interp1(m.lam_nm_qe, m.qe_of_lam, Lnm);
      double Lcm = Lnm*1e-7;
      double f = qe/(Lcm*Lcm);
      sum += trap_weight(i, N) * f;
    }
    double dlam_cm = dLam_nm * 1e-7;
    double integral = sum * dlam_cm;      
    return coeff * betaTerm * integral * step_cm;
  } else {
    const double l1 = cfg.lamMin_nm*1e-7;
    const double l2 = cfg.lamMax_nm*1e-7;
    if (l2<=l1) return 0.0;
    const double spectral = (1.0/l1 - 1.0/l2);
    return coeff * betaTerm * spectral * step_cm;
  }
}

double sensi::CherenkovPerStep_Disp(double beta, double z_abs_e, double step_len_mm,
                                    const DispOptics& cfg)
{
  if (beta<=0 || z_abs_e<=0 || step_len_mm<=0) return 0.0;
  DispOptics& m = const_cast<DispOptics&>(cfg);
  if (!m.loaded) {
    LoadDispersion(m.refrPath, m.qePath, m.dLam_nm, m.lam_nm_grid, m.n_of_lam, m.qe_of_lam);
    m.loaded = true;
  }
  const double step_cm = step_len_mm * 0.1;
  const double coeff = 2.0*CLHEP::pi*CLHEP::fine_structure_const*(z_abs_e*z_abs_e);
  const size_t N = m.lam_nm_grid.size();
  if (N<2) return 0.0;

  double sum=0.0;
  for (size_t i=0;i<N;++i) {
    double Lcm = m.lam_nm_grid[i]*1e-7;
    double n   = m.n_of_lam[i];
    double qe  = m.qe_of_lam[i];
    double betaTerm = 1.0 - 1.0/(beta*beta*n*n);
    if (betaTerm<=0 || qe<=0) continue;
    double f = qe/(Lcm*Lcm) * betaTerm;
    sum += trap_weight(i, N) * f;
  }
  double dlam_cm = cfg.dLam_nm * 1e-7;
  double integral = sum * dlam_cm;
  return coeff * integral * step_cm;
}

double sensi::BetaFromTandM_MeV(double T, double m)
{
  if (m<=0) return 0.0;
  double gamma = 1.0 + T/m;
  if (gamma<=1.0) return 0.0;
  double b2 = 1.0 - 1.0/(gamma*gamma);
  return (b2>0) ? std::sqrt(b2) : 0.0;
}

// --------- scintillation ----------

// Fraction of the emitted scintillation light the photocathode converts:
//
//     eff = ∫ I(λ)·QE(λ) dλ / ∫ I(λ) dλ
//
// integrated over the *emission* band. Wavelengths the QE file does not cover
// count as QE = 0 in the numerator but still as emitted light in the
// denominator - emission outside the sensitive band is lost.
double sensi::SpectralEfficiency(SpecScint& cfg)
{
  if (cfg.loaded) return cfg.effCached;

  std::vector<double> lamE, I, lamQ, QE;
  LoadSpectrum(cfg.emisPath, lamE, I);
  LoadQEFile(cfg.qePath, lamQ, QE);

  cfg.loaded    = true;
  cfg.effCached = 0.0;

  const double lam_min = lamE.front();
  const double lam_max = lamE.back();
  const size_t N = (lam_max > lam_min && cfg.dLam_nm > 0.0)
                 ? size_t(std::floor((lam_max - lam_min)/cfg.dLam_nm)) + 1 : 0;
  if (N < 2) {
    cfgReject("ScintSpec","Emission spectrum spans less than one dlam_nm step"
                          " - no scintillation light for this detector");
    return cfg.effCached;
  }

  double num = 0.0, den = 0.0;
  for (size_t i=0;i<N;++i) {
    const double L = lam_min + i*cfg.dLam_nm;
    const double w = trap_weight(i, N);
    const double Ii = interp1(lamE, I, L);
    num += w * Ii * interp1_zero(lamQ, QE, L);
    den += w * Ii;
  }
  if (den <= 0.0) {   // spectrum is all zeros
    cfgReject("ScintSpec","Emission spectrum integrates to zero"
                          " - no scintillation light for this detector");
    return cfg.effCached;
  }

  cfg.effCached = num/den;                // the dλ factors cancel
  G4cout << "sensi: scintillation spectral efficiency for " << cfg.emisPath
         << " folded with " << cfg.qePath << " = " << cfg.effCached << G4endl;
  return cfg.effCached;
}

// Birks-quenched light yield of one step:
//
//     dL = S · dE / (1 + kB·(dE/dx) + C·(dE/dx)²) · eff
//
// with dE the *ionizing* deposit in MeV (non-ionizing losses do not
// scintillate), dx the step length in mm and kB in mm/MeV 
double sensi::ScintPerStep(double dE_MeV, double step_len_mm, const ScintConfig& C)
{
  if (dE_MeV <= 0.0 || step_len_mm <= 0.0) return 0.0;

  double S = 0.0, kB = 0.0, eff = 0.0;
  if (C.type == ScintType::Const) {
    S = C.cs.S_perMeV; kB = C.cs.kB_mm_per_MeV; eff = C.cs.eff;
  }
  else if (C.type == ScintType::Spectral) {
    S = C.sp.S_perMeV; kB = C.sp.kB_mm_per_MeV;
    eff = SpectralEfficiency(const_cast<SpecScint&>(C.sp));
  }
  else return 0.0;

  if (S <= 0.0 || eff <= 0.0) return 0.0;

  const double dEdx   = dE_MeV / step_len_mm;                       // MeV/mm
  const double quench = 1.0 + kB*dEdx + C.chou_C*dEdx*dEdx;
  if (quench <= 0.0) return 0.0;

  return S * dE_MeV / quench * eff;
}

// ---------------- sensi implementation ----------------

sensi::sensi(const G4String& name, G4bool zeroEnergy)
 : G4VSensitiveDetector(name),
   fHitsCollection(nullptr),
   id(gid++),
   zeroE(zeroEnergy)
{
    collectionName.insert(name);
    InitMessenger();
}

sensi::sensi(const G4String& name, G4int detid, G4bool zeroEnergy)
   : G4VSensitiveDetector(name),
   fHitsCollection(nullptr),
   id(detid),
   zeroE(zeroEnergy)
{
   collectionName.insert(name);
   InitMessenger();
}

sensi::sensi(G4int detid, G4bool zeroEnergy)
   : G4VSensitiveDetector(std::to_string(detid)),
   fHitsCollection(nullptr),
   id(detid),
   zeroE(zeroEnergy)
{
   collectionName.insert(std::to_string(detid));
   InitMessenger();
}

sensi::~sensi()
{
    if (!ownsCmds) return;
    delete verboseCmd;
    delete fileCmd;
    delete formatCmd;
    delete stateCmd;
    delete stateLCmd;
    delete optConstCmd;
    delete optDispCmd;
    delete optClearCmd;
    delete scintConstCmd;
    delete scintSpecCmd;
    delete scintChouCmd;
    delete scintClearCmd;
    cmdsCreated = false;
}

void sensi::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection = new sensiHitsCollection(SensitiveDetectorName, collectionName[0]);
    G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    hce->AddHitsCollection( hcID, fHitsCollection );
}

G4bool sensi::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    if (verboseLevel <= 0) return false;

    G4Track *trk = step->GetTrack();
    const G4ParticleDefinition *part = trk->GetDefinition();

    const G4Event *evt = G4RunManager::GetRunManager()->GetCurrentEvent();
    const G4Run *run   = G4RunManager::GetRunManager()->GetCurrentRun();
    const G4StepPoint *pre  = step->GetPreStepPoint();
    const G4StepPoint *post = step->GetPostStepPoint();

    sensiHit *newHit = new sensiHit();

    newHit->vol  = pre->GetPhysicalVolume()->GetName();
    newHit->det  = this->GetName();
    newHit->part = part;

    newHit->pos      = post->GetPosition();
    newHit->pre_pos  = pre->GetPosition();
    newHit->post_pos = post->GetPosition();
    newHit->post_dir = post->GetMomentumDirection();
    newHit->pre_dir  = pre->GetMomentumDirection();
    newHit->ekin     = pre->GetKineticEnergy();

    newHit->pre_proc  = pre->GetProcessDefinedStep();
    newHit->post_proc = post->GetProcessDefinedStep();
    newHit->trk_proc  = trk->GetCreatorProcess();

    newHit->pre_global_time  = pre->GetGlobalTime();
    newHit->post_global_time = post->GetGlobalTime();
    newHit->pre_local_time   = pre->GetLocalTime();
    newHit->post_local_time  = post->GetLocalTime();

    newHit->pv_pos  = evt->GetPrimaryVertex()->GetPosition();
    newHit->pv_dir  = evt->GetPrimaryVertex()->GetPrimary()->GetMomentumDirection();
    newHit->pv_ekin = evt->GetPrimaryVertex()->GetPrimary()->GetKineticEnergy();
    newHit->pv_part = evt->GetPrimaryVertex()->GetPrimary()->GetG4code();

    newHit->nioni = step->GetNonIonizingEnergyDeposit();
    newHit->ioni  = step->GetTotalEnergyDeposit() - newHit->nioni;

    newHit->evid   = evt->GetEventID();
    newHit->trid   = trk->GetTrackID();
    newHit->runid  = run->GetRunID();
    newHit->parent = trk->GetParentID();

    newHit->len   = step->GetStepLength();

    newHit->detid = this->GetID();
    newHit->pdg_m = part->GetPDGMass();    // MeV
    newHit->pdg_c = part->GetPDGCharge();  // e

    newHit->optPhot   = 0;
    newHit->scintPhot = 0;
    newHit->beta      = 0;

    // --- detected Cherenkov light, per the detector's optics config ---
    const auto itCfg = opticsMap.find(newHit->det);
    if (itCfg != opticsMap.end() && std::abs(newHit->pdg_c)>0.0) {
      const OpticsConfig& C = itCfg->second;
      const double T_MeV = pre->GetKineticEnergy()/MeV;
      const double m_MeV = newHit->pdg_m/MeV;
      const double beta  = BetaFromTandM_MeV(T_MeV, m_MeV);
      newHit->beta = beta;
      if (beta > 0.0) {
        if (C.type == OpticsType::ConstN) {
          newHit->optPhot = CherenkovPerStep_ConstN(beta, std::abs(newHit->pdg_c),
                                                    newHit->len/mm, C.cn);
        } else if (C.type == OpticsType::Dispersive) {
          newHit->optPhot = CherenkovPerStep_Disp(beta, std::abs(newHit->pdg_c),
                                                  newHit->len/mm, C.disp);
        }
      }
    }

    // --- detected scintillation light, per the detector's scint config ---
    const auto itScint = scintMap.find(newHit->det);
    if (itScint != scintMap.end() && itScint->second.type != ScintType::None) {
      newHit->scintPhot = ScintPerStep(newHit->ioni/MeV, newHit->len/mm, itScint->second);
    }

    fHitsCollection->insert( newHit );
    return true;
}

void sensi::PrintEvents(sensiHit *hit)
{
    if(!zeroE && hit->ioni == 0) return;
    hit->Print();
    if (outstream.is_open())
        outstream << FormatOutput(hit) << '\n';
    else
        G4cout << "SD: "<< FormatOutput(hit) << '\n';
}

void sensi::PrintTracks(sensiHit *hit)
{
    if(!zeroE && hit->ioni == 0) return;
    hit->Print();
    if (trkoutstream.is_open())
        trkoutstream << FormatOutput(hit) << '\n';
    else
        G4cout << "SDST: " << FormatOutput(hit) << '\n';
}
void sensi::PrintSteps(sensiHit *hit)
{
    if(!zeroE && hit->ioni == 0) return;
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
                    hitTrackMap[hit->trid].ioni      += hit->ioni;
                    hitTrackMap[hit->trid].nioni     += hit->nioni;
                    hitTrackMap[hit->trid].post_proc  = hit->post_proc;
                    hitTrackMap[hit->trid].post_dir   = hit->post_dir;
                    hitTrackMap[hit->trid].len       += hit->len;
                    hitTrackMap[hit->trid].optPhot   += hit->optPhot;
                    hitTrackMap[hit->trid].scintPhot += hit->scintPhot;
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
                hit.optPhot = 0;
                hit.scintPhot = 0;
                for( auto &hh : hitTrackMap ) {
                    if( verboseLevel & 2 ) {
                        PrintTracks( &(hh.second) );
                    }
                    hit.ioni      += hh.second.ioni;
                    hit.nioni     += hh.second.nioni;
                    hit.len       += hh.second.len;
                    hit.optPhot   += hh.second.optPhot;
                    hit.scintPhot += hh.second.scintPhot;
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
	if (cmdsCreated) return;
    cmdsCreated = true;
    ownsCmds    = true;

    verboseCmd = new G4UIcmdWithAnInteger("/sd/verbose", this);
    verboseCmd->SetGuidance("Set Sensitive Detector verboseLevel (bitmask: 1=events,2=tracks,4=steps)");
    verboseCmd->SetParameterName("Verbose", false, false);
    verboseCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fileCmd = new G4UIcmdWithAString("/sd/file", this);
    fileCmd->SetGuidance("Set Sensitive Detector Output file name (base)");
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

    // Cherenkov optics assignment UI
    optConstCmd = new G4UIcmdWithAString("/sd/optics/const", this);
    optConstCmd->SetGuidance("Set constant-n optics: <det> <n> <lamMin_nm> <lamMax_nm> [qe_file]");
    optConstCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    optDispCmd = new G4UIcmdWithAString("/sd/optics/disp", this);
    optDispCmd->SetGuidance("Set dispersive optics: <det> <refr_file> <qe_file> [dlam_nm]");
    optDispCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    optClearCmd = new G4UIcmdWithAString("/sd/optics/clear", this);
    optClearCmd->SetGuidance("Clear optics for <det> or 'all'");
    optClearCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // scintillation assignment UI
    scintConstCmd = new G4UIcmdWithAString("/sd/scint/const", this);
    scintConstCmd->SetGuidance("Set scintillation with a flat efficiency:");
    scintConstCmd->SetGuidance("  <det> <S_perMeV> <kB_mm/MeV> [eff]");
    scintConstCmd->SetGuidance("  S   absolute light yield in photons/MeV");
    scintConstCmd->SetGuidance("  kB  Birks constant in mm/MeV (e.g. 0.126 for plastic)");
    scintConstCmd->SetGuidance("  eff collection x quantum efficiency, 0..1 (default 1)");
    scintConstCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    scintSpecCmd = new G4UIcmdWithAString("/sd/scint/spec", this);
    scintSpecCmd->SetGuidance("Set scintillation with a spectrum-folded efficiency:");
    scintSpecCmd->SetGuidance("  <det> <S_perMeV> <kB_mm/MeV> <emission_file> <qe_file> [dlam_nm]");
    scintSpecCmd->SetGuidance("  emission_file  lambda[nm] I(lambda), arbitrary units");
    scintSpecCmd->SetGuidance("  qe_file        lambda[nm] QE(lambda), fraction or percent");
    scintSpecCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    scintChouCmd = new G4UIcmdWithAString("/sd/scint/chou", this);
    scintChouCmd->SetGuidance("Set the second-order Birks/Chou term: <det> <C_mm2/MeV2>");
    scintChouCmd->SetGuidance("Applies to whichever scintillation model <det> uses.");
    scintChouCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    scintClearCmd = new G4UIcmdWithAString("/sd/scint/clear", this);
    scintClearCmd->SetGuidance("Clear scintillation for <det> or 'all'");
    scintClearCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
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
        this->WriteFormatString();
    }
    else if (cmd == stateCmd)
        this->WriteRandomState(val);
    else if (cmd == stateLCmd)
        this->LoadRandomState(val);
    else if (cmd == optConstCmd) {
        // /sd/optics/const <det> <n> <lamMin_nm> <lamMax_nm> [qe_file]
        const std::string sval = val;   
        std::istringstream iss(sval);  
        std::string det, qe;
        double n, lmin, lmax;
        if (!(iss >> det >> n >> lmin >> lmax)) {
          cfgReject("OpticsConst","Usage: /sd/optics/const <det> <n> <lamMin_nm> <lamMax_nm> [qe_file]");
          return;
        }
        if (n <= 1.0) {
          cfgReject("OpticsConst","Refractive index must be > 1, no Cherenkov light otherwise");
          return;
        }
        if (lmin <= 0.0 || lmax <= lmin) {
          cfgReject("OpticsConst","Need 0 < lamMin_nm < lamMax_nm");
          return;
        }
        if (iss >> qe && !opticsFileReadable(qe.c_str(), "QE")) return;
        OpticsConfig& cfg = opticsMap[det.c_str()];
        cfg.type = OpticsType::ConstN;
        cfg.cn.n = n;
        cfg.cn.lamMin_nm = lmin;
        cfg.cn.lamMax_nm = lmax;
        cfg.cn.qePath = qe.c_str();
        cfg.cn.qeLoaded = false;
    }
    else if (cmd == optDispCmd) {
        // /sd/optics/disp <det> <refr_file> <qe_file> [dlam_nm]
        const std::string sval = val;  
        std::istringstream iss(sval);   
        std::string det, refr, qe;
        double dlam=1.0;
        if (!(iss >> det >> refr >> qe)) {
          cfgReject("OpticsDisp","Usage: /sd/optics/disp <det> <refr_file> <qe_file> [dlam_nm]");
          return;
        }
        if (!optionalDouble(iss, dlam) || dlam <= 0.0) {
          cfgReject("OpticsDisp","dlam_nm must be a number > 0");
          return;
        }
        if (!opticsFileReadable(refr.c_str(), "refractive") ||
            !opticsFileReadable(qe.c_str(), "QE")) return;
        OpticsConfig& cfg = opticsMap[det.c_str()];
        cfg.type = OpticsType::Dispersive;
        cfg.disp.refrPath = refr.c_str();
        cfg.disp.qePath   = qe.c_str();
        cfg.disp.dLam_nm  = dlam;
        cfg.disp.loaded   = false;
    }
    else if (cmd == optClearCmd) {
        std::string what = std::string(val);
        if (what=="all") opticsMap.clear();
        else             opticsMap.erase(what.c_str());
    }
    else if (cmd == scintConstCmd) {
        // /sd/scint/const <det> <S_perMeV> <kB_mm/MeV> [eff]
        const std::string sval = val;  
        std::istringstream iss(sval);  
        std::string det;
        double S, kB, eff = 1.0;
        if (!(iss >> det >> S >> kB)) {
          cfgReject("ScintConst","Usage: /sd/scint/const <det> <S_perMeV> <kB_mm/MeV> [eff]");
          return;
        }
        if (S < 0.0 || kB < 0.0) {
          cfgReject("ScintConst","S_perMeV and kB_mm/MeV must be >= 0");
          return;
        }
        if (!optionalDouble(iss, eff) || eff < 0.0 || eff > 1.0) {
          cfgReject("ScintConst","eff must be a number in [0,1]");
          return;
        }
        ScintConfig& cfg = scintMap[det.c_str()];
        cfg.type = ScintType::Const;
        cfg.cs.S_perMeV      = S;
        cfg.cs.kB_mm_per_MeV = kB;
        cfg.cs.eff           = eff;
    }
    else if (cmd == scintSpecCmd) {
        // /sd/scint/spec <det> <S_perMeV> <kB_mm/MeV> <emission_file> <qe_file> [dlam_nm]
        const std::string sval = val;   
        std::istringstream iss(sval); 
        std::string det, emis, qe;
        double S, kB, dlam = 1.0;
        if (!(iss >> det >> S >> kB >> emis >> qe)) {
          cfgReject("ScintSpec","Usage: /sd/scint/spec <det> <S_perMeV> <kB_mm/MeV> <emission_file> <qe_file> [dlam_nm]");
          return;
        }
        if (S < 0.0 || kB < 0.0) {
          cfgReject("ScintSpec","S_perMeV and kB_mm/MeV must be >= 0");
          return;
        }
        if (!optionalDouble(iss, dlam) || dlam <= 0.0) {
          cfgReject("ScintSpec","dlam_nm must be a number > 0");
          return;
        }
        if (!opticsFileReadable(emis.c_str(), "emission spectrum") ||
            !opticsFileReadable(qe.c_str(), "QE")) return;
        ScintConfig& cfg = scintMap[det.c_str()];
        cfg.type = ScintType::Spectral;
        cfg.sp.S_perMeV      = S;
        cfg.sp.kB_mm_per_MeV = kB;
        cfg.sp.emisPath      = emis.c_str();
        cfg.sp.qePath        = qe.c_str();
        cfg.sp.dLam_nm       = dlam;
        cfg.sp.loaded        = false;
        cfg.sp.effCached     = 0.0;
    }
    else if (cmd == scintChouCmd) {
        // /sd/scint/chou <det> <C_mm2/MeV2>
        const std::string sval = val;  
        std::istringstream iss(sval);  
        std::string det;
        double C;
        if (!(iss >> det >> C)) {
          cfgReject("ScintChou","Usage: /sd/scint/chou <det> <C_mm2/MeV2>");
          return;
        }
        if (C < 0.0) {
          cfgReject("ScintChou","C_mm2/MeV2 must be >= 0");
          return;
        }
        
        scintMap[det.c_str()].chou_C = C;
    }
    else if (cmd == scintClearCmd) {
        std::string what = std::string(val);
        if (what=="all") scintMap.clear();
        else             scintMap.erase(what.c_str());
    }
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
    static G4ThreadLocal std::ostringstream *oss = nullptr;
    if (!oss) oss = new std::ostringstream;
    oss->str(std::string());
    oss->clear();

    for(auto &fmt : this->format) {
        fmt(hit, *oss);
        *oss << " ";
    }
    std::string s = oss->str();
    if (!s.empty()) s.pop_back();
    return s;
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
