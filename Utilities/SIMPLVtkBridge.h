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

#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
#endif

#include <string>

#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/Geometry/EdgeGeom.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Geometry/QuadGeom.h"
#include "SIMPLib/Geometry/RectGridGeom.h"
#include "SIMPLib/Geometry/TetrahedralGeom.h"
#include "SIMPLib/Geometry/TriangleGeom.h"
#include "SIMPLib/Geometry/VertexGeom.h"

#include "vtkSmartPointer.h"
#include "vtkDataSet.h"

#define VTK_NEW(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define VTK_PTR(type) vtkSmartPointer<type>

class vtkImageData;
class vtkDataArray;
class vtkScalarsToColors;
class vtkScalarBarActor;

/**
 * @brief The SIMPLVtkBridge class
 */
class SIMPLVtkBridge
{
public:
  virtual ~SIMPLVtkBridge();

  static VTK_PTR(vtkDataSet) WrapDataContainerAsVtkDataset(DataContainer::Pointer dc);

  static VTK_PTR(vtkDataSet) WrapImageGeomAsVtkImageData(ImageGeom::Pointer image, Int32ArrayType::Pointer data);

  static VTK_PTR(vtkDataSet) WrapGeometry(EdgeGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(ImageGeom::Pointer image);
  static VTK_PTR(vtkDataSet) WrapGeometry(QuadGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(RectGridGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(TetrahedralGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(TriangleGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(VertexGeom::Pointer geom);
  static VTK_PTR(vtkDataSet) WrapGeometry(IGeometry::Pointer geom);

  static VTK_PTR(vtkDataArray) WrapVertices(SharedVertexList::Pointer vertexArray);
  static VTK_PTR(vtkDataArray) WrapIDataArray(IDataArray::Pointer array);
  static VTK_PTR(vtkDataArray) WrapRectGridCoords(IDataArray::Pointer array);

  template <typename T> static VTK_PTR(T) WrapIDataArrayTemplate(IDataArray::Pointer array)
  {
    VTK_NEW(T, vtkArray);
    vtkArray->SetNumberOfComponents(array->getNumberOfComponents());
    vtkArray->SetNumberOfTuples(array->getNumberOfTuples());

    vtkArray->SetVoidArray(array->getVoidPointer(0), array->getSize(), 1);

    for(int i = 0; i < vtkArray->GetNumberOfComponents(); i++)
    {
      int strlen = std::strlen(array->getName().toStdString().c_str()) + ((i + 1) / 10 + 1) + 2;
      char* componentName = new char[strlen];
      std::strcpy(componentName, array->getName().toStdString().c_str());
      std::strcat(componentName, "_");
      std::strcat(componentName, std::to_string(i).c_str());

      vtkArray->SetComponentName(i, componentName);
    }

    return vtkArray;
  }

protected:
  SIMPLVtkBridge();

private:
  SIMPLVtkBridge(const SIMPLVtkBridge&) = delete; // Copy Constructor Not Implemented
  void operator=(const SIMPLVtkBridge&) = delete; // Move assignment Not Implemented
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

