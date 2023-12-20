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
 * Lesser GNU Lesser General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
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
    class ITimeGeometryComponentDataItem : public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {

        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeGeometryComponentItem.
         */
        virtual ~ITimeGeometryComponentDataItem() = 0;

        /*!
         * \brief geometryType
         * \return
         */
        virtual HydroCouple::Spatial::IGeometry::GeometryType geometryType() const = 0;

        /*!
         * \brief geometryCount The number of geometries in the data item.
         * \return The number of geometries in the data item.
         */
        virtual int geometryCount() const = 0;

        /*!
         * \brief geometry The geometry at the specified index.
         * \param geometryIndex The index of the geometry to return.
         * \return The geometry at the specified index.
         */
        virtual HydroCouple::Spatial::IGeometry *geometry(int geometryIndex) const = 0;

        /*!
         * \returns The dimension attributes for the data with the geometry. 
         * This can be the field name for an attribute for a shapefile.
         * Must be the first dimension in the dimensions() list
         */
        virtual HydroCouple::IDimension *geometryDimension() const = 0;

        /*!
         * \brief envelope The envelope of the data item.
         * \return The envelope of the data item.
         */
        virtual HydroCouple::Spatial::IEnvelope *envelope() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index and geometry dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int geometryIndex, void *data) const = 0;

        /*!
        * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
        * \param timeIndex is the time dimension index from where to obtain the requested data.
        * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
        */
        virtual void getValues(int timeIndex, int geometryIndex, int timeStride, int geomStride, void *data) const = 0;

        /*!
         * \brief setValues Sets the value for given time dimension index and geometry dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param geometryIndex is the geometry dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int geometryIndex, const void *data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param geometryIndex is the geometry dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
         * \param data is the input multi dimensional array to be written.
         */
        virtual void setValues(int timeIndex, int geometryIndex, int timeStride, int geomStride, const void *data) = 0;

    };

    /*!
     * \brief The ITimeNetworkComponentDataItem class
     */
    class ITimeNetworkComponentDataItem : public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeNetworkComponentItem.
         */
        virtual ~ITimeNetworkComponentDataItem() = 0;

        /*!
         * \brief network associated with this ITimeNetworkComponentItem.
         * \return The network associated with this ITimeNetworkComponentItem.
         */
        virtual HydroCouple::Spatial::INetwork *network() const = 0;

        /*!
         * \brief meshDataType The mesh data type for the network.
         * \return The mesh data type for the network.
         */
        virtual HydroCouple::Spatial::MeshDataType meshDataType() const = 0;

        /*!
         * \brief edgeDimension The dimension for the edges of the network.
         * \return The dimension for the edges of the network.
         */
        virtual IDimension *edgeDimension() const = 0;

        /*!
         * \brief nodeDimension The dimension for the nodes of the network.
         * \return The dimension for the nodes of the network.
         */
        virtual IDimension *nodeDimension() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index, edge dimension index and node dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
         * \param nodeDimensionIndex is the node dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex,  int edgeDimensionIndex, int nodeDimensionIndex, void *data) const = 0;

        /*!
         * \brief setValue Sets the value for given time dimension index, edge dimension index and node dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param edgeDimensionIndex is the edge dimension index where data is to be written.
         * \param nodeDimensionIndex is the node dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int edgeDimensionIndex, int nodeDimensionIndex, const void *data) = 0;
    };

    /*!
     * \brief The ITimePolyhedralSurfaceComponentItem class.
     */
    class ITimePolyhedralSurfaceComponentDataItem :
        public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimePolyhedralSurfaceComponentItem.
         */
        virtual ~ITimePolyhedralSurfaceComponentDataItem() = 0;

        /*!
         * \brief polyhedralSurfaceDataType The mesh data type for the polyhedral surface.
         * \return The mesh data type for the polyhedral surface.
         */
        virtual HydroCouple::Spatial::MeshDataType meshDataType() const = 0;

        /*!
         * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceDimension.
         */
        virtual HydroCouple::Spatial::IPolyhedralSurface *polyhedralSurface() const = 0;

        /*!
          * \brief cellDimension The dimension for the cells of the polyhedral surface.
         * \returns The dimension for the cells of the polyhedral surface.
         */
        virtual IDimension *cellDimension() const = 0;

        /*!
         * \brief edgeDimension The dimension for the edges of the polyhedral surface.
         * \return The dimension for the edges of the polyhedral surface.
         */
        virtual IDimension *edgeDimension() const = 0;

        /*!
         * \brief nodeDimension The dimension for the nodes of the polyhedral surface.
         * \return The dimension for the nodes of the polyhedral surface.
         */
        virtual IDimension *nodeDimension() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index, cell dimension index, edge dimension index and node dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param cellDimensionIndex is the cell dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
         * \param nodeDimensionIndex is the node dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int cellDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex, void *data) const = 0;

        /*!
         * \brief setValues Sets the value for given time dimension index, cell dimension index, edge dimension index and node dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param cellDimensionIndex is the cell dimension index where data is to be written.
         * \param edgeDimensionIndex is the edge dimension index where data is to be written.
         * \param nodeDimensionIndex is the node dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int cellDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex,  const void *data) = 0;
    };

    /*!
     * \brief The ITINComponentItem class.
     */
    class ITimeTINComponentDataItem : public virtual ITimePolyhedralSurfaceComponentDataItem
    {
      public:

        /*!
         * \brief ~ITimeTINComponentItem.
         */
        virtual ~ITimeTINComponentDataItem() = 0;

        /*!
           * \returns The ITIN associated with this ITINComponentDataItem.
           */
        virtual HydroCouple::Spatial::ITIN* TIN() const = 0;

    };

    /*!
     * \brief The ITimeRasterComponentDataItem class.
     */
    class ITimeRasterComponentDataItem :  public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeRasterComponentItem.
         */
        virtual ~ITimeRasterComponentDataItem() = 0;

        /*!
         * \brief IRaster associated with this IRasterComponentDataItem.
         */
        virtual HydroCouple::Spatial::IRaster* raster() const = 0;

        /*!
         * \brief IDimension for xDirection.
         */
        virtual IDimension* xDimension() const = 0;

        /*!
         * \brief IDimension for yDirection.
         */
        virtual IDimension* yDimension() const = 0;

        /*!
         * \brief IDimension for IRasterBands.
         */
        virtual IDimension* bandDimension() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index, x dimension index, y dimension index and band dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param xIndex is the x dimension index from where to obtain the requested data.
         * \param yIndex is the y dimension index from where to obtain the requested data.
         * \param band is the band dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int xIndex, int yIndex, int band, void *data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param xindex is the x dimension index from where to obtain the requested data.
         * \param yindex is the y dimension index from where to obtain the requested data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeIndex, int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride,
                               int ystride, int bandStride, void *data) const = 0;

        /*!
         * \brief setValue Sets the value for given time dimension index, x dimension index, y dimension index and band dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param xIndex is the x dimension index where data is to be written.
         * \param yIndex is the y dimension index where data is to be written.
         * \param band is the band dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int xIndex, int yIndex, int band, const void *data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param xindex is the x dimension index where to set data.
         * \param yindex is the y dimension index where to set data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the input array to be written.
         */
        virtual void setValues(int timeIndex,int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride,
                               int ystride, int bandStride, const void *data) = 0;

    };

    /*!
     * \brief The ITimeRegularGrid2DComponentItem class.
     */
    class ITimeRegularGrid2DComponentDataItem :  public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeRegularGrid2DComponentItem. Destructor.
         */
        virtual ~ITimeRegularGrid2DComponentDataItem() = 0;

        /*!
         * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid2D *grid() const = 0;

        /*!
         * \brief meshDataType The mesh data type for the regular grid.
         * \return The mesh data type for the regular grid.
         */
        virtual HydroCouple::Spatial::MeshDataType meshDataType() const = 0;

        /*!
        * \brief Number of X cells IDimension.
        */
        virtual IDimension *xCellDimension() const = 0;

        /*!
        * \brief Number of Y cells IDimension.
        */
        virtual IDimension *yCellDimension() const = 0;

        /*!
        * \brief cellEdgeDimension. Edge indices start from the bottom and go in a counter clockwise order.
        * \return
        */
        virtual IDimension *cellEdgeDimension() const = 0;

        /*!
        * \brief cellNodeDimension. Node indices start from the bottom left and go in a counter clockwise order.
        * \return
        */
        virtual IDimension *cellNodeDimension() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index, x cell dimension index, y cell dimension index, cell edge dimension index and cell node dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x cell dimension index from where to obtain the requested data.
         * \param yCellIndex is the y cell dimension index from where to obtain the requested data.
         * \param cellEdgeIndex is the cell edge dimension index from where to obtain the requested data.
         * \param cellNodeIndex is the cell node dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex, void *data) const = 0;


        /*!
         * \brief setValue Sets the value for given time dimension index, x cell dimension index, y cell dimension index, cell edge dimension index and cell node dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param xCellIndex is the x cell dimension index where data is to be written.
         * \param yCellIndex  is the y cell dimension index where data is to be written.
         * \param cellEdgeIndex is the cell edge dimension index where data is to be written.
         * \param cellNodeIndex is the cell node dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex, const void *data) = 0;
    };

    /*!
     * \brief The ITimeRegularGrid3DComponentItem class.
     */
    class ITimeRegularGrid3DComponentDataItem :  public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeRegularGrid3DComponentItem. Destructor.
         */
        virtual ~ITimeRegularGrid3DComponentDataItem() = 0;

        /*!
         * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid3D *grid() const = 0;

        /*!
         * \brief meshDataType The mesh data type for the regular grid.
         * \return The mesh data type for the regular grid.
         */
        virtual HydroCouple::Spatial::MeshDataType meshDataType() const = 0;

        /*!
         * \brief Number of X cells IDimension.
         */
        virtual IDimension *xCellDimension() const = 0;

        /*!
         * \brief Number of Y cells IDimension.
         */
        virtual IDimension *yCellDimension() const = 0;

        /*!
         * \brief Number of Z cells IDimension.
         */
        virtual IDimension *zCellDimension() const = 0;

        /*!
         * \brief cellFaceDimension 0 = Top , 1 = Bottom, 2 = left , 3 = Right, Up = 4, Down = 5
         * \return
         */
        virtual IDimension *cellFaceDimension() const = 0;

        /*!
         * \brief cellNodeDimension
         * \return
         */
        virtual IDimension *cellNodeDimension() const = 0;

        /*!
         * \brief getValue  Gets the value for given time dimension index, x cell dimension index, y cell dimension index, z cell dimension index, cell face dimension index and cell node dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x cell dimension index from where to obtain the requested data.
         * \param yCellIndex is the y cell dimension index from where to obtain the requested data.
         * \param zCellIndex is the z cell dimension index from where to obtain the requested data.
         * \param cellFaceIndex is the cell face dimension index from where to obtain the requested data.
         * \param cellNodeIndex is the cell node dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellNodeIndex, void *data) const = 0;

        /*!
         * \brief setValue Sets the value for given time dimension index, x cell dimension index, y cell dimension index, z cell dimension index, cell face dimension index and cell node dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param xCellIndex is the x cell dimension index where data is to be written.
         * \param yCellIndex is the y cell dimension index where data is to be written.
         * \param zCellIndex is the z cell dimension index where data is to be written.
         * \param cellFaceIndex is the cell face dimension index where data is to be written.
         * \param cellNodeIndex is the cell node dimension index where data is to be written.
         * \param data is the input array to be written.
         */
        virtual void setValue(int timeIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellNodeIndex, const void *data) = 0;

    };

    /*!
     * \brief The ITimeVectorComponentDataItem class
     */
    class ITimeVectorComponentDataItem : public virtual HydroCouple::Temporal::ITimeComponentDataItem
    {
        using IComponentDataItem::getValue;
        using IComponentDataItem::setValue;

      public:

        /*!
         * \brief ~ITimeVectorComponentItem.
         */
        virtual ~ITimeVectorComponentDataItem() = 0;

        /*!
         * \brief locationCount
         * \return
         */
        virtual int locationsCount() const = 0;

        /*!
         * \brief location The location at the specified index.
         * \param locationIndex The index of the location to return.
         * \return The location at the specified index.
         */
        virtual HydroCouple::Spatial::IPoint *location(int locationIndex) const = 0;

        /*!
         * \brief locationsDimension The dimension for the locations of the vector.
         * \return The dimension for the locations of the vector.
         */
        virtual IDimension *locationsDimension() const = 0;

        /*!
         * \brief dataTypeDimension alway has a length of 3. 0 = x-direction value,
         * 1 = y-direction, 2 = z-direction,
         * \return
         */
        virtual IDimension *spatialDimension() const = 0;

        /*!
         * \brief getValue Gets the value for given time dimension index, location dimension index and spatial dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param locationIndex is the location dimension index from where to obtain the requested data.
         * \param spatialIndex is the spatial dimension index from where to obtain the requested data.
         * \param data is the array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValue(int timeIndex, int locationIndex, int spatialIndex, void *data) const = 0;

        /*!
         * \brief getValues Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param locationIndex is the location dimension index from where to obtain the requested data.
         * \param spatialDimensionIndex is the spatial dimension index from where to obtain the requested data.
         * \param locationStride is the size for the location dimension for hyperslab from which to copy data.
         * \param spatialDimensionStride is the size for the spatial dimension for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeIndex, int locationIndex, int spatialDimensionIndex,
                               int locationStride, int spatialDimensionStride, void *data) const = 0;
        /*!
         * \brief setValue Sets the value for given time dimension index, location dimension index and spatial dimension index.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param locationIndex is the location dimension index where data is to be written.
         * \param spatialDimensionIndex is the spatial dimension index where data is to be written.
         * \param data is the input array to be written.
         */ 
        virtual void setValue(int timeIndex, int locationIndex, int spatialDimensionIndex, const void *data) = 0;

        /*!
         * \brief setValues Sets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeIndex is the  time dimension index where data is to be written.
         * \param locationIndex is the location dimension index where data is to be written.
         * \param spatialDimensionIndex is the spatial dimension index where data is to be written.
         * \param locationStride is the size for the location dimension for hyperslab from which to copy data.
         * \param spatialDimensionStride is the size for the spatial dimension for hyperslab from which to copy data.
         * \param data is the input multi dimensional array to be written.
         */
        virtual void setValues(int timeIndex, int locationIndex, int spatialDimensionIndex,
                               int locationStride, int spatialDimensionStride, const void *data) = 0;
    };
  }
} 
#endif // HYDROCOUPLESPATIOTEMPORAL_H

