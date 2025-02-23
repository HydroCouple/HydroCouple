/*!
 * \file hydrocouplespatiotemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \description
 * This header file contains the spatio-temporal interface definitions for the
 * HydroCouple component-based modeling definitions.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * MIT License as published by the Free Software Foundation.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 * \copyright Copyright 2014-2024, Caleb Buahin, All rights reserved.
 * \date 2014-2024
 * \pre
 * \bug
 * \warning
 * \todo
 */

#ifndef HYDROCOUPLESPATIOTEMPORAL_H
#define HYDROCOUPLESPATIOTEMPORAL_H

#include "hydrocoupletemporal.h"
#include "hydrocouplespatial.h"

namespace HydroCouple
{
  //! HydroCouple's interfaces that have both spatial and temporal components.
  namespace SpatioTemporal
  {
    /*!
     * \brief ITimeGeometryComponentItem represents an IComponentItem with both temporal and geometric components.
     */
    class ITimeGeometryComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::IGeometryComponentDataItem
    {

    public:
      using IGeometryComponentDataItem::getValue;
      using IGeometryComponentDataItem::getValues;
      using IGeometryComponentDataItem::setValue;
      using IGeometryComponentDataItem::setValues;
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief ~ITimeGeometryComponentItem.
       */
      virtual ~ITimeGeometryComponentDataItem() = default;

      /*!
       * \brief getValue Gets the value for given time dimension index and geometry dimension index.
       * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
       * \param dimensionIndexes indexes to use for the other dimensions to get the data if they exist otherwise an empty vector.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeIndex,
          int geometryIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
       * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
       * \param dimensionIndexes indexes to use for the other dimensions to get the data if they exist otherwise an empty vector.
       * \param timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param geometryIndexLength is the length of the geometry dimension for the data to be obtained.
       * \param dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeIndex,
          int geometryIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int geometryIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValues Sets the value for given time dimension index and geometry dimension index.
       * \param data is the input array to be written.
       * \param timeIndex is the time dimension index where data is to be written.
       * \param geometryIndex is the geometry dimension index where data is to be written.
       * \param dimensionIndexes indexes to use for the other dimensions to get the data if they exist otherwise an empty vector.
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeIndex,
          int geometryIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
       * \param data is the input multi dimensional array to be written.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
       * \param dimensionIndexes indexes to use for the other dimensions to get the data if they exist otherwise an empty vector.
       * \param timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeIndex,
          int geometryIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int geometryIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The ITimeNetworkComponentDataItem class
     */
    class ITimeNetworkComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::INetworkComponentDataItem
    {

    public:
      using INetworkComponentDataItem::getValue;
      using INetworkComponentDataItem::getValues;
      using INetworkComponentDataItem::setValue;
      using INetworkComponentDataItem::setValues;
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief getValue for given edge dimension index and node dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given edge dimension index and node dimension index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for any additional dimensions for the data to be obtained.
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] edgeDimensionIndexLength is the length of the edge dimension for the data to be obtained.
       * \param[in] vertexDimensionIndexLength is the length of the node dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given edge dimension index and node dimension index and data.
       * \param[out] data is a pointer data thata to is to be copied
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given edge dimension index and node dimension index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] edgeDimensionIndexLength is the length of the edge dimension for the data to be obtained.
       * \param[in] vertexDimensionIndexLength is the length of the node dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The ITimeSeriesPolyhedralSurfaceComponentDataItem class.
     */
    class ITimeSeriesPolyhedralSurfaceComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::IPolyhedralSurfaceComponentDataItem
    {

    public:
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      using IPolyhedralSurfaceComponentDataItem::getValue;
      using IPolyhedralSurfaceComponentDataItem::getValues;
      using IPolyhedralSurfaceComponentDataItem::setValue;
      using IPolyhedralSurfaceComponentDataItem::setValues;

      /*!
       * \brief ~ITimePolyhedralSurfaceComponentItem.
       */
      virtual ~ITimeSeriesPolyhedralSurfaceComponentDataItem() = default;

