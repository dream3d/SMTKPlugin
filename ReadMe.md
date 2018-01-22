# Introduction #

SMTKPlugin is a DREAM3D plugin that uses Kitware's [Simulation Model Toolkit (SMTK)][SMTK], an open-source, multi-platform toolkit for supporting simulation workflows by providing access to different modeling kernels, mesh databases, and simulation infomation (via its attribute system).

## Building SMTK (Mac OS X) ##

instructions are similar for **Linux**. Windows users may need to manually compile all of the dependencies.



### Download, compile, and install GDAL ###

- Download at http://download.osgeo.org/gdal/2.2.3/gdal-2.2.3.tar.gz
- Un-tar the tar.gz file and use Terminal to navigate to the resulting gdal source folder
- Execute command
		./configure --prefix=[PATH_TO_GDAL_INSTALL_FOLDER]
- Execute command
		make
- Execute command
		make install

### Download and compile VTK ###

	- Download at http://www.vtk.org/files/release/8.0/VTK-8.0.1.tar.gz
	- Un-tar the tar.gz file and use Terminal and run CMake on the resulting VTK source folder
	- Run CMake using the following command:

		cmake -G Ninja -DVTK_Group_Qt=ON -DVTK_Group_Views=ON -DVTK_QT_VERSION=5 -DQt5_DIR=[PATH_TO_Qt5_CONFIG_FILE] -DModule_vtkIOGDAL=ON -DModule_vtkIOParallelExodus=ON -DModule_vtkIOXdmf2=ON -DModule_vtkRenderingMatplotlib=ON -DModule_vtkTestingRendering=ON -DGDAL_INCLUDE_DIR=[PATH_TO_GDAL_INCLUDE_FOLDER] -DGDAL_LIBRARY=[PATH_TO_GDAL_LIBRARY_FILE] -DVTK_USE_SYSTEM_HDF5=ON -DHDF5_ROOT=[PATH_TO_HDF5_ROOT_FOLDER] [PATH_TO_VTK_SOURCE_FOLDER]

- Execute command
		ninja


### Download and compile JLohmann JSON Library ###

+ Use git to clone from

	[user]$ git clone ssh://git@github.com/nlohmann/json nlohmann_json
   	[user]$ cd nlohmann_json
    [user]$ git fetch --all --tags
    [user]$ git checkout v3.0.1
    [user]$ mkdir Debug
    [user]$ cd Debug
    [user]$ cmake -G Ninja -DCMAKE_INSTALL_PREFIX=/Users/Shared/DREAM3D_SDK/NLohmanJson -DBUILD_TESTING=OFF  ../
    [user]$ ninja install



### Download and install Boost ###

- Download at https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
- Un-tar the tar.gz file and use Terminal to navigate to the resulting Boost source folder
- Execute command

		./bootstrap.sh --prefix=[PATH_TO_BOOST_INSTALL_FOLDER]

- Execute command

		./b2 install

### Clone SMTK from Github ###

- Clone SMTK from https://github.com/Kitware/SMTK.git.  Use the --recursive flag.
- Navigate to the repo directory that was just cloned and open smtk/extension/CMakeLists.txt.  Add vtkIOGeometry and vtkIOPLY to the required VTK modules section.  This should be around lines 31-46.

- Run CMake on SMTK using the following command:

		cmake -G Ninja -DVTK_DIR=/Users/Shared/DREAM3D_SDK/VTK-8.1.0-Debug -DBOOST_ROOT=/Users/Shared/DREAM3D_SDK/boost-1.66.0 -DGDAL_INCLUDE_DIR=/Users/Shared/DREAM3D_SDK/gdal-2.2.3/include -DGDAL_LIBRARY=/Users/Shared/DREAM3D_SDK/gdal-2.2.3/lib/libgdal.dylib  -DSMTK_ENABLE_TESTING=OFF -DSMTK_ENABLE_QT_SUPPORT=ON -DENABLE_HDF5=ON -DHDF5_DIR=/Users/Shared/DREAM3D_SDK/hdf5-1.8.19-Debug/share/cmake -Dnlohmann_json_DIR=/Users/Shared/DREAM3D_SDK/NLohmanJson/lib/cmake/nlohmann_json -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/Users/Shared/DREAM3D_SDK/SMTK-Debug ../

- Execute command

		ninja