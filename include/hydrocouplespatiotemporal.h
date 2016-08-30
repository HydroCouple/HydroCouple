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
    class ITimeGeometryComponentDataItem : public virtual IComponentDataItem
    {
      public:

        /*!
         * \brief ~ITimeGeometryComponentItem.
         */
        virtual ~ITimeGeometryComponentDataItem(){}

        /*!
         * \brief ITimes associated with this dimension.
         * \returns QList<ITime*>
         */
        virtual QList<HydroCouple::Temporal::ITime*> times() const = 0;

        /*!
         * \brief ITimeSpan associated with this dimension.
         */
        virtual HydroCouple::Temporal::ITimeSpan* timeSpan() const = 0;

        /*!
         * \brief IDimension of the times.
         * \returns IDimension
         */
        virtual IDimension* timeDimension() const = 0;

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
         * \brief Gets a single value for given time dimension index.
         *
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
         * \param data is the output QVariant where data is to be written.
         */
        using IComponentDataItem::getValue;
        virtual void getValue(int timeDimensionIndex, int geometryDimensionIndex, QVariant &data) const = 0;

        /*!
        * \brief Gets a multi-dimensional array of values for given dimension time dimension index and size for a hyperslab.
        * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
        * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
        */
        using IComponentDataItem::getValues;
        virtual void getValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, QVariant data[]) const = 0;

        /*!
        * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
        * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
        * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
        */
        virtual void getValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, void *data) const = 0;

        /*!
         * \brief Sets a single value for given time dimension index.
         * \param timeDimensionIndex is the time dimension index where data is to be written.
         * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
         * \param data is the input QVariant to be written.
         */
        using IComponentDataItem::setValue;
        virtual void setValue(int timeDimensionIndex, int geometryDimensionIndex, const QVariant &data) = 0;

        /*!
        * \brief Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
        * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
        * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is the input multi dimensional array to be written.
        */
        using IComponentDataItem::setValues;
        virtual void setValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, const QVariant data[]) = 0;

        //!Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
        /*!
        * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
        * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
        * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
        * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
        * \param data is the input multi dimensional array to be written.
        */
        virtual void setValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, const void *data) = 0;

    };

    /*!
     * \brief The ITimeGeometryExchangeItem class.
     */
    class ITimeGeometryExchangeItem : public virtual IExchangeItem,
        public virtual ITimeGeometryComponentDataItem
    {
      public:
        virtual ~ITimeGeometryExchangeItem(){}

    };

    /*!
     * \brief The ITimeGeometryArgument class.
     */
    class ITimeGeometryArgument : public virtual IArgument,
        public virtual ITimeGeometryComponentDataItem
    {
      public:
        virtual ~ITimeGeometryArgument(){}

    };


    /*==========================================================================================================*/

    /*!
     * \brief The ITimePolyhedralSurfaceComponentItem class.
     */
    class ITimePolyhedralSurfaceComponentDataItem :  public virtual IComponentDataItem
    {
      public:

        /*!
         * \brief ~ITimePolyhedralSurfaceComponentItem.
         */
        virtual ~ITimePolyhedralSurfaceComponentDataItem(){}

        /*!
         * \brief ITimes associated with this dimension.
         * \returns QList<ITime*>
         */
        virtual QList<HydroCouple::Temporal::ITime*> times() const = 0;

        /*!
         * \brief ITimeSpan associated with this dimension.
         */
        virtual HydroCouple::Temporal::ITimeSpan* timeSpan() const = 0;

        /*!
         * \brief IDimension of the times.
         * \returns IDimension
         */
        virtual IDimension* timeDimension() const = 0;

        /*!
         * \brief polyhedralSurfaceDataType
         * \return
         */
        virtual HydroCouple::Spatial::PolyhedralSurfaceDataType polyhedralSurfaceDataType() const = 0;

        /*!
           * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceDimension.
           */
        virtual HydroCouple::Spatial::IPolyhedralSurface* polyhedralSurface() const = 0;

        /*!
         * \returns The IDimension for this IPolyhedralSurfaceComponentItem patches.
         */
        virtual IDimension *patchDimension() const = 0;

        /*!
         * \brief polyhedralSurfaceEdgeDimension
         * \return
         */
        virtual IDimension *patchEdgeDimension() const = 0;

        /*!
         * \brief polyhedralSurfaceNodeDimensions
         * \return
         */
        virtual IDimension *edgeNodeDimensions() const = 0;


        //!Gets a single value for given patch dimension index.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param data is the output QVariant where data is to be written.
         */
        using IComponentDataItem::getValue;
        virtual void getValue(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex,
                              int nodeDimensionIndex, QVariant& data) const = 0;

        //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
         * \param patchStride is the size for hyperslab from which to copy data.
         * \param edgeStride is the size for hyperslab from which to copy data.
         * \param nodeStride is the size for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::getValues;
        virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex,
                               int timeStride, int patchStride, int edgeStride, int nodeStride, QVariant data[]) const = 0;

        //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
         * \param patchStride is the size for hyperslab from which to copy data.
         * \param edgeStride is the size for hyperslab from which to copy data.
         * \param nodeStride is the size for hyperslab from which to copy data.
         * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
         */
        virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex,
                               int timeStride, int patchStride, int edgeStride, int nodeStride,  void *data) const = 0;

        //!Sets a single value for given patch dimension index.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param data is the input QVariant to be written.
         */
        using IComponentDataItem::setValue;
        virtual void setValue(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex,
                              int nodeDimensionIndex, const QVariant &data) = 0;

        //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
         * \param patchStride is the size for hyperslab from which to copy data.
         * \param edgeStride is the size for hyperslab from which to copy data.
         * \param nodeStride is the size for hyperslab from which to copy data.
         * \param data is the input multi dimensional array to be written.
         */
        using IComponentDataItem::setValues;
        virtual void setValues(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex,
                               int timeStride, int patchStride, int edgeStride, int nodeStride, const QVariant data[]) = 0;

        //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
        /*!
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
         * \param edgeDimensionIndex the index of the edge to retrieve data for.
         * \param nodeDimensionIndex the index of the edge to retrieve data for.
         * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
         * \param patchStride is the size for hyperslab from which to copy data.
         * \param edgeStride is the size for hyperslab from which to copy data.
         * \param nodeStride is the size for hyperslab from which to copy data.
         * \param data is the input multi dimensional array to be written.
         */
        virtual void setValues(int timeDimensionIndex, int patchDimensionIndex, int edgeDimensionIndex, int nodeDimensionIndex,
                               int timeStride, int patchStride, int edgeStride, int nodeStride, const void *data) = 0;
    };

    /*!
     * \brief The ITimePolyhedralSurfaceExchangeItem class.
     */
    class ITimePolyhedralSurfaceExchangeItem : public virtual IExchangeItem,
        public virtual ITimePolyhedralSurfaceComponentDataItem
    {
      public:
        virtual ~ITimePolyhedralSurfaceExchangeItem(){}
    };

    /*!
     * \brief The ITimePolyhedralSurfaceArgument class.
     */
    class ITimePolyhedralSurfaceArgument : public virtual IArgument,
        public virtual ITimePolyhedralSurfaceComponentDataItem
    {
      public:
        virtual ~ITimePolyhedralSurfaceArgument(){}
    };

    /*==========================================================================================================*/

    /*!
     * \brief The ITINComponentItem class.
     */
    class ITimeTINComponentDataItem : public virtual IComponentDataItem,
        public virtual ITimePolyhedralSurfaceComponentDataItem
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
     * \brief The ITimeTINExchangeItem class.
     */
    class ITimeTINExchangeItem : public virtual IExchangeItem,
        public virtual ITimeTINComponentDataItem
    {
      public:
        virtual ~ITimeTINExchangeItem(){}
    };

    /*!
     * \brief The ITimeTINArgument class.
     */
    class ITimeTINArgument : public virtual IArgument ,
        public virtual ITimeTINComponentDataItem
    {
      public:
        virtual ~ITimeTINArgument(){}
    };


    /*==========================================================================================================*/

    /*!
     * \brief The ITimeRasterComponentDataItem class.
     */
    class ITimeRasterComponentDataItem :  public virtual IComponentDataItem
    {
      public:

        virtual ~ITimeRasterComponentDataItem(){}


        /*!
         * \brief ITimes associated with this dimension.
         * \returns QList<ITime*>
         */
        virtual QList<HydroCouple::Temporal::ITime*> times() const = 0;

        /*!
         * \brief ITimeSpan associated with this dimension.
         */
        virtual HydroCouple::Temporal::ITimeSpan* timeSpan() const = 0;

        /*!
         * \brief IDimension of the times.
         * \returns IDimension
         */
        virtual IDimension* timeDimension() const = 0;

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
         * \brief Gets a single value for given x, y, raster band indexes.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xindex is the x dimension index from where to obtain the requested data.
         * \param yindex is the y dimension index from where to obtain the requested data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param data is the output QVariant where data is to be written.
         */
        using IComponentDataItem::getValue;
        virtual void getValue(int timeDimensionIndex, int xindex, int yindex, int bandIndex, QVariant &data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xindex is the x dimension index from where to obtain the requested data.
         * \param yindex is the y dimension index from where to obtain the requested data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::getValues;
        virtual void getValues(int timeDimensionIndex, int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride, int ystride,
                               int bandStride, QVariant data[]) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xindex is the x dimension index from where to obtain the requested data.
         * \param yindex is the y dimension index from where to obtain the requested data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeDimensionIndex, int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride,
                               int ystride, int bandStride, void *data) const = 0;

        /*!
         * \brief Sets a single value for given x, y, raster band indexes.
         * \param timeDimensionIndex is the time dimension index where data is to be written.
         * \param xindex is the x dimension index where to set data.
         * \param yindex is the y dimension index where to set data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param data is the input QVariant to be written.
         */
        using IComponentDataItem::setValue;
        virtual void setValue(int timeDimensionIndex, int xindex, int yindex, int bandIndex, const QVariant &data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index where data is to be written.
         * \param xindex is the x dimension index where to set data.
         * \param yindex is the y dimension index where to set data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the input QVariant array to be written.
         */
        using IComponentDataItem::setValues;
        virtual void setValues(int timeDimensionIndex, int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride,
                               int ystride, int bandStride,  const QVariant data[]) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index where data is to be written.
         * \param xindex is the x dimension index where to set data.
         * \param yindex is the y dimension index where to set data.
         * \param bandIndex is the band dimension index from where to obtain the requested data.
         * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
         * \param xstride is the x size for hyperslab from which to copy data.
         * \param ystride is the x size for hyperslab from which to copy data.
         * \param bandStride is the band size for hyperslab from which to copy data.
         * \param data is the input array to be written.
         */
        virtual void setValues(int timeDimensionIndex,int xindex, int yindex,
                               int bandIndex, int timeStride, int xstride,
                               int ystride, int bandStride, const void *data) = 0;

    };

    /*!
     * \brief The ITimeRasterExchangeItem class.
     */
    class ITimeRasterExchangeItem : public virtual IExchangeItem,
        public virtual ITimeRasterComponentDataItem
    {
      public:
        virtual ~ITimeRasterExchangeItem(){}
    };

    /*!
     * \brief The ITimeRasterArgument class.
     */
    class ITimeRasterArgument : public virtual IArgument,
        public virtual ITimeRasterComponentDataItem
    {
      public:
        virtual ~ITimeRasterArgument(){}
    };

    /*==========================================================================================================*/


    /*!
     * \brief The ITimeRegularGrid2DComponentItem class.
     */
    class ITimeRegularGrid2DComponentDataItem :  public virtual IComponentDataItem
    {
      public:

        virtual ~ITimeRegularGrid2DComponentDataItem(){}

        /*!
         * \brief ITimes associated with this dimension.
         * \returns QList<ITime*>
         */
        virtual QList<HydroCouple::Temporal::ITime*> times() const = 0;

        /*!
         * \brief ITimeSpan associated with this dimension.
         */
        virtual HydroCouple::Temporal::ITimeSpan* timeSpan() const = 0;

        /*!
         * \brief IDimension of the times.
         * \returns IDimension
         */
        virtual IDimension* timeDimension() const = 0;

        /*!
         * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid2D* grid() const = 0;

        /*!
             * \brief Number of X cells IDimension.
             */
        virtual IDimension* numXCellsDimension() const = 0;

        /*!
             * \brief Number of Y cells IDimension.
             */
        virtual IDimension* numYCellsDimension() const = 0;

        /*!
         * \brief cellEdgeDimension
         * \return
         */
        virtual IDimension* cellEdgeDimension() const = 0;

        /*!
         * \brief cellNodeDimension
         * \return
         */
        virtual IDimension* cellNodeDimension() const = 0;

        /*!
         * \brief Gets the value for the xcell index and ycell index.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
         * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data is output into which the requested data is to be written.
         */
        using IComponentDataItem::getValue;
        virtual void getValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex, QVariant &data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param timeStride is the time dimension size for hyperslab from which to copy data.
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::getValues;
        virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex,
                               int timeStride, int xCellStride, int yCellStride, int cellEdgeStride, int cellNodeStride, QVariant data[]) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param timeStride is the time dimension size for hyperslab from which to copy data.
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex,
                               int timeStride, int xCellStride, int yCellStride, int cellEdgeStride, int cellNodeStride, void *data) const = 0;

        /*!
         * \brief Sets the value for the xcell index and ycell index.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
         * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data is input data to be written.
         */
        using IComponentDataItem::setValue;
        virtual void setValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex, const QVariant &data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param timeStride is the time dimension size for hyperslab from which to copy data.
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::setValues;
        virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex,
                               int timeStride, int xCellStride, int yCellStride, int cellEdgeStride, int cellNodeStride, const QVariant data[]) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param timeStride is the time dimension size for hyperslab from which to copy data.
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int cellEdgeIndex, int cellNodeIndex,
                               int timeStride, int xCellStride, int yCellStride, int cellEdgeStride, int cellNodeStride, const void *data) = 0;
    };

    /*!
     * \brief The ITimeRegularGrid2DExchangeItem class.
     */
    class ITimeRegularGrid2DExchangeItem : public virtual IExchangeItem,
        public virtual ITimeRegularGrid2DComponentDataItem
    {
      public:
        virtual ~ITimeRegularGrid2DExchangeItem(){}
    };

    /*!
     * \brief The ITimeRegularGrid2DArgument class.
     */
    class ITimeRegularGrid2DArgument : public virtual IArgument,
        public virtual ITimeRegularGrid2DComponentDataItem
    {
      public:
        virtual ~ITimeRegularGrid2DArgument(){}
    };

    /*==========================================================================================================*/

    /*!
     * \brief The ITimeRegularGrid3DComponentItem class.
     */
    class ITimeRegularGrid3DComponentDataItem :  public virtual IComponentDataItem
    {
      public:
        virtual ~ITimeRegularGrid3DComponentDataItem(){}

        /*!
         * \brief ITimes associated with this dimension.
         * \returns QList<ITime*>
         */
        virtual QList<HydroCouple::Temporal::ITime*> times() const = 0;

        /*!
         * \brief ITimeSpan associated with this dimension.
         */
        virtual HydroCouple::Temporal::ITimeSpan* timeSpan() const = 0;

        /*!
         * \brief IDimension of the times.
         * \returns IDimension
         */
        virtual IDimension* timeDimension() const = 0;

        /*!
         * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
         */
        virtual HydroCouple::Spatial::IRegularGrid3D* grid() const = 0;

        /*!
             * \brief Number of X cells IDimension.
             */
        virtual IDimension* numXCellsDimension() const = 0;

        /*!
             * \brief Number of Y cells IDimension.
             */
        virtual IDimension* numYCellsDimension() const = 0;

        /*!
             * \brief Number of Z cells IDimension.
             */
        virtual IDimension* numZCellsDimension() const = 0;

        /*!
         * \brief cellFaceDimension
         * \return
         */
        virtual IDimension* cellFaceDimension() const = 0;

        /*!
         * \brief cellEdgeDimension
         * \return
         */
        virtual IDimension* cellEdgeDimension() const = 0;

        /*!
         * \brief cellNodeDimension
         * \return
         */
        virtual IDimension* cellNodeDimension() const = 0;

        /*!
         * \brief Gets the value for the xcell index and ycell index.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
         * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
         * \param zCellIndex the z dimension index for the cell from which data is to be retrieved.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data is output into which the requested data is to be written.
         */
        using IComponentDataItem::getValue;
        virtual void getValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex, QVariant &data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param zCellIndex is the z dimension index from where to obtain the requested data.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param zCellStride is the zcell size for hyperslab from which to copy data.
         * \param cellFaceStride
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::getValues;
        virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                               int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex,
                               int xCellStride, int yCellStride, int zCellStride,
                               int cellFaceStride, int cellEdgeStride, int cellNodeStride, QVariant data[]) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param zCellIndex is the z dimension index from where to obtain the requested data.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param zCellStride is the zcell size for hyperslab from which to copy data.
         * \param cellFaceStride
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                               int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex,
                               int xCellStride, int yCellStride, int zCellStride,
                               int cellFaceStride, int cellEdgeStride, int cellNodeStride, void *data) const = 0;

        /*!
         * \brief Sets the value for the xcell, ycell, and zcell indexes.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
         * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
         * \param zCellIndex the z dimension index for the cell from which data is to be retrieved.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param data is input data to be written.
         */
        using IComponentDataItem::setValue;
        virtual void setValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                              int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex, const QVariant &data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param zCellIndex is the z dimension index from where to obtain the requested data.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param zCellStride is the zcell size for hyperslab from which to copy data.
         * \param cellFaceStride
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        using IComponentDataItem::setValues;
        virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                               int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex,
                               int xCellStride, int yCellStride, int zCellStride,
                               int cellFaceStride, int cellEdgeStride, int cellNodeStride, const QVariant data[]) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
         * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
         * \param xCellIndex is the x dimension index from where to obtain the requested data.
         * \param yCellIndex is the y dimension index from where to obtain the requested data.
         * \param zCellIndex is the z dimension index from where to obtain the requested data.
         * \param cellFaceIndex
         * \param cellEdgeIndex
         * \param cellNodeIndex
         * \param xCellStride is the xcell size for hyperslab from which to copy data.
         * \param yCellStride is the ycell size for hyperslab from which to copy data.
         * \param zCellStride is the zcell size for hyperslab from which to copy data.
         * \param cellFaceStride
         * \param cellEdgeStride
         * \param cellNodeStride
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                               int cellFaceIndex, int cellEdgeIndex, int cellNodeIndex,
                               int xCellStride, int yCellStride, int zCellStride,
                               int cellFaceStride, int cellEdgeStride, int cellNodeStride, const void *data) = 0;
    };

    /*!
     * \brief The ITimeRegularGrid3DExchangeItem class.
     */
    class ITimeRegularGrid3DExchangeItem : public virtual IExchangeItem,
        public virtual ITimeRegularGrid3DComponentDataItem
    {
      public:

        virtual ~ITimeRegularGrid3DExchangeItem(){}

    };

    /*!
     * \brief The ITimeRegularGrid3DExchangeItem class.
     */
    class ITimeRegularGrid3DArgument : public virtual IArgument,
        public virtual ITimeRegularGrid3DComponentDataItem
    {
      public:

        virtual ~ITimeRegularGrid3DArgument(){}

    };

  }
}

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem, "HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem, "HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryArgument, "HydroCouple::SpatioTemporal::ITimeGeometryArgument/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINComponentDataItem, "HydroCouple::SpatioTemporal::ITimeTINComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINExchangeItem, "HydroCouple::SpatioTemporal::ITimeTINExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINArgument, "HydroCouple::SpatioTemporal::ITimeTINArgument/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterExchangeItem, "HydroCouple::SpatioTemporal::ITimeRasterExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterArgument, "HydroCouple::SpatioTemporal::ITimeRasterArgument/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DArgument, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DArgument/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DArgument, "HydroCouple::SpatioTemporal::ITimeRegularGrid3DArgument/1.0")

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeGeometryArgument*)

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeTINComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeTINExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeTINArgument*)

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument*)

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRasterExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRasterArgument*)

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DArgument*)

Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DArgument*)

#endif // HYDROCOUPLESPATIOTEMPORAL_H

