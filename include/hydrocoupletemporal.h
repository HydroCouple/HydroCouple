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

            //!Date and time as a modified julian day value.
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
      class ITimeComponentItem : public virtual IComponentItem
      {

         public:

            /*!
             * \brief ~ITimeComponentItem.
             */
            virtual ~ITimeComponentItem(){}

            /*!
             * \brief IDimension of the times.
             * \returns IDimension
             */
            virtual IDimension* timeDimension() const = 0;

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
             * \returns ITime for index.
             */
            virtual ITime* time(int timeIndex) const = 0;
      };

      /*!
       * \brief The ITimeValueSet class is a time varying IValueSet class.
       */
      class ITimeValueSet : public virtual HydroCouple::IValueSet
      {
         public:

            /*!
             * \brief ~ITimeValueSet
             */
            virtual ~ITimeValueSet() { }

            /*!
             * \brief ITimeComponentItem associated with this ITimeValueSet.
             */
            virtual ITimeComponentItem* timeComponentItem() const = 0;

            /*!
             * \brief Gets a single value for given time dimension index.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(size_t timeDimensionIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given
             * dimension time dimension index and size for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t stride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
             * \param timeDimensionIndex is the time dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(size_t timeDimensionIndex, size_t stride, void* data) const = 0;

            /*!
             * \brief Sets a single value for given time dimension index.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(size_t timeDimensionIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given time
             *  dimension index and size for a hyperslab.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t stride, const QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given
             * time dimension index and size for a hyperslab.
             * \param timeDimensionIndex is the time dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(size_t timeDimensionIndex, size_t stride, const void* data) = 0;

      };

      /*!
       * \brief The ITimeArgument class.
       */
      class ITimeArgument : public virtual IArgument , public virtual ITimeComponentItem
      {
         public:

            /*!
             * \brief ~ITimeArgument.
             */
            virtual ~ITimeArgument(){}

            //!Set ITimes associated with this ITimeArgument. Dont forget to update timeSpace and timeDimension accordingly
            virtual void  setTimes(const QList<ITime*>& times) = 0;

            /*!
             * \brief ITimeValueSet objects associated with this ITimeArgument.
             * \returns A list of ITimeValueSet objects associated with this ITimeArgument.
             */
            virtual QList<ITimeValueSet*> timeValueSets() const = 0;
      };

      /*!
       * \brief The ITimeExchangeItem class.
       */
      class ITimeExchangeItem : public virtual IExchangeItem, public virtual ITimeComponentItem
      {
         public:
            virtual ~ITimeExchangeItem(){}

            /*!
             * \brief ITimeValueSet associated with this ITimeExchangeItem.
             * \returns A ITimeValueSet objects associated with this ITimeArgument.
             */
            virtual ITimeValueSet* timeValues() const = 0;
      };
   }
}

Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITime, "HydroCouple::Temporal::ITime/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeSpan, "HydroCouple::Temporal::ITimeSpan/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeComponentItem, "HydroCouple::Temporal::ITimeComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeValueSet, "HydroCouple::Temporal::ITimeValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeArgument, "HydroCouple::Temporal::ITimeArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeExchangeItem, "HydroCouple::Temporal::ITimeExchangeItem/1.0")

#endif // HYDROCOUPLETEMPORAL

