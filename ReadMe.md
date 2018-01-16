Introduction
============
SMTKPlugin is a DREAM3D plugin that uses Kitware's [Simulation Model Toolkit (SMTK)][SMTK], an open-source, multi-platform toolkit for supporting simulation workflows by providing access to different modeling kernels, mesh databases, and simulation infomation (via its attribute system).

Building SMTK (Mac OS X)
============
1.  Download, compile, and install GDAL
	- Download at http://download.osgeo.org/gdal/2.2.3/gdal-2.2.3.tar.gz
	- Un-tar the tar.gz file and use Terminal to navigate to the resulting gdal source folder
	- Execute command
		- ./configure --prefix=[PATH_TO_GDAL_INSTALL_FOLDER]
	- Execute command
		- make
	- Execute command
		- make install

2.  Download and compile VTK
	- Download at http://www.vtk.org/files/release/8.0/VTK-8.0.1.tar.gz
	- Un-tar the tar.gz file and use Terminal and run CMake on the resulting VTK source folder
	- Run CMake using the following command:

		cmake -G Ninja -DVTK_Group_Qt=ON -DVTK_Group_Views=ON -DVTK_QT_VERSION=5 -DQt5_DIR=[PATH_TO_Qt5_CONFIG_FILE] -DModule_vtkIOGDAL=ON -DModule_vtkIOParallelExodus=ON -DModule_vtkIOXdmf2=ON -DModule_vtkRenderingMatplotlib=ON -DModule_vtkTestingRendering=ON -DGDAL_INCLUDE_DIR=[PATH_TO_GDAL_INCLUDE_FOLDER] -DGDAL_LIBRARY=[PATH_TO_GDAL_LIBRARY_FILE] -DVTK_USE_SYSTEM_HDF5=ON -DHDF5_ROOT=[PATH_TO_HDF5_ROOT_FOLDER] [PATH_TO_VTK_SOURCE_FOLDER]

	- Execute command
		- ninja


3.  Download and install Boost
	- Download at https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
	- Un-tar the tar.gz file and use Terminal to navigate to the resulting Boost source folder
	- Execute command
		- ./bootstrap.sh --prefix=[PATH_TO_BOOST_INSTALL_FOLDER]
	- Execute command
		- ./b2 install

4. Clone SMTK from Github
	- Clone SMTK from https://github.com/Kitware/SMTK.git.  Use the --recursive flag.
	- Navigate to the repo directory that was just cloned and open smtk/extension/CMakeLists.txt.  Add vtkIOGeometry and vtkIOPLY to the required VTK modules section.  This should be around lines 31-46.

	- Run CMake on SMTK using the following command:

		cmake -G Ninja -DVTK_DIR=[PATH_TO_VTK_BUILD_FOLDER] -DBOOST_ROOT=[PATH_TO_BOOST_INSTALL_FOLDER] -DGDAL_INCLUDE_DIR=[PATH_TO_GDAL_INSTALL_INCLUDE_FOLDER] -DGDAL_LIBRARY=[PATH_TO_libgdal.20_LIBRARY] -Dnlohmann_json_DIR=[PATH_TO_nlohmann_json_DIR] -DSMTK_ENABLE_TESTING=OFF -DSMTK_ENABLE_QT_SUPPORT=ON -DENABLE_HDF5=ON [PATH_TO_SMTK_SRC_FOLDER]

	- Execute command
		- ninja