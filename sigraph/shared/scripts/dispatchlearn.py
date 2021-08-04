#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sys, os, shutil

powermode = 0
machines = []
progsuffix = ""
divname = "siDivNameList"
silearn = "siLearn"
pidcom = "siPIDcommand"

i = 1
while i < len( sys.argv ):
    arg = sys.argv[ i ]
    if arg  == '-p' or arg == '--power':
        powermode = 1
    elif arg == "-d" or arg == "--devel":
        progsuffix = sys.argv[i+1]
        i += 1
    else:
        if powermode:
            machines.append( ( arg, float( sys.argv[i+1] ) ) )
            i = i + 1
        else:
            machines.append( ( arg, 1 ) )
    i = i + 1

divname += progsuffix
silearn += progsuffix
pidcom += progsuffix
print('siLearn command:', silearn)

print("machines :")
print(machines)
if len( machines ) == 0:
    print("usage: ", sys.argv[0], " [-d|--devel] [-p|--power] machine1 ",
    "[power1] machine2 [power2]...")
    print("dispatches sigraph model learning over a set of machines")
    print("-d: use devel version of learning command lines (si*-dev)")
    print("-p: specify power index for each machine (default: all set to 1)")
    print("[power1], [power2]....: if -p is given, power index associated with")
    print("preceding machine")
    sys.exit( 1 )

npow = 0
i = 0
while i < len( machines ):
    npow += machines[i][1]
    i += 1

print("power sum: ", npow)

file = os.popen( "ls model/adap/*.mod | wc" )
n = file.readlines()
file.close()
if len( n ) == 0:
    print("could not find nodes models")
    sys.exit( 1 )
nnodes = int( n[0].split()[0] ) - 1
print("nodes: ", nnodes)

file = os.popen( "ls model/edges/*.mod | wc" )
n = file.readlines()
file.close()
if len( n ) == 0:
    print("could not find relations models")
    sys.exit( 1 )
nedges = int( n[0].split()[0] )
print("edges: ", nedges)

mnodes = nnodes * npow / ( nnodes + nedges )
print(mnodes)
medges = npow - mnodes
print(medges)

nodemach = 0
nodepow = 0
while nodemach < len( machines ):
    nodepow_old = nodepow
    nodepow += machines[nodemach][1]
    if nodepow < mnodes:
        nodemach += 1
    else:
        if abs( nodepow_old - mnodes ) < abs( nodepow - mnodes ) \
           and nodemach > 0:
            nodemach -= 1
            nodepow = nodepow_old
        break

nodemach += 1
print("machines for nodes: ", nodemach, "(power: ", nodepow, ")")

i = 0
cpustr = ""
while i < nodemach:
    if len( cpustr ) > 0:
        cpustr += " "
    cpustr += str( machines[i][1] )
    i += 1
print("dividing nodes...")
os.system( divname + " -c model/adap /tmp/nodes " + cpustr )

cpustr = ""
while i < len( machines ):
    if len( cpustr ) > 0:
        cpustr += " "
    cpustr += str( machines[i][1] )
    i += 1
print("dividing edges...")
os.system( divname + " -c -p edg model/edges /tmp/edges " + cpustr )

cfgbase = 'siPartLearn-'
dir = os.getcwd()

i = 0
for m in machines:
    cfg = cfgbase + str(i) + '-' + m[0] + ".cfg"
    print("config: ", cfg)
    log = "loglearn-" + str(i) + '-' + m[0] + ".log"
    print("log file: ", log)
    cmd = "cd " + dir + "; nice " + pidcom + " PID-" + str(i) + '-' \
          + m[0] + " " + silearn + " " + cfg + " >& " + log
    print("command:")
    print(cmd)
    try:
        os.remove( log )
    except:
        pass
    shutil.copy( "siLearnTempl.cfg", cfg )
    lfile = open( cfg, "a" )
    if i < nodemach:
        lfile.write( 'filter_attributes label\n' )
        cfile = "/tmp/nodes." + str(i)
    else:
        lfile.write( 'filter_attributes label1\n' )
        cfile = "/tmp/edges." + str( i - nodemach )
    file = open( cfile )
    lines  = file.readlines()
    file.close()
    if len( lines ) == 0 or ( len( lines ) == 1 and lines[0] == '' ):
        print("empty param file ", cfile)
        lfile.close()
        os.remove( cfile )
    else:
        lfile.write( "filter_pattern " + lines[0] )
        lfile.write( '*END\n' )
        lfile.close()
        os.remove( cfile )
        rshcom = "rsh " + m[0] + ' "' + cmd + '" &'
        print(rshcom)
        os.system( rshcom )
    i += 1
