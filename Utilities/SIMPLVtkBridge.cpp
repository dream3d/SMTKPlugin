/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "SIMPLVtkBridge.h"

#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkLine.h>
#include <vtkLongLongArray.h>
#include <vtkLookupTable.h>
#include <vtkMappedUnstructuredGrid.h>
#include <vtkNamedColors.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkRectilinearGrid.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkShortArray.h>
#include <vtkTextProperty.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertexGlyphFilter.h>

#include "Utilities/VtkTetrahedralGeom.h"
#include "Utilities/VtkTriangleGeom.h"
#include "Utilities/VtkEdgeGeom.h"
#include "Utilities/VtkVertexGeom.h"
#include "Utilities/VtkQuadGeom.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLVtkBridge::SIMPLVtkBridge()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLVtkBridge::~SIMPLVtkBridge()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<VTK_PTR(vtkDataSet)> SIMPLVtkBridge::WrapDataContainerArrayAsVtkDatasets(DataContainerArray::Pointer dca)
{
  std::vector<VTK_PTR(vtkDataSet)> dataSets;

  if(!dca)
  {
    return dataSets;
  }

  QList<DataContainer::Pointer> dcs = dca->getDataContainers();

  for(QList<DataContainer::Pointer>::Iterator dc = dcs.begin(); dc != dcs.end(); ++dc)
  {
    VTK_PTR(vtkDataSet) dataSet = WrapDataContainerAsVtkDataset((*dc));

    if(dataSet)
    {
      dataSets.push_back(dataSet);
    }
  }

  return dataSets;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapDataContainerAsVtkDataset(DataContainer::Pointer dc)
{
  VTK_PTR(vtkDataSet) dataSet;

  if(!dc)
  {
    return dataSet;
  }

  if(!dc->getGeometry())
  {
    return dataSet;
  }

  dataSet = WrapGeometry(dc->getGeometry());

  if(!dataSet)
  {
    return dataSet;
  }
  else
  {
    DataContainer::AttributeMatrixMap_t attrMats = dc->getAttributeMatrices();

    for(DataContainer::AttributeMatrixMap_t::Iterator attrMat = attrMats.begin(); attrMat != attrMats.end(); ++attrMat)
    {
      if(!(*attrMat))
      {
        continue;
      }

      // For now, only support Cell, Edge, and Vertex AttributeMatrices
      if((*attrMat)->getType() != AttributeMatrix::Type::Cell
         && (*attrMat)->getType() != AttributeMatrix::Type::Edge
         && (*attrMat)->getType() != AttributeMatrix::Type::Face
         && (*attrMat)->getType() != AttributeMatrix::Type::Vertex)
      {
        continue;
      }
      else
      {
        // If the attribute matrix is of type cell but the elements and tuples do not
        // match up, just continue ; this really should never happen!
        if((*attrMat)->getNumberOfTuples() != dataSet->GetNumberOfCells())
        {
          continue;
        }

        QStringList arrayNames = (*attrMat)->getAttributeArrayNames();

        for(QStringList::Iterator arrayName = arrayNames.begin(); arrayName != arrayNames.end(); ++arrayName)
        {
          IDataArray::Pointer array = (*attrMat)->getAttributeArray((*arrayName));

          if(!array)
          {
            continue;
          }
          // Do not support bool arrays for the moment...
          // else
          {
            VTK_PTR(vtkDataArray) vtkArray = WrapIDataArray(array);
            if(!vtkArray)
            {
              continue;
            }
            else
            {
              vtkArray->SetName(array->getName().toStdString().c_str());

              dataSet->GetCellData()->AddArray(vtkArray);
            }
          }
        }
      }
    }

    vtkCellData* cellData = dataSet->GetCellData();
    if(cellData->GetNumberOfArrays() > 0)
    {
      cellData->SetActiveScalars(cellData->GetArray(0)->GetName());
    }
  }

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapDataArraysAsVtkDataset(QList<IDataArray::Pointer> das)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataArray) SIMPLVtkBridge::WrapVertices(SharedVertexList::Pointer vertexArray)
{
  VTK_NEW(vtkFloatArray, vtkArray);
  vtkArray->SetNumberOfComponents(vertexArray->getNumberOfComponents());
  vtkArray->SetNumberOfTuples(vertexArray->getNumberOfTuples());

  vtkArray->SetVoidArray(vertexArray->getVoidPointer(0), vertexArray->getSize(), 1);

  return vtkArray;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(EdgeGeom::Pointer geom)
{
  VTK_NEW(VtkEdgeGrid, dataSet);
  VtkEdgeGeom* edgeGeom = dataSet->GetImplementation();
  edgeGeom->SetGeometry(geom);

  VTK_NEW(vtkPoints, points);
  VTK_PTR(vtkDataArray) vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(QuadGeom::Pointer geom)
{
  VTK_NEW(VtkQuadGrid, dataSet);
  VtkQuadGeom* quadGeom = dataSet->GetImplementation();
  quadGeom->SetGeometry(geom);
  
  VTK_NEW(vtkPoints, points);
  VTK_PTR(vtkDataArray) vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(RectGridGeom::Pointer geom)
{
  // vtkRectilinearGrid requires 3 arrays with the x, y, and z coordinates of points
  VTK_PTR(vtkDataArray) xCoords = WrapIDataArray(geom->getXBounds());
  VTK_PTR(vtkDataArray) yCoords = WrapIDataArray(geom->getYBounds());
  VTK_PTR(vtkDataArray) zCoords = WrapIDataArray(geom->getZBounds());

  int x = xCoords->GetNumberOfTuples();
  int y = yCoords->GetNumberOfTuples();
  int z = zCoords->GetNumberOfTuples();

  // Create the vtkRectilinearGrid and apply values
  VTK_NEW(vtkRectilinearGrid, vtkRectGrid);
  vtkRectGrid->SetXCoordinates(xCoords);
  vtkRectGrid->SetYCoordinates(yCoords);
  vtkRectGrid->SetZCoordinates(zCoords);
  vtkRectGrid->SetExtent(0, x, 0, y, 0, z);
  vtkRectGrid->SetDimensions(x, y, z);

  return vtkRectGrid;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(TetrahedralGeom::Pointer geom)
{
  VTK_NEW(VtkTetrahedralGrid, dataSet);
  VtkTetrahedralGeom* tetGeom = dataSet->GetImplementation();
  tetGeom->SetGeometry(geom);
  
  VTK_NEW(vtkPoints, points);
  VTK_PTR(vtkDataArray) vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(TriangleGeom::Pointer geom)
{
  VTK_NEW(VtkTriangleGrid, dataSet);
  VtkTriangleGeom* triGeom = dataSet->GetImplementation();
  triGeom->SetGeometry(geom);
  
  VTK_NEW(vtkPoints, points);
  VTK_PTR(vtkDataArray) vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(VertexGeom::Pointer geom)
{
  VTK_NEW(VtkVertexGrid, dataSet);
  VtkVertexGeom* vertGeom = dataSet->GetImplementation();
  vertGeom->SetGeometry(geom);
  
  VTK_NEW(vtkPoints, points);
  VTK_PTR(vtkDataArray) vertexArray = WrapVertices(geom->getVertices());
  points->SetDataTypeToFloat();
  points->SetData(vertexArray);
  dataSet->SetPoints(points);

  return dataSet;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(ImageGeom::Pointer image)
{
  size_t dims[3] = {0, 0, 0};
  float res[3] = {0.0f, 0.0f, 0.0f};
  float origin[3] = {0.0f, 0.0f, 0.0f};

  image->getDimensions(dims);
  image->getResolution(res);
  image->getOrigin(origin);

  VTK_NEW(vtkImageData, vtkImage);
  vtkImage->SetExtent(0, dims[0], 0, dims[1], 0, dims[2]);
  vtkImage->SetDimensions(dims[0] + 1, dims[1] + 1, dims[2] + 1);
  vtkImage->SetSpacing(res[0], res[1], res[2]);
  vtkImage->SetOrigin(origin[0], origin[1], origin[2]);

  return vtkImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapGeometry(IGeometry::Pointer geom)
{
  if(std::dynamic_pointer_cast<EdgeGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<EdgeGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<ImageGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<ImageGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<QuadGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<QuadGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<RectGridGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<RectGridGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<TetrahedralGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<TetrahedralGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<TriangleGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<TriangleGeom>(geom));
  }
  else if(std::dynamic_pointer_cast<VertexGeom>(geom))
  {
    return WrapGeometry(std::dynamic_pointer_cast<VertexGeom>(geom));
  }

  // Default to nullptr if the type does not match a supported geometry
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataArray) SIMPLVtkBridge::WrapIDataArray(IDataArray::Pointer array)
{
  if(std::dynamic_pointer_cast<UInt8ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedCharArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int8ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkCharArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt16ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedShortArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int16ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkShortArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt32ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedIntArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int32ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkIntArray>(array);
  }
  else if(std::dynamic_pointer_cast<UInt64ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkUnsignedLongLongArray>(array);
  }
  else if(std::dynamic_pointer_cast<Int64ArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkLongLongArray>(array);
  }
  else if(std::dynamic_pointer_cast<FloatArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkFloatArray>(array);
  }
  else if(std::dynamic_pointer_cast<DoubleArrayType>(array))
  {
    return WrapIDataArrayTemplate<vtkDoubleArray>(array);
  }
  else
  {
    return nullptr;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VTK_PTR(vtkDataSet) SIMPLVtkBridge::WrapImageGeomAsVtkImageData(ImageGeom::Pointer image, Int32ArrayType::Pointer data)
{
  size_t dims[3] = {0, 0, 0};
  float res[3] = {0.0f, 0.0f, 0.0f};
  float origin[3] = {0.0f, 0.0f, 0.0f};

  image->getDimensions(dims);
  image->getResolution(res);
  image->getOrigin(origin);

  VTK_NEW(vtkImageData, vtk_image);
  vtk_image->SetExtent(0, dims[0], 0, dims[1], 0, dims[2]);
  vtk_image->SetDimensions(dims[0] + 1, dims[1] + 1, dims[2] + 1);
  vtk_image->SetSpacing(res[0], res[1], res[2]);
  vtk_image->SetOrigin(origin[0], origin[1], origin[2]);

  VTK_NEW(vtkIntArray, intArray);
  intArray->SetVoidArray(data->getVoidPointer(0), data->getSize(), 1);

  vtk_image->GetCellData()->SetScalars(intArray);

  return vtk_image;
}
