#!/bin/bash

GCC_VERSION=7.3.0
GLIBC_VERSION=2.25

setup_gcc () {
	# As best practice, each function sets its $CWD at its start
	cd $LDMX_BUILDDIR
	msg_info "Started at `date`. Current gcc:"
	gcc --version
	local PKG_VERSION=$GCC_VERSION
	local PKG_NAME='gcc'
	local PKG_DIR="$LDMX_LIBDIR/$PKG_NAME-$PKG_VERSION"
	local KIT_DIR="$LDMX_BUILDDIR/$PKG_NAME-$PKG_VERSION"

	# existence test
	if [ -f "$PKG_DIR/bin/gcc" ]; then
		already $PKG_NAME-$PKG_VERSION
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
	rm -rf tmp 2> /dev/null
	cd $PKG_NAME-$PKG_VERSION
	./contrib/download_prerequisites
	mkdir -p ../tmp; cd ../tmp
	../$PKG_NAME-$PKG_VERSION/configure --prefix="$PKG_DIR" --disable-multilib
	make -j$NR_CORES
	make install

	if [ $? -eq 0 ]; then
		msg_info "$PKG_NAME-$PKG_VERSION setup successfully ended"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		rm -rf ../tmp 2> /dev/null
		return 1
	fi

	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	rm -rf ../tmp 2> /dev/null
	msg_info "Finished at `date`. Installed gcc:"
	$PKG_DIR/bin/gcc --version
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

	# existence test
	if [ -f "$PKG_DIR/bin/ldd" ]; then
		already $PKG_NAME-$PKG_VERSION
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
		msg_info "$PKG_NAME-$PKG_VERSION setup successfully ended"
	else
		msg_err "$PKG_NAME-$PKG_VERSION setup failed"
		cd ..
		rm -rf tmp 2> /dev/null
		return 1
	fi

	if [ $LDMX_CLEAN = true ]; then 
		rm -rf $KIT_DIR 2> /dev/null
	fi
	cd ..
	rm -rf tmp 2> /dev/null
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

setup_gcc
setup_glibc