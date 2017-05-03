/*! \file   hydrocouplespatiotemporal.h
 *  \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 *  \version   1.0.0.0
 *  \section   Description
 *  This header files contains interface definitions that have both spatial and temporal components for the
 *  HydroCouple component-cased modeling framework.
 *  \section License
 *  hydrocouplespatiotemporal.h, associated files and libraries are free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  Lesser GNU General Public License as published by the Free Software Foundation;
 *  either version 3 of the License, or (at your option) any later version.
 *  hydrocouplespatiotemporal.h its associated files is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 *  \date 2014-2016
 *  \pre
 *  \bug
 *  \todo Add definitions for accessing node and edge associated data for meshes and polyhedral surfaces.
 *  \warning
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
        virtual ~ITimeGeometryComponentDataItem(){}

        /*!
         * \brief geometryType
         * \return
         */
        virtual HydroCouple::Spatial::GeometryType geometryType() const = 0;

        /*!
         * \brief geometryCount
         * \return
         */
        virtual int geometryCount() const = 0;

        /*!
         * \brief geometry
         * \param geometryIndex
         * \return
         */
        virtual HydroCouple::Spatial::IGeometry *geometry(int geometryIndex) const = 0;

        /*!
         * \returns The dimesion attributes for the data with the geometry. This can be the field name for an attribute for a shapefile.
         *  Must be the first dimension in the dimensions() list
         */
        virtual HydroCouple::IDimension *geometryDimension() const = 0;

        /*!
         * \brief envelope
         * \return
         */
        virtual HydroCouple::Spatial::IEnvelope *envelope() const = 0;

        /*!
         * \brief getValue
         * \param timeIndex
         * \param geometryDimensionIndex
         * \param data
         */
        virtual void getValue(int timeIndex, int geometryIndex, void *data) const = 0;

        /*!
        * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
        * \param timeIndex is the time dimension index from where to obtain the requested data.
        * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
        */
        virtual void getValues(int timeIndex, int geometryIndex, int timeStride, int geomStride, void *data) const = 0;

        /*!
         * \brief setValues
         * \param timeIndex
         * \param geometryDimensionIndex
         * \param data
         */
        virtual void setValue(int timeIndex, int geometryIndex, const void *data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
         * \param data is the input multi dimensional array to be written.
         */
        virtual void setValues(int timeIndex, int geometryIndex, int timeStride, int geomStride, const void *data) = 0;

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
        virtual ~ITimePolyhedralSurfaceComponentDataItem(){}

        /*!
         * \brief polyhedralSurfaceDataType
         * \return
         */
        virtual HydroCouple::Spatial::MeshDataType meshDataType() const = 0;

        /*!
         * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceDimension.
         */
        virtual HydroCouple::Spatial::IPolyhedralSurface *polyhedralSurface() const = 0;

        /*!
         * \returns TcellDimension.
         */
        virtual IDimension *cellDimension() const = 0;

        /*!
         * \brief edgeDimension
         * \return
         */
        virtual IDimension *edgeDimension() const = 0;

        /*!
         * \brief nodeDimension
         * \return
         */
        virtual IDimension *nodeDimension() const = 0;

        /*!
         * \brief getValue
         * \param timeIndex
         * \param cellDimensionIndex
         * \param edgeDimensionIndex
         * \param nodeDimensionIndex
         * \param data
         */
        virtual void getValue(int timeIndex, int cellDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex, void *data) const = 0;

        /*!
         * \brief setValues
         * \param timeIndex
         * \param cellDimensionIndex
         * \param edgeDimensionIndex
         * \param nodeDimensionIndex
         * \param data
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
        virtual ~ITimeTINComponentDataItem(){}

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

        virtual ~ITimeRasterComponentDataItem(){}

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
         * \brief getValue
         * \param timeIndex
         * \param xIndex
         * \param yIndex
         * \param band
         * \param data
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
         * \brief setValue
         * \param timeIndex
         * \param xIndex
         * \param yIndex
         * \param band
         * \param data
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

        virtual ~ITimeRegularGrid2DComponentDataItem(){}

        /*!
         * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid2D *grid() const = 0;

        /*!
         * \brief meshDataType
         * \return
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
         * \brief getValue
         * \param timeIndex
         * \param xCellIndex
         * \param yCellIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data
         */
        virtual void getValue(int timeIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex, void *data) const = 0;


        /*!
         * \brief setValue
         * \param timeIndex
         * \param xCellIndex
         * \param yCellIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data
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

        virtual ~ITimeRegularGrid3DComponentDataItem(){}

        /*!
         * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid3D *grid() const = 0;

        /*!
         * \brief meshDataType
         * \return
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
         * \brief getValue
         * \param timeIndex
         * \param xCellIndex
         * \param yCellIndex
         * \param zCellIndex
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data
         */
        virtual void getValue(int timeIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellNodeIndex, void *data) const = 0;

        /*!
         * \brief setValue
         * \param timeIndex
         * \param xCellIndex
         * \param yCellIndex
         * \param zCellIndex
         * \param cellFaceIndex
         * \param cellNodeIndex
         * \param data
         */
        virtual void setValue(int timeIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellNodeIndex, const void *data) = 0;

    };
  }
}

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem, "HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINComponentDataItem, "HydroCouple::SpatioTemporal::ITimeTINComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeTINComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentDataItem*)

#endif // HYDROCOUPLESPATIOTEMPORAL_H

