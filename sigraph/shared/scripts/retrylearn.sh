#!/bin/bash

bin="$(basename $0)"
if [ "$#" != "4" ];
then
	echo     "usage : $bin hostname num dbdir version "
	echo
	echo -en "  num:\ttask number"
	echo -en "  dbdir:\tshould be an absolute path"
	echo
	echo     "example: $bin pervehan 7 \$(pwd) -2006_02_28"
	exit 1
fi

hostname=$1
num=$2
dbdir=$3
ver=$4
firsthostname="$(basename $(ls -1 $dbdir/siPartLearn-$num-*.cfg) | sed 's/.*-//g;s/\.cfg//g')"

echo "run task $num (originally handle by '$firsthostname') on '$hostname'..."
rm -f loglearn-$num-$hostname.log

rsh $hostname "cd $dbdir; nice siPIDcommand$ver PID-$num-$hostname siLearn$ver siPartLearn-$num-$firsthostname.cfg >& loglearn-$num-$hostname.log"
