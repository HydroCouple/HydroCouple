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

#ifndef HYDROCOUPLETEMPORAL_H
#define HYDROCOUPLETEMPORAL_H

#include "hydrocouple.h"

namespace HydroCouple
{
  //! HydroCouple's interfaces that have a time varying component.
  namespace Temporal
  {
    /*!
     * \brief ITime interface based on a Modified Julian Date5
     * (number and fraction of days since 00:00 November 17, 1858).
     */
    class ITime : public virtual HydroCouple::IPropertyChanged
    {
      public:

        virtual ~ITime(){}

        /*!
         * \brief Date and time as a modified julian day value.
         */
        virtual double dateTime() const = 0;

        /*!
         * \brief setDateTime
         * \param dateTime
         */
        virtual void setDateTime(double dateTime) = 0;

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
     * This class cannot be directly instantiated and must be implemented as
     * an abstract class that can be inherited by its specializations e.g.,
     * ITimeSeriesComponentDataItem, ITimeSeriesArgument, ITimeIdBasedComponentDataItem,
     * ITimeIdBasedExchangeItem, ITimeIdBasedArgument, or other geotemporal datasets
     */
    class ITimeComponentDataItem : public virtual IComponentDataItem
    {

      public:

        using IComponentDataItem::getValue;
        using IComponentDataItem::getValues;
        using IComponentDataItem::setValue;
        using IComponentDataItem::setValues;

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
         * \brief Gets a single value for given time dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param data is the output QVariant where data is to be written.
         */
        virtual void getValue(int timeIndex, QVariant &data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given
         * dimension time dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param timeStride is the size for hyperslab from which to copy data.
         * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeIndex, int timeStride, QVariant data[]) const = 0;

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
         * \param data is the input QVariant to be written.
         */
        virtual void setValue(int timeIndex, const QVariant &data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given time
         *  dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index where data is to be written.
         * \param timeStride is the size for hyperslab where data is to be written.
         * \param data is the input multi dimensional array to be written.
         */
        virtual void setValues(int timeIndex, int timeStride, const QVariant data[]) = 0;

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
     * \brief The ITimeExchangeItem class.This class cannot be directly instantiated and must be implemented as
     * as an abstract class that can be inherited by its specializations i.e., ITimeSeriesExchangeItem ,  ITimeIdBasedExchangeItem, and other
     * geotemporal datasets.
     */
    class ITimeExchangeItem : public virtual IExchangeItem,
        public virtual ITimeComponentDataItem
    {
      public:

        virtual ~ITimeExchangeItem(){}

    };

    /*!
     * \brief The ITimeArgument class. his class cannot be directly instantiated and must be implemented as
     * as an abstract class that can be inherited by its specializations i.e., ITimeSeriesArgument or  ITimeIdBasedArgument,
     */
    class ITimeArgument : public virtual IArgument,
        public virtual ITimeComponentDataItem
    {
      public:

        virtual ~ITimeArgument(){}
    };

    /*!
     * \brief The ITimeIdBasedComponentDataItem class
     */
    class ITimeIdBasedComponentDataItem : public virtual IComponentDataItem
    {

      public:

        using IComponentDataItem::getValue;
        using IComponentDataItem::getValues;
        using IComponentDataItem::setValue;
        using IComponentDataItem::setValues;

        virtual ~ITimeIdBasedComponentDataItem(){}

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
         * \brief identifiers
         * \return
         */
        virtual QStringList identifiers() const = 0;

        /*!
         * \brief idDimensions
         * \return
         */
        virtual IDimension* identifierDimension() const = 0;

        /*!
         * \brief Gets a single value for given id dimension index.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param idIndex is the id dimension index from where to obtain the requested data.
         * \param data is the output QVariant where data is to be written.
         */
        virtual void getValue(int timeIndex, int idIndex, QVariant &data) const = 0;

        /*!
         * \brief Gets a multi-dimensional array of values for given
         *  id dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param idIndex is the id dimension index from where to obtain the requested data.
         * \param timeStride is the size for hyperslab from which to copy data.
         * \param idStride is the size for hyperslab from which to copy data.
         * \param data is the 1d array where data is to be written. Must be allocated beforehand.
         */
        virtual void getValues(int timeIndex, int idIndex, int timeStride, int idStride, QVariant data[]) const = 0;

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
         * \param data is the input QVariant to be written.
         */
        virtual void setValue(int timeIndex, int idIndex, const QVariant &data) = 0;

        /*!
         * \brief Sets a multi-dimensional array of values for given time
         *  dimension index and size for a hyperslab.
         * \param timeIndex is the time dimension index from where to obtain the requested data.
         * \param idIndex is the time dimension index where data is to be written.
         * \param timeStride is the id for hyperslab where data is to be written.
         * \param idStride is the id for hyperslab where data is to be written.
         * \param data is the input 1d array to be written.
         */
        virtual void setValues(int timeIndex, int idIndex, int timeStride, int idStride, const QVariant data[]) = 0;

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

    /*!
     * \brief The ITimeSeriesIdBasedExchangeItem class
     */
    class ITimeIdBasedExchangeItem : public virtual IExchangeItem,
        public virtual ITimeIdBasedComponentDataItem
    {
      public:
        virtual ~ITimeIdBasedExchangeItem(){}
    };

    /*!
     * \brief The ITimeSeriesIdBasedArgument class
     */
    class ITimeIdBasedArgument : public virtual ITimeArgument,
        public virtual ITimeIdBasedComponentDataItem
    {
      public:
        virtual ~ITimeIdBasedArgument(){}
    };
  }
}

Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITime, "HydroCouple::Temporal::ITime/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeSpan, "HydroCouple::Temporal::ITimeSpan/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeComponentDataItem, "HydroCouple::Temporal::ITimeComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeArgument, "HydroCouple::Temporal::ITimeArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeExchangeItem, "HydroCouple::Temporal::ITimeExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeIdBasedComponentDataItem, "HydroCouple::Temporal::ITimeIdBasedComponentDataItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeIdBasedExchangeItem, "HydroCouple::Temporal::ITimeIdBasedExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Temporal::ITimeIdBasedArgument, "HydroCouple::Temporal::ITimeIdBasedArgument/1.0")


Q_DECLARE_METATYPE(HydroCouple::Temporal::ITime*)
Q_DECLARE_METATYPE(QList<HydroCouple::Temporal::ITime*>)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeSpan*)
Q_DECLARE_METATYPE(QList<HydroCouple::Temporal::ITimeSpan*>)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeArgument*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeIdBasedComponentDataItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeIdBasedExchangeItem*)
Q_DECLARE_METATYPE(HydroCouple::Temporal::ITimeIdBasedArgument*)

#endif // HYDROCOUPLETEMPORAL_H

