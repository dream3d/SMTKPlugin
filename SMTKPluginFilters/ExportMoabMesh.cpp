/*
 * Your License or Copyright can go here
 */

#include "ExportMoabMesh.h"

#include <QtCore/QFileInfo>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"

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
ExportMoabMesh::ExportMoabMesh() :
  AbstractFilter()
{
  initialize();
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ExportMoabMesh::~ExportMoabMesh()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExportMoabMesh::initialize()
{
  setErrorCondition(0);
  setWarningCondition(0);
  setCancel(false);

  m_AllowedExtensions.push_back("h5m");
  m_AllowedExtensions.push_back("mhdf");
  m_AllowedExtensions.push_back("vtk");

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
  setErrorCondition(0);
  setWarningCondition(0);

  QVector<size_t> cDims = { 1 };
  m_SelectedArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DoubleArrayType, AbstractFilter>(this, getSelectedArrayPath(), cDims);
  if(nullptr != m_SelectedArrayPtr.lock().get())                                                                   /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  {
    m_SelectedArray = m_SelectedArrayPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */

  QFileInfo outFi(m_OutputFile);

  if (m_OutputFile.isEmpty())
  {
    QString ss = QObject::tr("The output file path is empty.  Please enter a valid output file path (%1).").arg(m_ExtensionsString);
    setErrorCondition(-20000);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  else if (m_AllowedExtensions.contains(outFi.completeSuffix()) == false)
  {
    QString ss = QObject::tr("The output file does not have a valid file extension.  Please enter a valid output file path (%1).").arg(m_ExtensionsString);
    setErrorCondition(-20001);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
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

  DataContainer::Pointer dc = getDataContainerArray()->getDataContainer(getSelectedArrayPath());

  VTK_PTR(vtkDataSet) imageDataPtr = SIMPLVtkBridge::WrapDataContainerAsVtkDataset(dc);
  vtkDataSet* dataSet = imageDataPtr.Get();

  if (!dataSet)
  {
    QString ss = QObject::tr("Could not encapsulate the selected data array inside a VTK dataset.");
    setErrorCondition(-20002);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
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
    setErrorCondition(-20003);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  bool didWrite = false;
  QFileInfo outFi(m_OutputFile);
  if (outFi.completeSuffix() == "vtk")
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
    setErrorCondition(-20004);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ExportMoabMesh::newFilterInstance(bool copyFilterParameters)
{
  ExportMoabMesh::Pointer filter = ExportMoabMesh::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getCompiledLibraryName()
{ return SMTKPluginConstants::SMTKPluginBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getBrandingString()
{
  return "SMTKPlugin";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getFilterVersion()
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  SMTKPlugin::Version::Major() << "." << SMTKPlugin::Version::Minor() << "." << SMTKPlugin::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getGroupName()
{ return SIMPL::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getSubGroupName()
{ return "SMTKPlugin"; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExportMoabMesh::getHumanLabel()
{ return "Export MOAB Mesh"; }

