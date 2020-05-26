# Recipe

```
scl enable devtoolset-8 bash
source /nfs/slac/g/ldmx/software/setup_gcc8.3.1_cos7.sh

# make a working dir and clone
mkdir mytest; cd mytest;

git clone https://github.com/LDMX-Software/ldmx-sw.git -b nvt/hcaldigi
cd ldmx-sw; mkdir build; cd build
cmake -DGeant4_DIR=$G4DIR -DROOT_DIR=$ROOTDIR -DXercesC_DIR=$XercesC_DIR -DPYTHON_EXECUTABLE=`which python` -DPYTHON_INCLUDE_DIR=$PYTHONHOME/include/python2.7 -DPYTHON_LIBRARY=$PYTHONHOME/lib/libpython2.7.so -DCMAKE_INSTALL_PREFIX=../install ..
make -j16 install

# out of build directory
cd .. 
# set new path to your ldmx-sw install
source install/bin/ldmx-setup-env.sh 
# this is broken in the cmake build
source /nfs/slac/g/ldmx/software/root-6.18.04/install_gcc8.3.1_cos7/bin/thisroot.sh 

# make a directory for interactive testing
cd test
ldmx-app simpleSim.cfg
```