      /*!
       * \brief getValue for given cell dimension index, edge dimension index, and node dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] patchDimensionIndex is the cell dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeDimensionIndex,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given cell dimension index, edge dimension index and node dimension index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] patchDimensionIndex are the cell dimension indexes from where to obtain the requested data.
       * \param[in] edgeDimensionIndex are the edge dimension indexes from where to obtain the requested data. If user wants to
       * only obtain cell data for MeshDataType::Cell, this vector should be empty.
       * \param[in] vertexDimensionIndex are the node dimension indexes from where to obtain the requested data. If user wants to
       * only obtain cell or edge data for MeshDataType::Cell or MeshDataType::Edge, this vector should be empty.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] patchDimensionIndexLength are the lengths of the cell dimensions for the data to be obtained.
       * \param[in] edgeDimensionIndexLength are the lengths of the edge dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] vertexDimensionIndexLength are the lengths of the node dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeDimensionIndex,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int patchDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValues for given cell dimension index, edge dimension index, and node dimension index.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] patchDimensionIndex is the face dimension index from where to write data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the edge dimension index from where to obtain the requested data. If user wants to
       * only obtain cell data for MeshDataType::Cell, this vector should be empty.
       * \param dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeDimensionIndex,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given cell dimension index, edge dimension index, and node dimension index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] patchDimensionIndex are the cell dimension indexes from where to write data.
       * \param[in] edgeDimensionIndex are the edge dimension indexes from where to write data.
       * \param[in] vertexDimensionIndex are the node dimension indexes from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] patchDimensionIndexLength are the lengths of the cell dimensions for the data to be obtained.
       * \param[in] edgeDimensionIndexLength are the lengths of the edge dimensions for the data to be obtained.
       * \param[in] vertexDimensionIndexLength are the lengths of the node dimensions for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeDimensionIndex,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int patchDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths) = 0;
    };

    /*!
     * \brief The ITINComponentItem class.
     */
    class ITimeSeriesTINComponentDataItem : public virtual ITimeSeriesPolyhedralSurfaceComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeTINComponentItem.
       */
      virtual ~ITimeSeriesTINComponentDataItem() = default;

      /*!
       * \returns The ITIN associated with this ITINComponentDataItem.
       */
      virtual HydroCouple::Spatial::ITIN *TIN() const = 0;
    };

    /*!
     * \brief The ITimeRasterComponentDataItem class.
     */
    class ITimeSeriesRasterComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::IRasterComponentDataItem
    {

    public:
      using IRasterComponentDataItem::getValue;
      using IRasterComponentDataItem::getValues;
      using IRasterComponentDataItem::setValue;
      using IRasterComponentDataItem::setValues;
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief ~ITimeSeriesRasterComponentDataItem Destructor.
       */
      virtual ~ITimeSeriesRasterComponentDataItem() = default;

      /*!
       * \brief getValue for given x dimension index, y dimension index, and band dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xIndex is the x dimension index from where to obtain the requested data.
       * \param[in] yIndex is the y dimension index from where to obtain the requested data.
       * \param[in] bandIndex is the band dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeDimensionIndex,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
       * \param[out] data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xIndex is the x dimension index from where to obtain the requested data.
       * \param[in] yIndex is the y dimension index from where to obtain the requested data.
       * \param[in] bandIndex is the band dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xIndexLength is the length of the x dimension for the data to be obtained.
       * \param[in] yIndexLength is the length of the y dimension for the data to be obtained.
       * \param[in] bandIndexLength is the length of the band dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeDimensionIndex,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int xIndexLength = 1,
          int yIndexLength = 1,
          int bandIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x dimension index, y dimension index, and band dimension index.
       * \param[in] data  is a pointer data thata to is to be copied
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xIndex is the x dimension index from where to write data.
       * \param[in] yIndex is the y dimension index from where to write data.
       * \param[in] band  is the band dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeDimensionIndex,
          int xIndex,
          int yIndex,
          int band,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
       * \param[in] data is the input array to be written.
       * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xIndex is the x dimension index where to set data.
       * \param[in] yIndex is the y dimension index where to set data.
       * \param[in] bandIndex is the band dimension index where to set data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xIndexLength is the x size for hyperslab where data is to be written.
       * \param[in] yIndexLength is the y size for hyperslab where data is to be written.
       * \param[in] bandIndexLength is the band size for hyperslab where data is to be written.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeDimensionIndex,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeDimensionIndexLength = 1,
          int xIndexLength = 1,
          int yIndexLength = 1,
          int bandIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The ITimeRegularGrid2DComponentItem class.
     */
    class ITimeRegularGrid2DComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::IRegularGrid2DComponentDataItem
    {

