/*!
 * \file   hydrocoupletemporal.h
 * \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 * \version   2.0.0
 * \description
 * This header file contains the temporal interface definitions for the
 * HydroCouple component-based modeling definitions.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * Lesser GNU Lesser General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 * \copyright Copyright 2014-2024, Caleb Buahin, All rights reserved.
 * \date 2014-2024
 * \pre
 * \bug
 * \warning
 * \todo
 */

#ifndef HYDROCOUPLETEMPORAL_H
#define HYDROCOUPLETEMPORAL_H

#include "hydrocouple.h"

using namespace std;

namespace HydroCouple
{
  //! HydroCouple's interfaces that have a time varying component.
  namespace Temporal
  {
    /*!
     * \brief IDateTime interface based on a Julian day
     */
    class IDateTime : public virtual HydroCouple::IPropertyChanged
    {
    public:
      /*
       * \brief ~IDateTime destructor.
       */
      virtual ~IDateTime() = 0;

      /*!
       * \brief Date and time as a julian day value.
       */
      virtual double julianDay() const = 0;
    };

    /*!
     * \brief ITimeSpan specifies a time duration.
     */
    class ITimeSpan : public virtual IDateTime
    {

    public:
      /*!
       * \brief ~ITimeSpan destructor.
       */
      virtual ~ITimeSpan() = 0;

      /*!
       * Duration of the timespan in days.
       */
      virtual double duration() const = 0;
    };

    /*!
     * \brief The ITimeModelComponent class
     */
    class ITimeModelComponent : public virtual HydroCouple::IModelComponent
    {
    public:
      /*!
       * \brief ~ITimeModelComponent
       */
      virtual ~ITimeModelComponent() = 0;

      /*!
       * \brief currentDateTime
       * \return
       */
      virtual IDateTime *currentDateTime() const = 0;

      /*!
       * \brief timeHorizon
       * \return
       */
      virtual ITimeSpan *timeHorizon() const = 0;
    };

    /*!
     * \brief ITimeComponentItem is an IComponentItem with a temporal attribute.
     * This class cannot be directly instantiated and must be implemented as
     * an abstract class that can be inherited by its specializations e.g.,
     * ITimeSeriesComponentDataItem, ITimeSeriesArgument, ITimeIdBasedComponentDataItem,
     * ITimeIdBasedExchangeItem, ITimeIdBasedArgument, or other geotemporal datasets
     */
    class ITimeComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeComponentItem destructor.
       */
      virtual ~ITimeComponentDataItem() = 0;

      /*!
       * \brief ITimes associated with this dimension.
       * \returns QList<IDateTime*>
       */
      virtual list<IDateTime *> times() const = 0;

      /*!
       * \brief time
       * \param timeIndex
       * \return
       */
      virtual IDateTime *time(int timeIndex) const = 0;

      /*!
       * \brief timeCount
       * \return
       */
      virtual int timeCount() const = 0;

      /*!
       * \brief ITimeSpan associated with this dimension.
       */
      virtual ITimeSpan *timeSpan() const = 0;

      /*!
       * \brief IDimension of the times.
       * \returns IDimension
       */
      virtual IDimension *timeDimension() const = 0;
    };

    /*!
     * \brief The ITimeSeriesComponentDataItem class
     */
    class ITimeSeriesComponentDataItem : public virtual ITimeComponentDataItem
    {

      using IComponentDataItem::getValue;
      using IComponentDataItem::setValue;

    public:
      virtual ~ITimeSeriesComponentDataItem() = 0;

      /*!
       * \brief Gets a single value for given time dimension index.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param data is pointer to the location where data is to be written.
       */
      virtual void getValue(int timeIndex, void *data) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param timeStride is the size for hyperslab from which to copy data.
       * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       */
      virtual void getValues(int timeIndex, int timeStride, void *data) const = 0;

      /*!
       * \brief Sets a single value for given time dimension index.
       * \param timeIndex is the time dimension index where data is to be written.
       * \param data is pointer to the data to be written.
       */
      virtual void setValue(int timeIndex, const void *data) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given
       * time dimension index and size for a hyperslab.
       * \param timeIndex is the time dimension index where data is to be written.
       * \param timeStride is the size for hyperslab where data is to be written.
       * \param data is the input multi dimensional array to be written.
       */
      virtual void setValues(int timeIndex, int timeStride, const void *data) = 0;
    };

    /*!
     * \brief The ITimeIdBasedComponentDataItem class
     */
    class ITimeIdBasedComponentDataItem : public virtual IComponentDataItem,
                                          public virtual ITimeComponentDataItem
    {

      using IComponentDataItem::getValue;
      using IComponentDataItem::setValue;

    public:

      /*!
       * \brief ~ITimeIdBasedComponentDataItem destructor.
       */
      virtual ~ITimeIdBasedComponentDataItem() = 0;

      /*!
       * \brief identifiers associated with this dimension.
       * \return list<string> of identifiers
       */
      virtual list<string> identifiers() const = 0;

      /*!
       * \brief idDimensions associated with this dimension.
       * \return IDimension of the identifiers associated with this dimension.
       */
      virtual IDimension *identifierDimension() const = 0;

      /*!
       * \brief Gets a single value for given id dimension index.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param idIndex is the id dimension index from where to obtain the requested data.
       * \param data is a pre-allocated pointer to the location data is to be written.
       */
      virtual void getValue(int timeIndex, int idIndex, void *data) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given id dimension index and size for a hyperslab.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param idIndex is the id dimension index from where to obtain the requested data.
       * \param timeStride is the size for hyperslab from which to copy data.
       * \param idStride is the size for hyperslab from which to copy data.
       * \param data is a 1d array where data is to be written. Must be allocated beforehand with the correct data type.
       */
      virtual void getValues(int timeIndex, int idIndex, int timeStride, int idStride, void *data) const = 0;

      /*!
       * \brief Sets a single value for given id dimension index.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param idIndex is the id dimension index where data is to be written.
       * \param data is the pointer to a pre-allocated location where data to be written.
       */
      virtual void setValue(int timeIndex, int idIndex, const void *data) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given
       * id dimension index and size for a hyperslab.
       * \param timeIndex is the time dimension index from where to obtain the requested data.
       * \param idIndex is the id dimension index where data is to be written.
       * \param timeStride is the size for hyperslab where data is to be written.
       * \param idStride is the size for hyperslab where data is to be written.
       * \param data is the input 1d array to be written.
       */
      virtual void setValues(int timeIndex, int idIndex, int timeStride, int idStride, const void *data) = 0;
    };
  }
}

#endif // HYDROCOUPLETEMPORAL_H