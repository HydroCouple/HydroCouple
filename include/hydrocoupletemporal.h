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
      //!ITime interface based on a Modified Julian Date (number and fraction of days since 00:00 November 17, 1858).
      class ITime
      {
         public:
            virtual ~ITime(){}

            //!Date and time as a modified julian day value.
            virtual double dateTime() const = 0;

      };

      //!ITimeSpan specifies a time duration.
      class ITimeSpan : public virtual ITime
      {

         public:
            virtual ~ITimeSpan(){}

            //!Duration of the timespan in days.
            virtual double duration() const = 0;
      };


      //!ITimeComponentItem is an IComponentItem with a temporal attribute.
      class ITimeComponentItem : public virtual IComponentItem
      {

         public:
            virtual ~ITimeComponentItem(){}

            //! IDimension of the times.
            virtual IDimension* timeDimension() const = 0;

            //! ITimes associated with this dimension.
            virtual QList<ITime*> times() const = 0;

            //! ITimeSpan associated with this dimension.
            virtual ITimeSpan* timeSpan() const = 0;

            //! Return ITime for index.
            virtual ITime* time(int timeIndex) const = 0;
      };


      /*!
       * \brief The ITimeArgument class.
       */
      class ITimeArgument : public virtual IArgument , public virtual ITimeComponentItem
      {
         public:
            virtual ~ITimeArgument(){}

            //!Set ITimes associated with this ITimeArgument. Dont forget to update timeSpace and timeDimension accordingly
            virtual void  setTimes(const QList<ITime*>& times) = 0;
      };

      /*!
       * \brief The ITimeExchangeItem class.
       */
      class ITimeExchangeItem : public virtual IExchangeItem, public virtual ITimeComponentItem
      {
         public:
            virtual ~ITimeExchangeItem(){}
      };
   }
}

Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITime, "HydroCouple::Temporal::ITime/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeSpan, "HydroCouple::Temporal::ITimeSpan/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeComponentItem, "HydroCouple::Temporal::ITimeComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeArgument, "HydroCouple::Temporal::ITimeArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeExchangeItem, "HydroCouple::Temporal::ITimeExchangeItem/1.0")

#endif // HYDROCOUPLETEMPORAL

