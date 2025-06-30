[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.15772294.svg)](https://doi.org/10.5281/zenodo.15772294)

# G4VM

## About
The G4VM (GEANT4 Virtual Machine) aims to make GEANT4 instrument simulations of SOHO/Chandra EPHIN and Solar Orbiter HET readily accessible.

It contains the corresponding GDML models along with a compatible GEANT4 application.
The models  incorporate the sensitive detectors of the instruments and the passive material shielding the detectors, represented with high precision.
Each instrument is provided individually and with a spacecraft model that maps the spacecraft's shielding material onto spherical shell segments with varying densities to account for directionality.

The GEANT4 application can be built independently but is provided in a configuration that works seamlessly within the recommended setup.

## Recommended Setup

Our recommended setup requires three resources:

1. A [Virtualization Software](https://www.vmware.com/products/workstation-player/workstation-player-evaluation.html) (VMware Workstation Player 17)
2. A [Geant4 Virtual Machine](https://extra.lp2ib.in2p3.fr/G4/download/)
3. The [GEANT-4 application](https://github.com/spearhead-he/G4VM/) source code (found here)

---

## Virtual Machine Setup

Here we describe the steps needed to set up a machine-independent virtual machine (VM).

### Virtualization Software

Virtualization software needs to be installed. For details, follow the description at [the G4VM](https://extra.lp2ib.in2p3.fr/G4/tutorial-2/).  
The proposed setup has been tested using VMware Workstation Player 17 obtained [from VMware](https://www.vmware.com/products/desktop-hypervisor/workstation-and-fusion) on macOS and Windows 11. However, an installation using [VirtualBox](https://www.virtualbox.org/wiki/Downloads) is also supported.

### VM Setup

- **VM Download**: The compressed VM can be downloaded from [G4VM](https://extra.lp2ib.in2p3.fr/G4/download/). Ensure at least **~10GB** of space compressed and **~20GB** decompressed on your hard disk.  
  *The release used throughout this manual is Geant4.11.2.1 as of 13/03/2024.*

- **Preparation**: Before starting the VM, you might want to modify settings such as the number of cores for multithreading or enable directory sharing with the host system. Refer to your VM manual for details.

- **Installation**:  
  1. Decompress the VM files.
  2. Start VMware Workstation Player and initialize the VM by clicking the "Open a Virtual Machine" button to select the VM configuration file.
  3. The VM should now be visible in the VMware Player library.

- **First Start**: After powering on the machine for the first time, select *"I copied it"* when asked whether you copied or moved the VM. You now have access to a fully operational Linux system with GEANT-4.  
  Default user account:  
  - **Username**: `local1`  
  - **Password**: `local1`  
  - **Root Password**: `rocky9`  
  > **Warning**: The default keyboard layout is French and might need adjustment in the upper-right panel.

---

## Obtaining and Running the GEANT-4 Applications

Follow these steps to get the GEANT-4 application running:

### Terminal

1. Open a terminal via `Activities -> Terminal`.  
   > **Warning**: The default shell is Tcsh.

### SPEARHEAD Application

2. Clone the repository containing the GEANT-4 applications using:
   ```csh
   git clone https://github.com/spearhead-he/G4VM.git
   ```

3. Navigate to the build directory of the GEANT-4 application:
   ```csh
   cd G4VM/G4VM_build/
   ```

#### ARM-Based Host System:
On ARM systems first compile the binarys by running:    
```csh
make
```

A successful compilation will output:
```csh
[100%] Built target Spearhead_G4VM
```
You can now proceed as on Intel-Based Host systems.

#### Intel-Based Host System:

Run the GEANT-4 application in QT/UI mode using:
 ```csh
 ./Spearhead_G4VM
 ```

And to run the GEANT-4 application with a macro file use:
```csh
./Spearhead_G4VM <path_to_macro>
```
    
---

## Acknowledgement

<img align="right" height="80px" src="https://github.com/user-attachments/assets/28c60e00-85b4-4cf3-a422-6f0524c42234"> 
<img align="right" height="80px" src="https://github.com/user-attachments/assets/854d45ef-8b25-4a7b-9521-bf8bc364246e"> 

This tool is developed within the SPEARHEAD (*SPEcification, Analysis & Re-calibration of High Energy pArticle Data*) project. SPEARHEAD has received funding from the European Union’s Horizon Europe programme under grant agreement No 101135044. 

The tool reflects only the authors’ view and the European Commission is not responsible for any use that may be made of the information it contains.