    public:
      using IRegularGrid2DComponentDataItem::getValue;
      using IRegularGrid2DComponentDataItem::getValues;
      using IRegularGrid2DComponentDataItem::setValue;
      using IRegularGrid2DComponentDataItem::setValues;
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief ~ITimeRegularGrid2DComponentItem. Destructor.
       */
      virtual ~ITimeRegularGrid2DComponentDataItem() = default;

      /*!
       * \brief getValue for given x cell index, y cell index, edge index, and node index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] cellEdgeIndex is the cell edge index from where to obtain the requested data.
       * \param[in] cellNodeIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.  If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given x cell index, y cell index, edge index, and node index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] cellEdgeIndex is the cell edge index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] cellEdgeIndexLength is the length of the cell edge index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int cellEdgeIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x cell index, y cell index, edge index, and node index.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] cellEdgeIndex is the cell edge index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given x cell index, y cell index, edge index, and node index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] cellEdgeIndex is the cell edge index from where to write data.
       * \param[in] cellNodeIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] cellEdgeIndexLength is the length of the cell edge index for the data to be obtained.
       * \param[in] cellNodeIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int cellEdgeIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The ITimeRegularGrid3DComponentItem class.
     */
    class ITimeRegularGrid3DComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem, public virtual HydroCouple::Spatial::IRegularGrid3DComponentDataItem
    {

    public:
      using IRegularGrid3DComponentDataItem::getValue;
      using IRegularGrid3DComponentDataItem::getValues;
      using IRegularGrid3DComponentDataItem::setValue;
      using IRegularGrid3DComponentDataItem::setValues;
      using ITimeSeriesComponentDataItem::getValue;
      using ITimeSeriesComponentDataItem::getValues;
      using ITimeSeriesComponentDataItem::setValue;
      using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief ~ITimeRegularGrid3DComponentItem. Destructor.
       */
      virtual ~ITimeRegularGrid3DComponentDataItem() = default;

      /*!
       * \brief getValue for given x cell index, y cell index, z cell index, face index, and node index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] zCellIndex is the z cell index from where to obtain the requested data.
       * \param[in] cellFaceIndex is the cell face index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given x cell index, y cell index, z cell index, face index, and node index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] zCellIndex is the z cell index from where to obtain the requested data.
       * \param[in] cellFaceIndex is the cell face index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] zCellIndexLength is the length of the z cell index for the data to be obtained.
       * \param[in] cellFaceIndexLength is the length of the cell face index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int zCellIndexLength = 1,
          int cellFaceIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x cell index, y cell index, z cell index, face index, and node index.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] zCellIndex is the z cell index from where to write data.
       * \param[in] cellFaceIndex is the cell face index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given x cell index, y cell index, z cell index, face index, and node index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] zCellIndex is the z cell index from where to write data.
       * \param[in] cellFaceIndex is the cell face index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] zCellIndexLength is the length of the z cell index for the data to be obtained.
       * \param[in] cellFaceIndexLength is the length of the cell face index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int timeIndex,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int timeIndexLength = 1,
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int zCellIndexLength = 1,
          int cellFaceIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };
  }
}
#endif // HYDROCOUPLESPATIOTEMPORAL_H
