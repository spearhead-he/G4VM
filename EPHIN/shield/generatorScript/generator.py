import numpy as np
import os

counter = 1000

def calcTheta(x, y, z):
    return np.arccos(z / np.sqrt(x**2 + y**2 + z**2))

def writePhysVol(theta, phi):
    if np.around(phi * 180 / np.pi) == 0 and np.around(theta * 180 / np.pi) == 0:
        with open('../shield_physvol.xml', 'a') as f:
            f.write("<physvol>\n")
            f.write("\t<volumeref ref='name"+str(theta)+"to"+str(phi)+"' />\n")
            f.write("\t<position name='pos"+str(theta)+"to"+str(phi)+"' unit='cm' x='0' y='0' z='8.765'/>\n")
            f.write("</physvol>\n")
        return None
    phi = float(np.around(phi*180./np.pi)) - 2.5
    theta = float(np.around(theta*180/np.pi)) - 2.5
    if theta < 0:
        theta = theta * -1
        phi = phi + 180
    if phi == -182.5:
        #print('Here')
        return None

    with open('../shield_physvol.xml', 'a') as f:
        f.write("<physvol>\n")
        f.write("\t<volumeref ref='name"+str(theta)+"to"+str(phi)+"' />\n")
        f.write("\t<position name='pos"+str(theta)+"to"+str(phi)+"' unit='cm' x='0' y='0' z='8.765'/>\n")
        f.write("</physvol>\n")
    return None

def writeStructure(theta, phi, sensi = False):
    global counter
    if np.around(phi * 180 / np.pi) == 0 and np.around(theta * 180 / np.pi) == 0:
        with open('../shield_structure.xml', 'a') as f:
            f.write("<volume name='name"+str(theta)+"to"+str(phi)+"'>\n")
            f.write("\t<materialref ref='density"+str(theta)+"to"+str(phi)+"'/>\n")
            f.write("\t<solidref ref='ShieldElement"+str(theta)+"to"+str(phi)+"'/>\n")
            if sensi:
                f.write("\t<auxiliary auxtype='sensi' auxvalue='"+str(counter)+"' auxunit='id'/>\n")
                counter = counter + 1
            f.write("</volume>\n")
        return None
    phi = float(np.around(phi*180./np.pi)) - 2.5
    theta = float(np.around(theta*180/np.pi)) - 2.5
    if theta < 0:
        theta = theta * -1
        phi = phi + 180
    if phi == -182.5:
        return None
    with open('../shield_structure.xml', 'a') as f:
        f.write("<volume name='name"+str(theta)+"to"+str(phi)+"'>\n")
        f.write("\t<materialref ref='density"+str(theta)+"to"+str(phi)+"'/>\n")
        f.write("\t<solidref ref='ShieldElement"+str(theta)+"to"+str(phi)+"'/>\n")
        if sensi:
            f.write("\t<auxiliary auxtype='sensi' auxvalue='"+str(counter)+"' auxunit='id'/>\n")
            counter = counter + 1
        f.write("</volume>\n")
        return None

def writeMaterial(theta, phi, length):
    if np.around(phi * 180 / np.pi) == 0 and np.around(theta * 180 / np.pi) == 0:
        with open('../shield_materials.xml', 'a') as f:
            f.write("<material name='density"+str(theta)+"to"+str(phi)+"' formula='Al' Z='13'>\n")
            f.write("\t<D value='"+str(0.0573*length/10.)+"'/>\n")
            f.write("\t<atom value='26.98'/>\n</material>\n")
        return None
    phi = float(np.around(phi*180./np.pi)) - 2.5
    theta = float(np.around(theta*180/np.pi)) - 2.5
    if theta < 0:
        theta = theta * -1
        phi = phi + 180
    if phi == -182.5:
        return None
    with open('../shield_materials.xml', 'a') as f:
        f.write("<material name='density"+str(theta)+"to"+str(phi)+"' formula='Al' Z='13'>\n")
        f.write("\t<D value='"+str(0.0573*length/10.)+"'/>\n")
        f.write("\t<atom value='26.98'/>\n</material>\n")
    return None

def writeSolid(theta, phi):
    if np.around(phi * 180 / np.pi) == 0 and np.around(theta * 180 / np.pi) == 0:
        with open('../shield_solids.xml', 'a') as f:
            f.write("<sphere name='ShieldElement"+str(theta)+"to"+str(phi)+"' rmin='10' rmax='11' starttheta='0' deltatheta='2.5' startphi='0' deltaphi='360' aunit='deg' lunit='cm'/>\n")
        return None
    phi = float(np.around(phi*180./np.pi)) - 2.5
    theta = float(np.around(theta*180/np.pi)) - 2.5
    if theta < 0:
        theta = theta * -1
        phi = phi + 180
    if phi == -182.5:
        return None
    if (phi * 10) % 25 != 0 or (theta * 10) % 25 != 0:
        print("Weird phi or theta at theta = " + str(theta) + ", phi = " + str(phi))
    with open('../shield_solids.xml', 'a') as f:
        f.write("<sphere name='ShieldElement"+str(theta)+"to"+str(phi)+"' rmin='10' rmax='11' starttheta='"+str(theta)+"' deltatheta='5' startphi='"+str(phi)+"' deltaphi='5' aunit='deg' lunit='cm'/>\n")
    return None
   








data = np.loadtxt('./geantino.0.tracks', usecols = [1, 2, 3, 4, 5, 6, 7], skiprows = 1)

ind = np.where(data[:,0] == 666.)[0]
#print(data[ind][:,0])
data = data[ind][:,[3,2,1]]

#print(data)
#print(len(data))

if os.path.exists('../shield_physvol.xml'):
    answer = input('delete shield_physvol.xml?\n')
    if answer.upper() == 'Y':
        os.system('rm ../shield_physvol.xml')

if os.path.exists('../shield_structure.xml'):
    answer = input('delete shield_structure.xml?\n')
    if answer.upper() == 'Y':
        os.system('rm ../shield_structure.xml')

if os.path.exists('../shield_materials.xml'):
    answer = input('delete shield_materials.xml?\n')
    if answer.upper() == 'Y':
        os.system('rm ../shield_materials.xml')

if os.path.exists('../shield_solids.xml'):
    answer = input('delete shield_solids.xml?\n')
    if answer.upper() == 'Y':
        os.system('rm ../shield_solids.xml')

answer = input("Sensi shield?\n")
if answer.upper() == "Y":
    sensi = True
else:
    sensi = False

for dat in data:
    writePhysVol(dat[0],dat[1])
    writeStructure(dat[0],dat[1],sensi)
    #with open('../shield_materials.xml', 'a') as f:
    #    f.write('<materials>')
    writeMaterial(dat[0],dat[1],dat[2])
    #with open('../shield_materials.xml', 'a') as f:
    #    f.write('</materials>')
    writeSolid(dat[0],dat[1])
