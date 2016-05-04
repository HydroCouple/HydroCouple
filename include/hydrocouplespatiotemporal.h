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
 *  \warning
 */

#ifndef HYDROCOUPLESPATIOTEMPORAL
#define HYDROCOUPLESPATIOTEMPORAL

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
             * \returns An IGeometryCollection associated with this IGeometryComponentItem.
             */
            virtual HydroCouple::Spatial::IGeometryCollection* geometryCollection() const = 0;

            /*!
             * \returns An IGeometry for the geometry index.
             */
            virtual HydroCouple::Spatial::IGeometry* geometry(int geometryIndex) const = 0;

            /*!
             * \returns The IDimension for this IGeometryComponentItem.
             */
            virtual IDimension* geometryDimension() const = 0;

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
             * \brief Gets a single value for given time dimension index.
             *
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int geometryDimensionIndex, QVariant& data) const = 0;

            /*!
            * \brief Gets a multi-dimensional array of values for given dimension time dimension index and size for a hyperslab.
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
            */
            virtual void getValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
            */
            virtual void getValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, void* data) const = 0;

            //!Sets a single value for given time dimension index.
            /*!
            * \param timeDimensionIndex is the time dimension index where data is to be written.
            * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
            * \param data is the input QVariant to be written.
            */
            virtual void setValue(int timeDimensionIndex, int geometryDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the input multi dimensional array to be written.
            */
            virtual void setValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the input multi dimensional array to be written.
            */
            virtual void setValues(int timeDimensionIndex, int geometryDimensionIndex, int timeStride, int geomStride, const void* data) = 0;

      };

      /*!
       * \brief The ITimeGeometryExchangeItem class.
       */
      class ITimeGeometryExchangeItem :  public virtual IExchangeItem,  public virtual ITimeGeometryComponentDataItem
      {
         public:
            virtual ~ITimeGeometryExchangeItem(){}

      };

      /*!
       * \brief The ITINComponentItem class.
       */
      class ITimeTINComponentDataItem : public virtual IComponentDataItem
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

            /*!
             * \returns An ITriangle for the patch index.
             */
            virtual HydroCouple::Spatial::ITriangle* triangle(int patchIndex) const = 0;

            /*!
             * \returns The ITINPatchDimension for this ITINComponentDataItem triangles.
             */
            virtual IDimension* TINPatchDimension() const = 0;

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

            //!Gets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size of the time dimension for hyperslab from which to copy data.
             * \param patchStride is the size of the patch dimension for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride, QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size of the time dimension for hyperslab from which to copy data.
             * \param patchStride is the size of the patch dimension for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int timeDimensionIndex, int patchDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size of the time dimension for hyperslab where data is to be written.
             * \param patchStride is the size of the time dimension for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeDimensionIndex, int patchDimensionIndex , int timeStride, int patchStride, const void* data) = 0;
      };

      /*!
       * \brief The ITimeTINExchangeItem class.
       */
      class ITimeTINExchangeItem : public virtual IExchangeItem, public virtual ITimeTINComponentDataItem
      {
         public:
            virtual ~ITimeTINExchangeItem(){}
      };

      /*!
       * \brief The ITimePolyhedralSurfaceComponentItem class.
       */
      class ITimePolyhedralSurfaceComponentDataItem : public virtual IComponentDataItem
      {
         public:

            /*!
             * \brief ~ITimePolyhedralSurfaceComponentItem.
             */
            virtual ~ITimePolyhedralSurfaceComponentDataItem(){}

            /*!
             * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceDimension.
             */
            virtual HydroCouple::Spatial::IPolyhedralSurface* polyhedralSurface() const = 0;

            /*!
             * \returns An IPolygon for the patch index.
             */
            virtual HydroCouple::Spatial::IPolygon* polygon(int patchIndex) const = 0;

            /*!
             * \returns The IDimension for this IPolyhedralSurfaceComponentItem patches.
             */
            virtual IDimension* polyhedralSurfacePatchDimension() const = 0;

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

            //!Gets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride,  QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeDimensionIndex, int patchDimensionIndex, int timeStride, int patchStride, const void* data) = 0;
      };

      /*!
       * \brief The ITimePolyhedralSurfaceExchangeItem class.
       */
      class ITimePolyhedralSurfaceExchangeItem : public virtual IExchangeItem, public virtual ITimePolyhedralSurfaceComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceExchangeItem(){}
      };

      /*!
       * \brief The ITimeRasterComponentDataItem class.
       */
      class ITimeRasterComponentDataItem : public virtual IComponentDataItem
      {
         public:
            virtual ~ITimeRasterComponentDataItem(){}

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
             * \brief IRaster associated with this IRasterComponentDataItem.
             */
            virtual HydroCouple::Spatial::IRaster* raster() const = 0;

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
             * \brief Gets a single value for given x, y, raster band indexes.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int xindex, int yindex, int band, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xindex, int yindex, int band, int timeStride, int xstride, int ystride,  QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xindex, int yindex, int band, int timeStride, int xstride, int ystride,  void* data) const = 0;

            /*!
             * \brief Sets a single value for given x, y, raster band indexes.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int timeDimensionIndex, int xindex, int yindex, int band, int timeStride, int xstride, int ystride, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the input QVariant array to be written.
             */
            virtual void setValues(int timeDimensionIndex, int xindex, int yindex, int band, int timeStride, int xstride, int ystride,  const QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the input array to be written.
             */
            virtual void setValues(int timeDimensionIndex,int xindex, int yindex, int band, int timeStride, int xstride, int ystride,  const void* data) = 0;

      };

      /*!
       * \brief The ITimeRasterExchangeItem class.
       */
      class ITimeRasterExchangeItem : public virtual IExchangeItem, public virtual ITimeRasterComponentDataItem
      {
         public:
            virtual ~ITimeRasterExchangeItem(){}
      };

      /*!
       * \brief The ITimeRegularGrid2DComponentItem class.
       */
      class ITimeRegularGrid2DComponentDataItem : private virtual IComponentDataItem
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
             * \brief Number of X cells IDimension.
             */
            virtual IDimension* numXCellsDimension() const = 0;

            /*!
             * \brief Number of Y cells IDimension.
             */
            virtual IDimension* numYCellsDimension() const = 0;

            /*!
             * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
            */
            virtual HydroCouple::Spatial::IRegularGrid2D* grid() const = 0;

            /*!
             * \brief Gets the value for the xcell index and ycell index.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
             * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
             * \param data is output into which the requested data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param timeStride is the time dimension size for hyperslab from which to copy data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int timeStride, int xCellStride, int yCellStride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param timeStride is the time dimension size for hyperslab from which to copy data.
             * \param xCellStride is the ycell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int timeStride, int xCellStride, int yCellStride, void* data) const = 0;

            /*!
             * \brief Sets the value for the xcell index and ycell index.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex the x dimension index for the cell from which data is to be written.
             * \param yCellIndex the y dimension index for the cell from which data is to be written.
             * \param data is input data to be written.
             */
            virtual void setValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param timeStride is the time dimension size for hyperslab where data is to be written.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int timeStride, int xCellStride, int yCellStride, QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param timeStride is the time dimension size for hyperslab where data is to be written.
             * \param xCellStride is the xcell dimension size for hyperslab where data is to be written.
             * \param yCellStride is the ycell dimension size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int timeStride, int xCellStride, int yCellStride, void* data) = 0;
      };

      /*!
       * \brief The ITimeRegularGrid2DExchangeItem class.
       */
      class ITimeRegularGrid2DExchangeItem : public virtual IExchangeItem, public virtual ITimeRegularGrid2DComponentDataItem
      {
         public:
            virtual ~ITimeRegularGrid2DExchangeItem(){}
      };

      /*!
       * \brief The ITimeRegularGrid3DComponentItem class.
       */
      class ITimeRegularGrid3DComponentDataItem : public virtual IComponentDataItem
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
             * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
            */
            virtual HydroCouple::Spatial::IRegularGrid3D* grid() const = 0;

            /*!
             * \brief Gets the value for the xcell index and ycell index.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
             * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
             * \param zCellIndex the y dimension index for the cell from which data is to be retrieved.
             * \param data is output into which the requested data is to be written.
             */
            virtual void getValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param zCellIndex is the z dimension index from where to obtain the requested data.
             * \param timeStride is the time dimension size for hyperslab from which to copy data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param zCellStride is the zcell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                                   int timeStride, int xCellStride, int yCellStride, int zCellStride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param zCellIndex is the z dimension index from where to obtain the requested data.
             * \param timeStride is the time dimension size for hyperslab from which to copy data.
             * \param xCellStride is the ycell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param zCellStride is the zcell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeDimensionIndex, int xCellIndex, int yCellIndex,int zCellIndex,
                                   int timeStride, int xCellStride, int yCellStride, int zCellStride, void* data) const = 0;

            /*!
             * \brief Sets the value for the xcell, ycell, and zcell indexes.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex the x dimension index for the cell from which data is to be written.
             * \param yCellIndex the y dimension index for the cell from which data is to be written.
             * \param zCellIndex the z dimension index for the cell from which data is to be written.
             * \param data is input data to be written.
             */
            virtual void setValue(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param zCellIndex is the z dimension index where to write data.
             * \param timeStride is the time dimension size for hyperslab where data is to be written.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param zCellStride is the zcell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                                   int timeStride, int xCellStride, int yCellStride, int zCellStride, QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param zCellIndex is the y dimension index where to write data.
             * \param timeStride is the time dimension size for hyperslab where data is to be written.
             * \param xCellStride is the xcell dimension size for hyperslab where data is to be written.
             * \param yCellStride is the ycell dimension size for hyperslab where data is to be written.
             * \param zCellStride is the ycell dimension size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int timeDimensionIndex, int xCellIndex, int yCellIndex, int zCellIndex,
                                   int timeStride, int xCellStride, int yCellStride, int zCellStride, void* data) = 0;
      };

      /*!
       * \brief The ITimeRegularGrid3DExchangeItem class.
       */
      class ITimeRegularGrid3DExchangeItem : public virtual IExchangeItem, public virtual ITimeRegularGrid3DComponentDataItem
      {
         public:
            virtual ~ITimeRegularGrid3DExchangeItem(){}

      };

   }
}

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem, "HydroCouple::SpatioTemporal::ITimeGeometryComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem, "HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINComponentDataItem, "HydroCouple::SpatioTemporal::ITimeTINComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINExchangeItem, "HydroCouple::SpatioTemporal::ITimeTINExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRasterComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterExchangeItem, "HydroCouple::SpatioTemporal::ITimeRasterExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentDataItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")
#endif // HYDROCOUPLESPATIOTEMPORAL

