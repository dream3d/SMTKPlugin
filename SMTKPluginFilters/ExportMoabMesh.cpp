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

#include <memory>

#include "ExportMoabMesh.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"

#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"
#include "SIMPLib/Utilities/FileSystemPathHelper.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#ifdef DEBUG
#undef DEBUG
#define DEBUG_ExportMoabMesh
#endif

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/core/CellField.h"
#include "smtk/io/WriteMesh.h"
#include "smtk/io/ExportMesh.h"

#ifdef DEBUG_ExportMoabMesh
#define DEBUG
#endif

#include "Utilities/SIMPLVtkBridge.h"

#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXdmfReader.h"
#include "vtkXMLPolyDataWriter.h"

#include "SMTKPlugin/SMTKPluginConstants.h"
#include "SMTKPlugin/SMTKPluginVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ExportMoabMesh::ExportMoabMesh()
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ExportMoabMesh::~ExportMoabMesh() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::initialize()
{
  clearErrorCode();
  clearWarningCode();
  setCancel(false);

  m_AllowedExtensions.push_back("h5m");
  m_AllowedExtensions.push_back("mhdf");
  m_AllowedExtensions.push_back("vtk");
  m_AllowedExtensions.push_back("vtu");

  m_ExtensionsString = m_AllowedExtensions.join(" *.");
  m_ExtensionsString.prepend("*.");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(SIMPL_NEW_OUTPUT_FILE_FP("Output File", OutputFile, FilterParameter::Parameter, ExportMoabMesh, m_ExtensionsString, "Output"));

  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Double, 1, AttributeMatrix::Type::Cell, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Selected Array", SelectedArrayPath, FilterParameter::RequiredArray, ExportMoabMesh, req));
  }

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  QFileInfo fi(getOutputFile());
  if(fi.suffix().compare("") == 0)
  {
    setOutputFile(getOutputFile().append(".h5m"));
  }
  FileSystemPathHelper::CheckOutputFile(this, "Output File Path", getOutputFile(), true);

  std::vector<size_t> cDims = {1};
  m_SelectedArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DoubleArrayType, AbstractFilter>(this, getSelectedArrayPath(), cDims);
  if (getErrorCondition() < 0)
  {
    return;
  }

  if(nullptr != m_SelectedArrayPtr.lock()) /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  {
    m_SelectedArray = m_SelectedArrayPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::preflight()
{
  // These are the REQUIRED lines of CODE to make sure the filter behaves correctly
  setInPreflight(true); // Set the fact that we are preflighting.
  emit preflightAboutToExecute(); // Emit this signal so that other widgets can do one file update
  emit updateFilterParameters(this); // Emit this signal to have the widgets push their values down to the filter
  dataCheck(); // Run our DataCheck to make sure everthing is setup correctly
  emit preflightExecuted(); // We are done preflighting this filter
  setInPreflight(false); // Inform the system this filter is NOT in preflight mode anymore.
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::execute()
{
  initialize();
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  QFileInfo fi(getOutputFile());

  QDir dir(fi.path());
  if(!dir.mkpath("."))
  {
    QString ss;
    ss = QObject::tr("Error creating parent path '%1'").arg(dir.path());
    setErrorCondition(-101002, ss);
    return;
  }

  DataContainer::Pointer dc = getDataContainerArray()->getDataContainer(getSelectedArrayPath());

  VTK_PTR(vtkDataSet) imageDataPtr = SIMPLVtkBridge::WrapDataContainerAsVtkDataset(dc);
  vtkDataSet* dataSet = imageDataPtr.Get();

  if (!dataSet)
  {
    QString ss = QObject::tr("Could not encapsulate the selected data array inside a VTK dataset.");
    setErrorCondition(-20002, ss);
    return;
  }

  // Construct a mesh manager.
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  // Import the vtkImageData into smtk::mesh.
  smtk::extension::vtk::io::mesh::ImportVTKData imprt;
  smtk::mesh::CollectionPtr collection = imprt(dataSet, manager, getSelectedArrayPath().getDataArrayName().toStdString());

  if (!collection)
  {
    QString ss = QObject::tr("Unable to import the selected data array into an SMTK collection.");
    setErrorCondition(-101003, ss);
    return;
  }

  bool didWrite = false;
  QFileInfo outFi(m_OutputFile);
  if (outFi.completeSuffix() == "vtk" || outFi.completeSuffix() == "vtu")
  {
    smtk::io::ExportMesh exporter;
    smtk::model::ManagerPtr manager = smtk::model::Manager::create();
    didWrite = exporter(m_OutputFile.toStdString(), collection);
  }
  else
  {
    smtk::io::WriteMesh writer;
    didWrite = writer(m_OutputFile.toStdString(), collection);
  }

  if (didWrite == false)
  {
    QString ss = QObject::tr("Unable to write MOAB mesh to the specified file.");
    setErrorCondition(-101004, ss);
    return;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ExportMoabMesh::newFilterInstance(bool copyFilterParameters) const
{
  ExportMoabMesh::Pointer filter = ExportMoabMesh::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getCompiledLibraryName() const
{
  return SMTKPluginConstants::SMTKPluginBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getBrandingString() const
{
  return "SMTKPlugin";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  SMTKPlugin::Version::Major() << "." << SMTKPlugin::Version::Minor() << "." << SMTKPlugin::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getGroupName() const
{
  return SIMPL::FilterGroups::IOFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ExportMoabMesh::getUuid() const
{
  return QUuid("{724fcea9-b0cf-5077-828a-f3cbc297e49e}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getSubGroupName() const
{
  return "SMTKPlugin";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ExportMoabMesh::getHumanLabel() const
{
  return "Export MOAB Mesh";
}

// -----------------------------------------------------------------------------
ExportMoabMesh::Pointer ExportMoabMesh::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ExportMoabMesh> ExportMoabMesh::New()
{
  struct make_shared_enabler : public ExportMoabMesh
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getNameOfClass() const
{
  return QString("ExportMoabMesh");
}

// -----------------------------------------------------------------------------
QString ExportMoabMesh::ClassName()
{
  return QString("ExportMoabMesh");
}

// -----------------------------------------------------------------------------
void ExportMoabMesh::setSelectedArrayPath(const DataArrayPath& value)
{
  m_SelectedArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ExportMoabMesh::getSelectedArrayPath() const
{
  return m_SelectedArrayPath;
}

// -----------------------------------------------------------------------------
void ExportMoabMesh::setOutputFile(const QString& value)
{
  m_OutputFile = value;
}

// -----------------------------------------------------------------------------
QString ExportMoabMesh::getOutputFile() const
{
  return m_OutputFile;
}
