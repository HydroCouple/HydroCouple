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
            virtual ~ITimeGeometryComponentItem(){}

      };

      /*!
       * \brief The ITimeGeometryArgument class.
       */
      class ITimeGeometryArgument :
            public virtual HydroCouple::Temporal::ITimeArgument,
            public virtual HydroCouple::Spatial::IGeometryArgument,
            public virtual ITimeGeometryComponentItem
      {
         public:
            virtual ~ITimeGeometryArgument(){}
      };

      /*!
       * \brief The ITimeGeometryExchangeItem class.
       */
      class ITimeGeometryExchangeItem :
            public virtual HydroCouple::Temporal::ITimeExchangeItem,
            public virtual HydroCouple::Spatial::IGeometryExchangeItem,
            public virtual ITimeGeometryComponentItem
      {
         public:
            virtual ~ITimeGeometryExchangeItem(){}

      };


      /*!
       * \brief The ITimePolyhedralSurfaceComponentItem class.
       */
      class ITimePolyhedralSurfaceComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::IPolyhedralSurfaceComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceComponentItem(){}
      };

      /*!
       * \brief The ITimePolyhedralSurfaceArgument class.
       */
      class ITimePolyhedralSurfaceArgument :
            public virtual HydroCouple::Temporal::ITimeArgument,
            public virtual HydroCouple::Spatial::IPolyhedralSurfaceArgument,
            public virtual ITimePolyhedralSurfaceComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceArgument(){}
      };


      /*!
       * \brief The ITimePolyhedralSurfaceExchangeItem class.
       */
      class ITimePolyhedralSurfaceExchangeItem :
            public virtual HydroCouple::Temporal::ITimeExchangeItem,
            public virtual HydroCouple::Spatial::IPolyhedralSurfaceExchangeItem,
            public virtual ITimePolyhedralSurfaceExchangeItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceExchangeItem(){}
      };



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
       * \brief The ITimeRasterArgument class.
       */
      class ITimeRasterArgument :
            public virtual HydroCouple::Temporal::ITimeArgument,
            public virtual HydroCouple::Spatial::IRasterArgument,
            public virtual ITimeRasterComponentItem
      {
         public:
            virtual ~ITimeRasterArgument(){}
      };


      /*!
       * \brief The ITimeRasterExchangeItem class.
       */
      class ITimeRasterExchangeItem :
            public virtual HydroCouple::Temporal::ITimeExchangeItem,
            public virtual HydroCouple::Spatial::IRasterExhangeItem,
            public virtual ITimeRasterComponentItem
      {
         public:
            virtual ~ITimePolyhedralSurfaceExchangeItem(){}
      };

      /*!
       * \brief The ITime2DRegularGridComponentItem class.
       */
      class ITime2DRegularGridComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::I2DRegularGridComponentItem
      {
         public:
            virtual ~ITime2DRegularGridComponentItem(){}
      };

      /*!
       * \brief The ITime2DRegularGridArgument class.
       */
      class ITime2DRegularGridArgument :
            public virtual HydroCouple::Temporal::ITimeArgument,
            public virtual HydroCouple::Spatial::I2DRegularGridArgument,
            public virtual ITime2DRegularGridComponentItem
      {
         public:
            virtual ~ITime2DRegularGridArgument(){}
      };


      /*!
       * \brief The ITime3DRegularGridExchangeItem class.
       */
      class ITime3DRegularGridExchangeItem :
            public virtual HydroCouple::Temporal::ITimeExchangeItem,
            public virtual HydroCouple::Spatial::I3DRegularGridExhangeItem,
            public virtual ITime3DRegularGridComponentItem
      {
         public:
            virtual ~ITime3RegularGridExchangeItem(){}
      };


      /*!
       * \brief The ITime3DRegularGridComponentItem class.
       */
      class ITime3DRegularGridComponentItem :
            public virtual HydroCouple::Temporal::ITimeComponentItem,
            public virtual HydroCouple::Spatial::I3DRegularGridComponentItem
      {
         public:
            virtual ~ITime3DRegularGridComponentItem(){}
      };

      /*!
       * \brief The ITime3DRegularGridArgument class.
       */
      class ITime3DRegularGridArgument :
            public virtual HydroCouple::Temporal::ITimeArgument,
            public virtual HydroCouple::Spatial::I3DRegularGridArgument,
            public virtual ITime3DRegularGridComponentItem
      {
         public:
            virtual ~ITime3DRegularGridArgument(){}
      };


      /*!
       * \brief The ITime3DRegularGridExchangeItem class.
       */
      class ITime3DRegularGridExchangeItem :
            public virtual HydroCouple::Temporal::ITimeExchangeItem,
            public virtual HydroCouple::Spatial::I3DRegularGridExhangeItem,
            public virtual ITime3DRegularGridComponentItem
      {
         public:
            virtual ~ITime3DRegularGridExchangeItem(){}
      };

   }
}

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryComponentItem, "HydroCouple::SpatioTemporal::ITimeGeometryComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryArgument, "HydroCouple::SpatioTemporal::ITimeGeometryArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem, "HydroCouple::SpatioTemporal::ITimeGeometryExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem, "HydroCouple::SpatioTemporal::ITimePolyhedralSurfaceExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterComponentItem, "HydroCouple::SpatioTemporal::ITimeRasterComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterArgument, "HydroCouple::SpatioTemporal::ITimeRasterArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITimeRasterExchangeItem, "HydroCouple::SpatioTemporal::ITimeRasterExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime2DRegularGridComponentItem, "HydroCouple::SpatioTemporal::ITime2DRegularGridComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime2DRegularGridArgument, "HydroCouple::SpatioTemporal::ITime2DRegularGridArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime2DRegularGridExchangeItem, "HydroCouple::SpatioTemporal::ITime2DRegularGridExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime3DRegularGridComponentItem, "HydroCouple::SpatioTemporal::ITime3DRegularGridComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime3DRegularGridArgument, "HydroCouple::SpatioTemporal::ITime3DRegularGridArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::SpatioTemporal::ITime3DRegularGridExchangeItem, "HydroCouple::SpatioTemporal::ITime3DRegularGridExchangeItem/1.0")

#endif // HYDROCOUPLESPATIOTEMPORAL

