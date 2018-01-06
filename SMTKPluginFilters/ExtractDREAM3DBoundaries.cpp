/*
 * Your License or Copyright can go here
 */

#include "ExtractDREAM3DBoundaries.h"

#include <QtCore/QFileInfo>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"

#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#ifdef DEBUG
#undef DEBUG
#define DEBUG_ExtractDREAM3DBoundaries
#endif

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/core/CellField.h"

#ifdef DEBUG_ExtractDREAM3DBoundaries
#define DEBUG
#endif

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
ExtractDREAM3DBoundaries::ExtractDREAM3DBoundaries() :
  AbstractFilter()
{
  initialize();
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ExtractDREAM3DBoundaries::~ExtractDREAM3DBoundaries()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExtractDREAM3DBoundaries::initialize()
{
  setErrorCondition(0);
  setWarningCondition(0);
  setCancel(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExtractDREAM3DBoundaries::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(SIMPL_NEW_INPUT_FILE_FP("Input File", InputFile, FilterParameter::Parameter, ExtractDREAM3DBoundaries, "*.xdmf"));
  parameters.push_back(SIMPL_NEW_OUTPUT_FILE_FP("Output File", OutputFile, FilterParameter::Parameter, ExtractDREAM3DBoundaries, "*.vtp"));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExtractDREAM3DBoundaries::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);

  QFileInfo inFi(m_InputFile);
  QFileInfo outFi(m_OutputFile);
  
  if (m_InputFile.isEmpty())
  {
    QString ss = QObject::tr("The input file path is empty.  Please enter a valid input file path (*.xdmf).");
    setErrorCondition(-20000);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  else if (inFi.exists() == false)
  {
    QString ss = QObject::tr("The input file does not exist.  Please enter a valid input file path (*.xdmf).");
    setErrorCondition(-20001);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  else if (inFi.completeSuffix() != "xdmf")
  {
    QString ss = QObject::tr("The input file does not have a valid file extension.  Please enter a valid input file path (*.xdmf).");
    setErrorCondition(-20002);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  if (m_OutputFile.isEmpty())
  {
    QString ss = QObject::tr("The output file path is empty.  Please enter a valid output file path (*.vtp).");
    setErrorCondition(-20003);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  else if (outFi.completeSuffix() != "vtp")
  {
    QString ss = QObject::tr("The output file does not have a valid file extension.  Please enter a valid output file path (*.vtp).");
    setErrorCondition(-20004);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ExtractDREAM3DBoundaries::preflight()
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
void ExtractDREAM3DBoundaries::execute()
{
  initialize();
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  QString materialName("FeatureIds");

  // Construct a mesh manager.
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  // Read the xdmf file and access the resulting vtkImageData.
  vtkNew<vtkXdmfReader> reader;
  reader->SetFileName(m_InputFile.toStdString().c_str());
  reader->Update();
  vtkImageData* imageData = vtkImageData::SafeDownCast(reader->GetOutputDataObject(0));

  if (!imageData)
  {
    QString ss = QObject::tr("Could not extract the output image data.");
    setErrorCondition(-20005);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  // Import the vtkImageData into smtk::mesh.
  smtk::extension::vtk::io::mesh::ImportVTKData imprt;
  smtk::mesh::CollectionPtr collection = imprt(imageData, manager, materialName.toStdString());

  if (!collection)
  {
    QString ss = QObject::tr("Unable to import the collection properly.");
    setErrorCondition(-20006);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  // Access the top-level mesh set from the mesh collection.
  smtk::mesh::MeshSet mesh = collection->meshes();

  // Construct a vector of domain tags.
  auto domains = mesh.domains();

  std::vector<smtk::mesh::MeshSet> boundaries;

  // Starting with the exterior shell, loop over all of the domain meshsets and
  // extract their shells.
  boundaries.push_back(mesh.extractShell());

  for (auto& domain : domains)
  {
    if (getCancel() == true) { return; }

    notifyStatusMessage(getHumanLabel(), tr("Extracting shell for domain %1 of %2").arg(domain.value()).arg(domains.size()));
    smtk::mesh::MeshSet submesh = mesh.subset(domain);

    // Add the shell to our vector of shells.
    boundaries.push_back(submesh.extractShell());

    // At this point you have access to both the mesh (and its volumetric data)
    // and its shell. You can apply mesh data values to the shell mesh.
  }

  // We need a way to tag 2-dimensional cells with the two domain values they
  // border (assumming the exterior has a domain value 0). One way to do this
  // is to assign a doublet to each boundary cell, and have the smaller domain
  // value always populate the first position in the doublet.
  std::vector<double> boundaryValues;
  smtk::mesh::MeshSet surfaces;

  for (int i = 0; i < boundaries.size(); i++)
  {
    // For each boundary shell i...
    smtk::mesh::MeshSet mesh_i = boundaries[i];
    for (int j = i + 1; j < boundaries.size(); j++)
    {
      // ...and for each boundary shell j...
      smtk::mesh::MeshSet mesh_j = boundaries[j];

      /// ...we compute the intersection ij.
      smtk::mesh::CellSet cells_ij = smtk::mesh::set_intersect(mesh_i.cells(), mesh_j.cells());

      // If it doesn't exist, we skip this intersection.
      if (cells_ij.is_empty())
      {
        continue;
      }

      // Resize our domain doublet vector to accommodate the number of cells in
      // mesh ij
      if (boundaryValues.size() < 2 * cells_ij.size())
      {
        boundaryValues.resize(2 * cells_ij.size());
      }

      // Assign the boundary values for each component of the doublet.
      for (int k = 0; k < 2 *cells_ij.size(); k++)
      {
        boundaryValues[k] = (k % 2 == 0 ? static_cast<double>(i) : static_cast<double>(j));
      }
      smtk::mesh::MeshSet mesh_ij = collection->createMesh(cells_ij);
      surfaces.append(mesh_ij);

      mesh_ij.createCellField("boundary", 2, &boundaryValues[0]);
    }
  }

  // Export the shells of all of the domains
  notifyStatusMessage(getHumanLabel(), tr("Exporting the shells of all of the domains..."));
  vtkNew<vtkPolyData> outputPolyData;
  smtk::extension::vtk::io::mesh::ExportVTKData exprt;
  exprt(surfaces, outputPolyData.Get());

  // Write the resulting vtkPolyData to file.
  notifyStatusMessage(getHumanLabel(), tr("Writing resultant data to file..."));
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetFileName(m_OutputFile.toStdString().c_str());
  writer->SetInputData(outputPolyData.Get());
  writer->Update();

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ExtractDREAM3DBoundaries::newFilterInstance(bool copyFilterParameters)
{
  ExtractDREAM3DBoundaries::Pointer filter = ExtractDREAM3DBoundaries::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getCompiledLibraryName()
{ return SMTKPluginConstants::SMTKPluginBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getBrandingString()
{
  return "SMTKPlugin";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getFilterVersion()
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  SMTKPlugin::Version::Major() << "." << SMTKPlugin::Version::Minor() << "." << SMTKPlugin::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getGroupName()
{ return SIMPL::FilterGroups::Unsupported; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getSubGroupName()
{ return "SMTKPlugin"; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ExtractDREAM3DBoundaries::getHumanLabel()
{ return "Extract DREAM3D Boundaries"; }

