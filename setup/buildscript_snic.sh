#!/bin/bash
# Set project and partition
#SBATCH -A snic2019-3-245

# number of cores/slots
# OBS this will reserve such number preventing others to submit
# which can be a waste of resources.
# means the jobs submitted below with srun will be 
# always filling a batch of 200.
# you can increase this number to be above 300 but allocation may become harder.
# If you are the only one running you should definitely do.
#SBATCH -n 1
# Total processing time of all the jobs above
#SBATCH -t 00:80:00

# job name and output file names
#SBATCH -J ldmxbuilder
#SBATCH -o ldmxbuilder_%j.out
#SBATCH -e ldmxbuilder_%j.out

LDMX_BASEDIR='/home/cosminst/ldmx/ldmx_190706/ldmx-sw'
LDMX_BUILDDIR="$LDMX_BASEDIR/build"
LDMX_LIBDIR="$LDMX_BASEDIR/lib"
mkdir -p $LDMX_BUILDDIR
mkdir -p $LDMX_LIBDIR

# set the number of cores to be used by make.
# Override it with the an indentically named variable in subscript where you see the need to
NR_CORES=8
# if enabled, delete library kits after installing the libraries
# keep false if you want to see logs for each libs' installation
LDMX_CLEAN=true
# variable to switch between verbose and silent unarchiving.
# Verbose tends to clutter the logs which wind up being big enough as it is
VERBOSE_TAR=false
if [ $VERBOSE_TAR = true ]; then
    TAR='tar -xvf'
else
    TAR='tar -xf'
fi

# printing bash functions
msg_info () {
	echo "[$(basename $BASH_SOURCE)][INFO]: $1"
}

msg_err () {
	echo "[$(basename $BASH_SOURCE)][ERROR]: $1" 2>&1
}

already () {
	echo "[$(basename $BASH_SOURCE)][INFO]: $1 already installed."
}

mirror_error () {
	echo "[$(basename $BASH_SOURCE)][ERROR]: Mirror \"$1\" is down. Replacement needed"
	exit
}

source buildGCC.sh
