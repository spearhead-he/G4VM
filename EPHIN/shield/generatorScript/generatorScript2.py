import numpy as np
import os

def writePhysVol(theta, phi):
    with open('../shield_physvol.xml', 'a') as f:
        f.write("<physvol>\n
                \t<volumeref ref='name"+str(theta)+"to"+str(phi)+"' />\n
                \t<position name='pos"+str(theta)+"to"+str(phi)+"' unit='cm' x='0' y='0' z='8.765'/>\n
                </physvol>\n")

def writeStructure(theta, phi, sensi = False):
    with open('../shield_structure.xml', 'a') as f:
        f.write("<volume name='name"+str(theta)+"to"+str(phi)+"'>\n
                \t<materialref ref='density"+str(theta)+"to"+str(phi)+"'/>\n
                \t<solidref ref='ShieldElement'/>\n")
        if sensi:
            f.write("\t<auxiliary auxtype='sensi' auxvalue='"+str(theta)+"to"+str(phi)+"' auxunit='id'/>\n")
        f.write("</volume>\n")
        
def writeMaterial(theta, phi, length):
    with open('../shield_materials.xml', 'a') as f:
        f.write("<material name='density"+str(theta)+"to"+str(phi)+"' formula='Al' Z='13'>\n
                \t<D value='"+str(0.0573*length/10.)+"'/>\n
                \t<atom value='26.98'/>\n</material>\n")


def writeSolid(theta, phi):








data = np.loadtxt('./geantino.0.tracks', usecols = [1, 2, 3, 4, 5, 6, 7], skiprows = 1)

ind = np.where(data[:,0] == 666.)[0]
print(data[ind][:,0])
data = data[ind][:,[2,3,1]]

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



for dat in data:
    writePhysVol(dat[0],dat[1])
    writeStructure(dat[0],dat[1])
    with open('../shield_materials.xml', 'a') as f:
        f.write('<materials>')
    writeMaterial(dat[0],dat[1],dat[2])
    with open('../shield_materials.xml', 'a') as f:
        f.write('</materials>')
