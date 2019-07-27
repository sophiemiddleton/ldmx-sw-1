#!/bin/bash

GCC_VERSION=7.3.0
# GCC_VERSION=6.2.0
# GCC requires Aritmethic and FP libraries
GMP_VERSION=6.1.2
# GMP_VERSION=6.1.2
MPFR_VERSION=4.0.2
# MPFR_VERSION=4.0.2
MPC_VERSION=1.1.0
# MPC_VERSION=1.1.0
# GCC requires glibc
GLIBC_VERSION=2.25
# GLIBC_VERSION=2.17
# glibc requires isl
ISL_VERSION=0.21
# ISL_VERSION=0.16.1

# GCC depends on GMP, MPFR, MPC. Note that versions of all 4 are corellated
# As of July '19, geant4 requires cmake 4.7.1+
# cmake 4.7.1 requires GCC 6.2.0+
# GCC 6.2.0 requires GMP 4.2+, MPFR 2.4.0+ and MPC 0.8.0+
# MPC requires GMP 5.0.0+ and MPFR 3.0.0

# So this installs GCC 6.2.0 with the dependencies:
# GMP 6.1.2, MPFR 4.0.2 and MPC 4.0.2
# TODO: install glibc

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

setup_gmp () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$GMP_VERSION
	local PKG_NAME='gmp'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	GMP_DIR=$PKG_DIR

	if [ -f "$PKG_DIR/lib/lib$PKG_NAME.so" -a -f "$PKG_DIR/include/$PKG_NAME.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_gmp
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_gmp () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.xz
	local PKG_MIRROR="https://gmplib.org/download/$PKG_NAME/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
			return 1
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_mpfr () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$MPFR_VERSION
	local PKG_NAME='mpfr'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	MPFR_DIR=$PKG_DIR

	# mpfr depends on gmp. Installer needs path to it 
	setup_gmp
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi

	if [ -f "$PKG_DIR/lib/lib$PKG_NAME.so" -a -f "$PKG_DIR/include/$PKG_NAME.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_mpfr
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR" --with-gmp="$GMP_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_mpfr () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.xz
	local PKG_MIRROR="https://www.mpfr.org/mpfr-current/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_mpc () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$MPC_VERSION
	local PKG_NAME='mpc'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	MPC_DIR=$PKG_DIR

	# mpc depends on mpfr. Installer needs path to it 
	setup_mpfr
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi

	if [ -f "$PKG_DIR/lib/lib$PKG_NAME.so" -a -f "$PKG_DIR/include/$PKG_NAME.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_mpc
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR" --with-gmp="$GMP_DIR" --with-mpfr="$MPFR_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_mpc () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.gz
	local PKG_MIRROR="https://ftp.gnu.org/gnu/$PKG_NAME/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_gcc () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$GCC_VERSION
	local PKG_NAME='gcc'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"

	setup_mpc
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	setup_isl
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	setup_glibc
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi

	if [ -f "$PKG_DIR/lib/lib$PKG_NAME.so" -a -f "$PKG_DIR/include/$PKG_NAME.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		rm -rf ../tmp 2> /dev/null
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_gcc
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	# gcc needs custom glibc
	OLDPATH=$PATH
	PATH=$GLIBC_DIR/bin:$GLIBC_DIR/include:$GLIBC_DIR/lib:$GLIBC_DIR/sbin:$PATH
	rm -rf ../tmp 2> /dev/null
	mkdir -p ../tmp; cd ../tmp
	../$PKG_NAME-$PKG_VERSION/contrib/download_prerequisites
	../$PKG_NAME-$PKG_VERSION/configure --prefix="$PKG_DIR" --with-gmp="$GMP_DIR" --with-mpfr="$MPFR_DIR" --with-mpc="$MPC_DIR" --with-isl="$ISL_DIR" --disable-multilib
	make -j$NR_CORES
	make install
	# restore old $PATH
	PATH=$OLDPATH
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_gcc () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.gz
	local PKG_MIRROR="ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/$PKG_NAME-$PKG_VERSION/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_glibc () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$GLIBC_VERSION
	local PKG_NAME='glibc'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	GLIBC_DIR=$PKG_DIR

	if [ -f "$PKG_DIR/bin/ldd" ]; then
		already $PKG_NAME-$PKG_VERSION
		rm -rf tmp 2> /dev/null
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_glibc
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	# glibc wants us to build in a different dir from its kit
	# See https://www.gnu.org/software/libc/manual/html_mono/libc.html#Configuring-and-compiling
	rm -rf tmp 2> /dev/null
	mkdir tmp; cd tmp
	../$PKG_NAME-$PKG_VERSION/configure --prefix="$PKG_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_glibc () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.xz
	local PKG_MIRROR="https://ftp.fau.de/gnu/$PKG_NAME/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
			return 1
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_isl () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$ISL_VERSION
	local PKG_NAME='isl'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	ISL_DIR=$PKG_DIR

	# isl needs gmp (existence check)
	setup_gmp
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi

	# unlike other libs, isl appears to not have a header name to check the existence of
	# so I'm using a random one
	if [ -f "$PKG_DIR/lib/lib$PKG_NAME.so" -a -f "$PKG_DIR/include/isl/stdint.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi

	download_isl
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR" --with-gmp-prefix="$GMP_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_isl () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.xz
	local PKG_MIRROR="http://isl.gforge.inria.fr/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
			return 1
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_make () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$MAKE_VERSION
	local PKG_NAME='make'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	MAKE_DIR=$PKG_DIR

	if [ -f "$PKG_DIR/include/gnu$PKG_NAME.h" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_make
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	# glibc needs to see gmake
	ln -s $MAKE_DIR/bin/$PKG_NAME $MAKE_DIR/bin/gmake
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_make () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.gz
	local PKG_MIRROR="https://ftp.fau.de/gnu/$PKG_NAME/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
			return 1
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_bison () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	local PKG_VERSION=$BISON_VERSION
	local PKG_NAME='bison'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"
	BISON_DIR=$PKG_DIR

	if [ -f "$PKG_DIR/bin/$PKG_NAME" ]; then
		already $PKG_NAME-$PKG_VERSION
		if [ $LDMX_CLEAN = true ]; then 
			rm -rf $KIT_DIR 2> /dev/null
		fi
		return 0
	fi
	download_bison
	if [ $? -eq 1 ]; then
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	msg_info "$PKG_NAME-$PKG_VERSION setup start"
	cd $PKG_NAME-$PKG_VERSION
	./configure --prefix="$PKG_DIR"
	make -j$NR_CORES
	make install
	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup end"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		return 1
	fi
	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	return 0
}

download_bison () {
	# downloading PKG
	cd $LDMX_BUILDDIR
	local PKG_TAR=$PKG_NAME-$PKG_VERSION.tar.xz
	local PKG_MIRROR="https://mirrors.dotsrc.org/gnu/$PKG_NAME/$PKG_TAR"
	
	if [ -d "$PKG_NAME-$PKG_VERSION" ]; then
		# dir exists. rm tar
		rm $PKG_TAR 2> /dev/null
		return 0
	fi
	if [ -f "$PKG_NAME-$PKG_TAR" ]; then
        # dir doesn't exist, tar exists. Untar
		$TAR $PKG_TAR
        rm $PKG_TAR
	else
		# neither exists. wget; untar; rm tar
		wget $PKG_MIRROR
		# checking if download worked
		if [ $? -gt 0 ]; then 
			mirror_error $PKG_MIRROR
			return 1
		fi
		$TAR $PKG_TAR
		rm $PKG_TAR
	fi
	return 0
}

setup_gcc