/*! \file   hydrocoupletemporal.h
 *  \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 *  \version   1.0.0.0
 *  \section   Description
 *  This header files contains interface definitions that have a temporal component for the
 *  HydroCouple component-cased modeling framework.
 *  \section License
 *  hydrocoupletemporal.h, associated files and libraries are free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  Lesser GNU General Public License as published by the Free Software Foundation;
 *  either version 3 of the License, or (at your option) any later version.
 *  hydrocoupletemporal.h its associated files is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 *  \date 2014-2016
 *  \pre
 *  \bug
 *  \warning
 */

#ifndef HYDROCOUPLETEMPORAL
#define HYDROCOUPLETEMPORAL

#include "include/hydrocouple.h"

namespace HydroCouple
{
   //! HydroCouple's interfaces that have a time varying component.
   namespace Temporal
   {

      /*!
       * \brief ITime interface based on a Modified Julian Date
       * (number and fraction of days since 00:00 November 17, 1858).
       */
      class ITime
      {
         public:
            virtual ~ITime(){}

            /*!
             * \brief Date and time as a modified julian day value.
             */
            virtual double dateTime() const = 0;

      };

      /*!
       * \brief ITimeSpan specifies a time duration.
       */
      class ITimeSpan : public virtual ITime
      {

         public:
            virtual ~ITimeSpan(){}

            /*!
             * Duration of the timespan in days.
             */
            virtual double duration() const = 0;
      };

      /*!
       * \brief ITimeComponentItem is an IComponentItem with a temporal attribute.
       */
      class ITimeComponentDataItem : public virtual IComponentDataItem
      {

         public:
            /*!
             * \brief ~ITimeComponentItem.
             */
            virtual ~ITimeComponentDataItem(){}

            /*!
             * \brief ITimes associated with this dimension.
             * \returns QList<ITime*>
             */
            virtual QList<ITime*> times() const = 0;

            /*!
             * \brief ITimeSpan associated with this dimension.
             */
            virtual ITimeSpan* timeSpan() const = 0;

            /*!
             * \brief IDimension of the times.
             * \returns IDimension
             */
            virtual IDimension* timeDimension() const = 0;

            /*!
             * \brief ITimeComponentItem associated with this ITimeValueSet.
             */
            virtual ITimeComponentDataItem* timeComponentDataItem() const = 0;

            /*!
             * \brief Gets a single value for given time dimension index.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int timeIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given
             * dimension time dimension index and size for a hyperslab.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int timeIndex, int stride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int timeIndex, int stride, void* data) const = 0;

            /*!
             * \brief Sets a single value for given time dimension index.
             * \param timeIndex is the time dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int timeIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given time
             *  dimension index and size for a hyperslab.
             * \param timeIndex is the time dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeIndex, int stride, const QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given
             * time dimension index and size for a hyperslab.
             * \param timeIndex is the time dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int timeIndex, int stride, const void* data) = 0;
      };

      /*!
       * \brief The ITimeExchangeItem class.
       */
      class ITimeExchangeItem : public virtual IExchangeItem, public virtual ITimeComponentItem
      {
         public:
            virtual ~ITimeExchangeItem(){}

      };

      /*!
       * \brief The IIdBasedTimeComponentDataItem class
       */
      class IIdBasedTimeComponentDataItem : public virtual IComponentDataItem
      {
         public:
            virtual ~IIdBasedTimeComponentDataItem(){}

            /*!
             * \brief identifiers
             * \return
             */
            virtual QList<QString> identifiers() const = 0;

            /*!
             * \brief idDimensions
             * \return
             */
            virtual IDimension* identifierDimension() const = 0;

            /*!
             * \brief ITimes associated with this dimension.
             * \returns QList<ITime*>
             */
            virtual QList<ITime*> times() const = 0;

            /*!
             * \brief ITimeSpan associated with this dimension.
             */
            virtual ITimeSpan* timeSpan() const = 0;

            /*!
             * \brief IDimension of the times.
             * \returns IDimension
             */
            virtual IDimension* timeDimension() const = 0;

            /*!
             * \brief Gets a single value for given id dimension index.
             * \param idIndex is the id dimension index from where to obtain the requested data.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int idIndex, int timeIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given
             *  id dimension index and size for a hyperslab.
             * \param idIndex is the id dimension index from where to obtain the requested data.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param idStride is the size for hyperslab from which to copy data.
             * \param timeStride is the size for hyperslab from which to copy data.
             * \param data is the 1d array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int idIndex, int timeIndex, int idStride, int timeStride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given id dimension index and size for a hyperslab.
             * \param idIndex is the id dimension index from where to obtain the requested data.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param idStride is the size for hyperslab from which to copy data.
             * \param timeStride is the size for hyperslab from which to copy data.
             * \param data is a 1d array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int idIndex, int timeIndex, int idStride, int timeStride, void* data) const = 0;

            /*!
             * \brief Sets a single value for given id dimension index.
             * \param idIndex is the id dimension index where data is to be written.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int idIndex, int timeIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given time
             *  dimension index and size for a hyperslab.
             * \param idIndex is the time dimension index where data is to be written.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param idStride is the id for hyperslab where data is to be written.
             * \param timeStride is the id for hyperslab where data is to be written.
             * \param data is the input 1d array to be written.
             */
            virtual void setValues(int idIndex, int timeIndex, int idStride, int timeStride, QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given
             * id dimension index and size for a hyperslab.
             * \param idIndex is the id dimension index where data is to be written.
             * \param timeIndex is the time dimension index from where to obtain the requested data.
             * \param idStride is the size for hyperslab where data is to be written.
             * \param timeStride is the size for hyperslab where data is to be written.
             * \param data is the input 1d array to be written.
             */
            virtual void setValues(int idIndex, int timeIndex, int idStride, int timeStride, void* data) = 0;
      };

      /*!
       * \brief The IIdBasedTimeExchangeItem class
       */
      class IIdBasedTimeExchangeItem : public virtual IExchangeItem , public virtual IIdBasedTimeComponentDataItem
      {
         public:
            virtual ~IIdBasedTimeExchangeItem(){}
      };
   }
}

Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITime, "HydroCouple::Temporal::ITime/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeSpan, "HydroCouple::Temporal::ITimeSpan/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeComponentDataItem, "HydroCouple::Temporal::ITimeComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeExchangeItem, "HydroCouple::Temporal::ITimeExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::IIdBasedTimeValueSet, "HydroCouple::Temporal::IIdBasedTimeValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::IIdBasedTimeComponentDataItem, "HydroCouple::Temporal::IIdBasedTimeComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::IIdBasedTimeExchangeItem, "HydroCouple::Temporal::IIdBasedTimeExchangeItem/1.0")


Q_DECLARE_METATYPE(HydroCouple::Temporal::ITime*)
Q_DECLARE_METATYPE(QList<HydroCouple::Temporal::ITime*>)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeSpan*)
Q_DECLARE_METATYPE(QList<HydroCouple::Temporal::ITimeSpan*>)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::IIdBasedTimeComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::IIdBasedTimeExchangeItem*)

#endif // HYDROCOUPLETEMPORAL

