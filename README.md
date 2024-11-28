# G4VM


## Recommended Setup

Our recommended setup requires three resources:

1. A [Virtualization Software](https://www.vmware.com/products/workstation-player/workstation-player-evaluation.html.html) (VMware Workstation Player 17)
2. A [Geant4 Virtual Machine](https://extra.lp2ib.in2p3.fr/G4/download/)
3. The [GEANT-4 application](https://cau-git.rz.uni-kiel.de/ieap/et/hap/spearhead) source code

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

## Getting and Running the GEANT-4 Applications

Follow these steps to get the GEANT-4 application running:

### Terminal

1. Open a terminal via `Activities -> Terminal`.  
   > **Warning**: The default shell is Tcsh.

### SPEARHEAD Application

2. Clone the repository containing the GEANT-4 applications using:
   ```csh
   git clone https://cau-git.rz.uni-kiel.de/ieap/et/hap/spearhead.git

3. Navigate to the build directory of the desired GEANT-4 application, e.g.:
   ```csh
   cd /spearhead/WP2/basic_sensi_build/

#### Intel-Based Host System

Run the GEANT-4 application in QT/UI mode using:
    ```csh

    ./basic-sensi-gdml

And to run the GEANT-4 application with a macro file use:
    '''csh
    
    ./basic-sensi-gdml <path_to_macro>

#### ARM-Based Host System
On ARM systems, first compile the binarys by running:    
    '''csh

    make

A successful compilation will output:
    '''csh

    [100%] Built target basic-sensi-gdml
    


