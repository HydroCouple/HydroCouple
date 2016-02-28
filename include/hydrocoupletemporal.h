/****************************************************************************
**
**  Copyright (C) 2014 Caleb Amoa Buahin
**  Contact: calebgh@gmail.com
**
**  This file is part of HydroCouple.dll
**
**  HydroCouple.dll and its associated files is free software; you can redistribute it and/or modify
**  it under the terms of the Lesser GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  (at your option) any later version.
**
**  HydroCouple.dll and its associated files is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  Lesser GNU General Public License for more details.
**
**  You should have received a copy of the Lesser GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>
**
****************************************************************************/

#ifndef HYDROCOUPLETEMPORAL
#define HYDROCOUPLETEMPORAL

#include "include/hydrocouple.h"

namespace HydroCouple
{
   namespace Data
   {
      //!Time related entities
      namespace Temporal
      {
         //!ITime interface based on a Modified Julian Date (number and fraction of days since 00:00 November 17, 1858).
         class ITime
         {
            public:
               virtual ~ITime(){}

               //!DateTime as a modified julian day value.
               virtual double dateTime() const = 0;

         };

         //!ITimeSpan specifies a time duration
         class ITimeSpan : public virtual ITime
         {

            public:
               virtual ~ITimeSpan(){}

               //!Duration of the timespan in days
               virtual double duration() const = 0;
         };

         //****************************************************************************************************************************

         //!ITimeComponentItem is an IComponentItem with a temporal attribute
         class ITimeComponentItem : public virtual IComponentItem
         {

            public:
               virtual ~ITimeComponentItem(){}

               //!IDimension of the times
               virtual IDimension* timeDimension() const = 0;

               //! ITimes associated with this dimension.
               virtual QList<ITime*> times() const = 0;

               //! ITimeSpan associated with this dimension.
               virtual ITimeSpan* timeSpan() const = 0;

               //! Return ITime for index.
               virtual ITime* time(int timeIndex) const = 0;
         };
      }
   }
}

Q_DECLARE_INTERFACE(HydroCouple::Data::Temporal::ITime, "HydroCouple::Data::Temporal::ITime/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Data::Temporal::ITimeSpan, "HydroCouple::Data::Temporal::ITimeSpan/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Data::Temporal::ITimeComponentItem, "HydroCouple::Data::Temporal::ITimeComponentItem/1.0")

#endif // HYDROCOUPLETEMPORAL

