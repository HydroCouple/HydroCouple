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
      class ITimeGeometryComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IGeometryComponentItem
      {
         public:

            /*!
             * \brief ~ITimeGeometryComponentItem.
             */
            virtual ~ITimeGeometryComponentItem(){}

      };

      /*!
       * \brief The ITimeGeometryValueSet class.
       */
      class ITimeGeometryValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~ITimeGeometryValueSet.
             */
            virtual ~ITimeGeometryValueSet(){}

            /*!
             * \brief ITimeGeometryComponentItem associated with this ITimeGeometryValueSet.
             */
            virtual ITimeGeometryComponentItem* timeGeometryComponentItem() const = 0;


            /*!
             * \brief Gets a single value for given time dimension index.
             *
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(size_t timeDimensionIndex, size_t geometryDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
            */
            virtual void getValues(size_t timeDimensionIndex, size_t geometryDimensionIndex, size_t timeStride, size_t geomStride, QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
            */
            virtual void getValues(size_t timeDimensionIndex, size_t geometryDimensionIndex, size_t timeStride, size_t geomStride, void* data) const = 0;

            //!Sets a single value for given time dimension index.
            /*!
            * \param timeDimensionIndex is the time dimension index where data is to be written.
            * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
            * \param data is the input QVariant to be written.
            */
            virtual void setValue(size_t timeDimensionIndex, size_t geometryDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the input multi dimensional array to be written.
            */
            virtual void setValues(size_t timeDimensionIndex, size_t geometryDimensionIndex, size_t timeStride, size_t geomStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
            /*!
            * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
            * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
            * \param timeStride is the size for the time dimension for hyperslab from which to copy data.
            * \param geomStride is the size for the geometry dimension for hyperslab from which to copy data.
            * \param data is the input multi dimensional array to be written.
            */
            virtual void setValues(size_t timeDimensionIndex, size_t geometryDimensionIndex, size_t timeStride, size_t geomStride, const void* data) = 0;
      };

      /*!
       * \brief The ITimeGeometryArgument class.
       */
      class ITimeGeometryArgument :
            public virtual IArgument,
            public virtual ITimeGeometryComponentItem
      {
         public:
            virtual ~ITimeGeometryArgument(){}

            //!Set ITimes associated with this ITimeArgument. Dont forget to update timeSpace and timeDimension accordingly
            virtual void  setTimes(const QList<HydroCouple::Temporal::ITime*>& times) = 0;

            /*!
             * \brief Sets an IGeometryCollection associated with this IGeometryArgument.
             * Update Appropriate dimensions.
             *
             * \param geometry to set.
             */
            virtual void setGeometryCollection(HydroCouple::Spatial::IGeometryCollection* geomtery) = 0;

            /*!
             * \brief ITimeGeometryValueSet objects associated with this ITimeGeometryArgument.
             * \returns A list of ITimeGeometryValueSet objects associated with this ITimeGeometryArgument.
             */
            virtual QList<ITimeGeometryValueSet*> timeGeometryValueSets() const = 0;

      };

      /*!
       * \brief The ITimeGeometryExchangeItem class.
       */
      class ITimeGeometryExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimeGeometryComponentItem
      {
         public:
            virtual ~ITimeGeometryExchangeItem(){}

            /*!
             * \brief IGeometryValueSet  associated with this IGeometryExchangeItem.
             * \returns An IGeometryValueSet associated with this IGeometryExchangeItem.
             */
            virtual ITimeGeometryValueSet* timeGeometryValues() const = 0;

      };

      //************************************************************************

      /*!
       * \brief The ITINComponentItem class.
       */
      class ITimeTINComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::ITINComponentItem
      {
         public:

            /*!
             * \brief ~ITimeTINComponentItem.
             */
            virtual ~ITimeTINComponentItem(){}
      };

      /*!
       * \brief The ITimeTINValueSet class.
       */
      class ITimeTINValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ITimeTINValueSet
             */
            virtual ITimeTINValueSet(){}

            /*!
             * \returns The ITimeTINComponentItem associated with this ITimeTINValueSet .
             */
            virtual ITimeTINComponentItem* timeTINComponentItem() const = 0;

            //!Gets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(size_t timeDimensionIndex, size_t patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size of the time dimension for hyperslab from which to copy data.
             * \param patchStride is the size of the patch dimension for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride, QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size of the time dimension for hyperslab from which to copy data.
             * \param patchStride is the size of the patch dimension for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(size_t timeDimensionIndex, size_t patchDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size of the time dimension for hyperslab where data is to be written.
             * \param patchStride is the size of the time dimension for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t patchDimensionIndex , size_t timeStride, size_t patchStride, const void* data) = 0;

      };

      /*!
       * \brief The ITimeTINArgument class.
       */
      class ITimeTINArgument :
            public virtual IArgument,
            public virtual ITimeTINComponentItem
      {
         public:

            /*!
             * \brief ~ITimeTINArgument.
             */
            virtual ~ITimeTINArgument(){}

            /*!
             * \brief Set ITimes associated with this ITimeTINArgument.
             * Dont forget to update timeSpace and timeDimension accordingly.
             */
            virtual void  setTimes(const QList<HydroCouple::Temporal::ITime*>& times) = 0;

            /*!
             * \brief Sets a HydroCouple::Spatial::ITIN associated with this ITimeTINArgument.
             * Update Appropriate dimensions.
             *
             */
            virtual void setTIN(HydroCouple::Spatial::ITIN* TINSurface) = 0;

            /*!
             * \brief ITimeTINValueSet objects associated with this ITimeTINArgument.
             * \returns A list of ITimeTINValueSet objects associated with this ITimeTINArgument.
             */
            virtual QList<ITimeTINValueSet*> timeTINValueSets() const = 0;
      };

      /*!
       * \brief The ITimeTINExchangeItem class.
       */
      class ITimeTINExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimeTINComponentItem
      {
         public:
            virtual ~ITimeTINExchangeItem(){}

            /*!
             * \brief ITimeTINValueSet associated with this ITimeTINExchangeItem.
             * \returns A ITimeTINValueSet objects associated with this ITimeTINExchangeItem.
             */
            virtual ITimeTINValueSet* timeTINValues() const = 0;
      };

      //************************************************************************

      /*!
       * \brief The ITimePolyhedralSurfaceComponentItem class.
       */
      class ITimePolyhedralSurfaceComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IPolyhedralSurfaceComponentItem
      {
         public:

            /*!
             * \brief ~ITimePolyhedralSurfaceComponentItem.
             */
            virtual ~ITimePolyhedralSurfaceComponentItem(){}
      };

      /*!
       * \brief The ITimePolyhedralSurfaceValueSet class,
       */
      class ITimePolyhedralSurfaceValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ITimePolyhedralSurfaceValueSet.
             */
            virtual ITimePolyhedralSurfaceValueSet(){}

            /*!
             * \returns The ITimePolyhedralSurfaceComponentItem associated with this ITimePolyhedralSurfaceValueSet .
             */
            virtual ITimePolyhedralSurfaceComponentItem* timePolyhedralSurfaceComponentItem() const = 0;

            //!Gets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(size_t timeDimensionIndex, size_t patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride,  QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param timeStride is the size for the time dimensions hyperslab where data is to be written.
             * \param patchStride is the size for the patch dimensions hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t patchDimensionIndex, size_t timeStride, size_t patchStride, const void* data) = 0;

      };

      /*!
       * \brief The ITimePolyhedralSurfaceArgument class.
       */
      class ITimePolyhedralSurfaceArgument :
            public virtual IArgument,
            public virtual ITimePolyhedralSurfaceComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceArgument(){}

            /*!
             * \brief Set ITime values associated with this ITimePolyhedralSurfaceArgument.
             * Dont forget to update timeSpace and timeDimension accordingly.
             */
            virtual void  setTimes(const QList<HydroCouple::Temporal::ITime*>& times) = 0;

            /*!
             * \brief Sets an HydroCouple::Spatial::IPolyhedralSurface associated with this ITimePolyhedralSurfaceArgument.
             * Update Appropriate dimensions.
             *
             */
            virtual void setPolyhedralSurface(HydroCouple::Spatial::IPolyhedralSurface* geomtery) = 0;

            /*!
             * \brief ITimePolyhedralSurfaceValueSet objects associated with this ITimePolyhedralSurfaceArgument.
             * \returns A list of ITimePolyhedralSurfaceValueSet objects associated with this ITimePolyhedralSurfaceArgument.
             */
            virtual QList<ITimePolyhedralSurfaceValueSet*> timePatchValueSets() const = 0;
      };

      /*!
       * \brief The ITimePolyhedralSurfaceExchangeItem class.
       */
      class ITimePolyhedralSurfaceExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimePolyhedralSurfaceComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceExchangeItem(){}

            /*!
             * \brief ITimePolyhedralSurfaceValueSet associated with this ITimePolyhedralSurfaceExchangeItem.
             * \returns A ITimePolyhedralSurfaceValueSet objects associated with this ITimePolyhedralSurfaceExchangeItem.
             */
            virtual ITimePolyhedralSurfaceValueSet* timePolyhedralSurfaceValues() const = 0;
      };

      //************************************************************************


      /*!
       * \brief The ITimeRasterComponentItem class.
       */
      class ITimeRasterComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IRasterComponentItem
      {
         public:
            virtual ~ITimeRasterComponentItem(){}
      };

      /*!
       * \brief The ITimeRasterValueSet class.
       */
      class ITimeRasterValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~ITimeRasterValueSet
             */
            virtual ~ITimeRasterValueSet(){}

            /*!
             * \returns The ITimeRasterComponentItem associated with this ITimeRasterValueSet .
             */
            virtual ITimeRasterComponentItem* timeRasterComponentItem() const = 0;

            /*!
             * \brief Gets a single value for given x, y, raster band indexes.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(size_t timeDimensionIndex, size_t xindex, size_t yindex, int band, QVariant& data) const = 0;

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
            virtual void getValues(size_t timeDimensionIndex, size_t xindex, size_t yindex, int band, size_t timeStride, size_t xstride, size_t ystride,  QVariant* data) const = 0;

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
            virtual void getValues(size_t timeDimensionIndex, size_t xindex, size_t yindex, int band, size_t timeStride, size_t xstride, size_t ystride,  void* data) const = 0;

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
            virtual void setValue(size_t timeDimensionIndex, size_t xindex, size_t yindex, int band, size_t timeStride, size_t xstride, size_t ystride, const QVariant& data) = 0;

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
            virtual void setValues(size_t timeDimensionIndex, size_t xindex, size_t yindex, int band, size_t timeStride, size_t xstride, size_t ystride,  const QVariant* data) = 0;

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
            virtual void setValues(size_t timeDimensionIndex,size_t xindex, size_t yindex, int band, size_t timeStride, size_t xstride, size_t ystride,  const void* data) = 0;



      };

      /*!
       * \brief The ITimeRasterArgument class.
       */
      class ITimeRasterArgument :
            public virtual IArgument,
            public virtual ITimeRasterComponentItem
      {
         public:
            virtual ~ITimeRasterArgument(){}

            /*!
             * \brief Set ITimes associated with this ITimeRasterArgument. Dont forget to update timeSpace and timeDimension accordingly.
             */
            virtual void  setTimes(const QList<ITime*>& times) = 0;

            /*!
             * \brief Set IRaster associated with IRasterArgument.
             *
             * \param raster IRaster to set.
             */
            virtual void setRaster(IRaster* raster) = 0;

            /*!
             * \brief ITimeRasterValueSet objects associated with this ITimeRasterArgument.
             * \returns A list of ITimeRasterValueSet objects associated with this ITimeRasterArgument.
             */
            virtual QList<ITimeRasterValueSet*> timeRasterValueSets() const = 0;
      };


      /*!
       * \brief The ITimeRasterExchangeItem class.
       */
      class ITimeRasterExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimeRasterComponentItem
      {
         public:
            virtual ~ITimeRasterExchangeItem(){}

            /*!
             * \brief ITimeRasterValueSet associated with this ITimeRasterExchangeItem.
             * \returns A ITimeRasterValueSet objects associated with this ITimeRasterExchangeItem.
             */
            virtual ITimeRasterValueSet* timeRasterValues() const = 0;
      };

      //************************************************************************

      /*!
       * \brief The ITimeRegularGrid2DComponentItem class.
       */
      class ITimeRegularGrid2DComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IRegularGrid2DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid2DComponentItem(){}
      };

      /*!
       * \brief The ITimeRegularGrid2DValueSet class.
       */
      class ITimeRegularGrid2DValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~ITimeRegularGrid2DValueSet
             */
            virtual ~ITimeRegularGrid2DValueSet(){}

            /*!
             * \returns The ITimeRegularGrid2DComponentItem associated with this ITimeRegularGrid2DValueSet.
             */
            virtual ITimeRegularGrid2DComponentItem* timeRegularGrid2DComponentItem() const = 0;

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
       * \brief The ITimeRegularGrid2DArgument class.
       */
      class ITimeRegularGrid2DArgument :
            public virtual IArgument,
            public virtual ITimeRegularGrid2DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid2DArgument(){}

            /*!
             * \brief Set ITime associated with this ITimeRegularGrid2DArgument.
             * Dont forget to update timeSpace and timeDimension accordingly.
             */
            virtual void  setTimes(const QList<HydroCouple::Temporal::ITime*>& times) = 0;

            /*!
             * \brief Set HydroCouple::Spatial::IRegularGrid2D associated with ITimeRegularGrid2DArgument.
              */
            virtual void setGrid(HydroCouple::Spatial::IRegularGrid2D* grid) = 0;

            /*!
             * \brief ITimeRegularGrid2DValueSet objects associated with this IRegularGrid2DArgument.
             * \returns A list of ITimeRegularGrid2DValueSet objects associated with this IRegularGrid2DArgument.
             */
            virtual QList<ITimeRegularGrid2DValueSet*> timeRegular2DValueSets() const = 0;
      };


      /*!
       * \brief The ITimeRegularGrid2DExchangeItem class.
       */
      class ITimeRegularGrid2DExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimeRegularGrid2DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid2DExchangeItem(){}

            /*!
             * \brief ITimeRegularGrid2DValueSet associated with this ITimeRegularGrid2DComponentItem.
             * \returns A ITimeRegularGrid2DValueSet objects associated with this ITimeRegularGrid2DComponentItem.
             */
            virtual ITimeRegularGrid2DValueSet* timeRegularGrid2DValues() const = 0;
      };

      //************************************************************************

      /*!
       * \brief The ITimeRegularGrid3DComponentItem class.
       */
      class ITimeRegularGrid3DComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IRegularGrid3DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid3DComponentItem(){}
      };


      /*!
       * \brief The ITimeRegularGrid3DValueSet class.
       */
      class ITimeRegularGrid3DValueSet : public virtual IValueSet
      {
         public:
            /*!
             * \brief ~ITimeRegularGrid2DValueSet
             */
            virtual ~ITimeRegularGrid3DValueSet(){}

            /*!
             * \returns The ITimeRegularGrid3DComponentItem associated with this ITimeRegularGrid3DValueSet.
             */
            virtual ITimeRegularGrid3DComponentItem* timeRegularGrid3DComponentItem() const = 0;

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
       * \brief The ITimeRegularGrid3DArgument class.
       */
      class ITimeRegularGrid3DArgument :
            public virtual IArgument,
            public virtual ITimeRegularGrid3DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid3DArgument(){}

            /*!
             * \brief Set ITime associated with this ITimeRegularGrid3DArgument.
             * Dont forget to update timeSpace and timeDimension accordingly.
             */
            virtual void  setTimes(const QList<HydroCouple::Temporal::ITime*>& times) = 0;

            /*!
             * \brief Set HydroCouple::Spatial::IRegularGrid3D associated with ITimeRegularGrid3DArgument.
              */
            virtual void setGrid(HydroCouple::Spatial::IRegularGrid3D* grid) = 0;

            /*!
             * \brief ITimeRegularGrid3DValueSet objects associated with this IRegularGrid3DArgument.
             * \returns A list of ITimeRegularGrid3DValueSet objects associated with this IRegularGrid3DArgument.
             */
            virtual QList<ITimeRegularGrid3DValueSet*> timeRegular3DValueSets() const = 0;
      };


      /*!
       * \brief The ITimeRegularGrid3DExchangeItem class.
       */
      class ITimeRegularGrid3DExchangeItem :
            public virtual IExchangeItem,
            public virtual ITimeRegularGrid3DComponentItem
      {
         public:
            virtual ~ITimeRegularGrid3DExchangeItem(){}


            /*!
             * \brief ITimeRegularGrid3DValueSet associated with this ITimeRegularGrid3DComponentItem.
             * \returns A ITimeRegularGrid3DValueSet objects associated with this ITimeRegularGrid3DComponentItem.
             */
            virtual ITimeRegularGrid3DValueSet* timeRegularGrid3DValues() const = 0;
      };

   }
}

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryComponentItem, "HydroCouple::SpatioTemporal::ITimeGeometryComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryValueSet, "HydroCouple::SpatioTemporal::ITimeGeometryValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryArgument, "HydroCouple::SpatioTemporal::ITimeGeometryArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem, "HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINComponentItem, "HydroCouple::SpatioTemporal::ITimeTINComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINValueSet, "HydroCouple::SpatioTemporal::ITimeTINValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINArgument, "HydroCouple::SpatioTemporal::ITimeTINArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeTINExchangeItem, "HydroCouple::SpatioTemporal::ITimeTINExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceValueSet, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterComponentItem, "HydroCouple::SpatioTemporal::ITimeRasterComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterValueSet, "HydroCouple::SpatioTemporal::ITimeRasterValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterArgument, "HydroCouple::SpatioTemporal::ITimeRasterArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterExchangeItem, "HydroCouple::SpatioTemporal::ITimeRasterExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DValueSet, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DArgument, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid2DExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid3DComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DValueSet, "HydroCouple::SpatioTemporal::ITimeRegularGrid3DValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DArgument, "HydroCouple::SpatioTemporal::ITimeRegularGrid3DArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRegularGrid3DExchangeItem, "HydroCouple::SpatioTemporal::ITimeRegularGrid3DExchangeItem/1.0")

#endif // HYDROCOUPLESPATIOTEMPORAL

