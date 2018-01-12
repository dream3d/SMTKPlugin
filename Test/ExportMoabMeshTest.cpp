/* ============================================================================
* Copyright (c) 2018 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Filtering/FilterPipeline.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Plugin/SIMPLibPluginLoader.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Filtering/QMetaObjectUtilities.h"

#include "H5Support/QH5Lite.h"
#include "H5Support/QH5Utilities.h"
#include "H5Support/HDF5ScopedFileSentinel.h"

#include "SMTKPlugin/SMTKPluginFilters/ExportMoabMesh.h"

#include "UnitTestSupport.hpp"

#include "SMTKPluginTestFileLocations.h"

const QString DataContainerName = "SyntheticVolumeDataContainer";
const QString AttributeMatrixName = "CellData";
const QString DataArrayName = "FeatureIdsDoubles";
const QString ErrorDataArrayName = "FeatureIds";

class ExportMoabMeshTest
{

  public:
    ExportMoabMeshTest() {}
    virtual ~ExportMoabMeshTest() {}


  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void RemoveTestFiles()
  {
  #if REMOVE_TEST_FILES
    QFile::remove(UnitTest::ExportMoabMeshTest::VTKOutputFile);
    QFile::remove(UnitTest::ExportMoabMeshTest::HDF5OutputFile);
  #endif
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  int TestFilterAvailability()
  {
    // Now instantiate the ExportMoabMeshTest Filter from the FilterManager
    QString filtName = "ExportMoabMesh";
    FilterManager* fm = FilterManager::Instance();
    IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);
    if (nullptr == filterFactory.get())
    {
      std::stringstream ss;
      ss << "The SMTKPlugin Requires the use of the " << filtName.toStdString() << " filter which is found in the SMTKPlugin Plugin";
      DREAM3D_TEST_THROW_EXCEPTION(ss.str())
    }

    filtName = "DataContainerReader";
    filterFactory = fm->getFactoryForFilter(filtName);
    if (nullptr == filterFactory.get())
    {
      std::stringstream ss;
      ss << "The SMTKPlugin Requires the use of the " << filtName.toStdString() << " filter which is found in Core Filters";
      DREAM3D_TEST_THROW_EXCEPTION(ss.str())
    }

    return 0;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  int TestExportMoabMesh()
  {
    FilterPipeline::Pointer pipeline = FilterPipeline::New();

    QString filtName = "ExportMoabMesh";
    FilterManager* fm = FilterManager::Instance();
    IFilterFactory::Pointer filterFactory = fm->getFactoryForFilter(filtName);

    if (nullptr != filterFactory.get())
    {
      AbstractFilter::Pointer filter = filterFactory->create();
      bool propWasSet;
      QVariant var;

      var.setValue(UnitTest::ExportMoabMeshTest::InputFile);
      propWasSet = filter->setProperty("InputFile", var);
      DREAM3D_REQUIRE_EQUAL(propWasSet, true);

      pipeline->pushBack(filter);
    }
    else
    {
      QString ss = QObject::tr("ExportMoabMeshTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
      DREAM3D_REQUIRE_EQUAL(0, 1);
    }

    filtName = "ExportMoabMesh";
    filterFactory = fm->getFactoryForFilter(filtName);

    if (nullptr != filterFactory.get())
    {
      AbstractFilter::Pointer filter = filterFactory->create();
      bool propWasSet;
      QVariant var;

      pipeline->pushBack(filter);

      pipeline->preflightPipeline();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), -80000);
      filter->setErrorCondition(0);

      var.setValue(DataArrayPath(DataContainerName, AttributeMatrixName, DataArrayName));
      propWasSet = filter->setProperty("SelectedArrayPath", var);
      DREAM3D_REQUIRE_EQUAL(propWasSet, true);

      pipeline->preflightPipeline();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), -20000);
      filter->setErrorCondition(0);

      propWasSet = filter->setProperty("OutputFile", "Foo.ftr");
      DREAM3D_REQUIRE_EQUAL(propWasSet, true);

      pipeline->preflightPipeline();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), -20001);
      filter->setErrorCondition(0);

      var.setValue(UnitTest::ExportMoabMeshTest::VTKOutputFile);
      propWasSet = filter->setProperty("OutputFile", var);
      DREAM3D_REQUIRE_EQUAL(propWasSet, true);

      pipeline->preflightPipeline();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), 0);

      var.setValue(UnitTest::ExportMoabMeshTest::HDF5OutputFile);
      propWasSet = filter->setProperty("OutputFile", var);
      DREAM3D_REQUIRE_EQUAL(propWasSet, true);

      pipeline->preflightPipeline();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), 0);
      pipeline->execute();
      DREAM3D_REQUIRE_EQUAL(filter->getErrorCondition(), 0);

      hid_t fileId = QH5Utilities::openFile(UnitTest::ExportMoabMeshTest::HDF5OutputFile, true);
      DREAM3D_REQUIRE(fileId >= 0);
      HDF5ScopedFileSentinel sentinel(&fileId, true);

      hid_t tsttId = QH5Utilities::openHDF5Object(fileId, "tstt");
      DREAM3D_REQUIRE(tsttId >= 0);
      sentinel.addGroupId(&tsttId);

      hid_t elementsId = QH5Utilities::openHDF5Object(fileId, "elements");
      DREAM3D_REQUIRE(elementsId >= 0);
      sentinel.addGroupId(&elementsId);

      hid_t hex8Id = QH5Utilities::openHDF5Object(fileId, "Hex8");
      DREAM3D_REQUIRE(hex8Id >= 0);
      sentinel.addGroupId(&hex8Id);

      hid_t tagsId = QH5Utilities::openHDF5Object(fileId, "tags");
      DREAM3D_REQUIRE(tagsId >= 0);
      sentinel.addGroupId(&tagsId);

      double* meshDataset = nullptr;
      QH5Lite::readScalarDataset(tagsId, DataArrayName + "_", meshDataset);
    }
    else
    {
      QString ss = QObject::tr("ExportMoabMeshTest Error creating filter '%1'. Filter was not created/executed. Please notify the developers.").arg(filtName);
      DREAM3D_REQUIRE_EQUAL(0, 1)
    }

    return EXIT_SUCCESS;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void operator()()
  {
    int err = EXIT_SUCCESS;

    DREAM3D_REGISTER_TEST( TestFilterAvailability() );

    DREAM3D_REGISTER_TEST( TestExportMoabMesh() )

    DREAM3D_REGISTER_TEST( RemoveTestFiles() )
  }

  private:
    ExportMoabMeshTest(const ExportMoabMeshTest&); // Copy Constructor Not Implemented
    void operator=(const ExportMoabMeshTest&); // Operator '=' Not Implemented


};

