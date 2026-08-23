/*!
 * \file   hydrocoupletemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version   2.0.0-alpha.1
 * \brief Temporal interface definitions for the HydroCouple component-based modeling framework.
 * \details This header file contains the temporal interface definitions for the
 * HydroCouple component-based modeling framework. It defines interfaces for
 * date/time representation, time spans, time-marching model components,
 * and time-series component data items.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
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
     * \brief IDateTime interface based on a Julian day
     * \details The normative convention is the astronomical Julian day number in the
     * proleptic Gregorian ("standard") calendar, UTC. Persistence layers writing CF
     * metadata should therefore emit units of "days since ..." with
     * calendar = "standard".
     */
    class IDateTime : public virtual HydroCouple::IPropertyChanged
    {
    public:
      /*!
       * \brief ~IDateTime destructor.
       */
      virtual ~IDateTime() = default;

      /*!
       * \brief Date and time as a julian day value.
       */
      [[nodiscard]] virtual double julianDay() const = 0;

      /*!
       * \brief Modified Julian day value.
       */
      [[nodiscard]] virtual double modifiedJulianDay() const = 0;

      /*!
       * \brief Serial date number.
       */
      [[nodiscard]] virtual double serialDate() const = 0;
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
      virtual ~ITimeSpan() = default;

      /*!
       * \brief Duration of the timespan in days.
       * \return double value of the duration.
       */
      [[nodiscard]] virtual double duration() const = 0;
    };

    /*!
     * \brief ITimeModelComponent is an IModelComponent that advances through
     * time during simulation and provides access to the current simulation time.
     */
    class ITimeModelComponent : public virtual HydroCouple::IModelComponent
    {
    public:
      /*!
       * \brief ITimeModelComponent destructor.
       */
      virtual ~ITimeModelComponent() = default;

      /*!
       * \brief Gets the current date and time of the model simulation.
       * \return IDateTime pointer representing the current date and time.
       */
      [[nodiscard]] virtual IDateTime *currentDateTime() const = 0;

      /*!
       * \brief simulationPeriod of the model.
       * \return ITimeSpan pointer. The time horizon of the model.
       */
      [[nodiscard]] virtual ITimeSpan *simulationPeriod() const = 0;
    };

    /*!
     * \brief ITimeSeriesComponentDataItem is an IComponentItem with a temporal attribute.
     *
     * \details This class cannot be directly instantiated and must be implemented as
     * an abstract class that can be inherited by its specializations e.g.,
     * ITimeSeriesArgument, ITimeIdBasedComponentDataItem,
     * ITimeIdBasedExchangeItem, ITimeIdBasedArgument, or other geotemporal datasets.
     */
    class ITimeSeriesComponentDataItem : public virtual IComponentDataItem
    {

    public:
      /*!
       * \brief ~ITimeSeriesComponentDataItem destructor.
       */
      virtual ~ITimeSeriesComponentDataItem() = default;

      /*!
       * \brief Gets the IDateTime for the given time index.
       * \param[in] timeIndex is the index of the time to retrieve.
       * \return A pointer to the IDateTime at the specified index.
       */
      [[nodiscard]] virtual const IDateTime *time(int64_t timeIndex) const = 0;

      /*!
       * \brief Gets the number of times.
       * \return The number of times.
       */
      [[nodiscard]] virtual int64_t timeCount() const = 0;

      /*!
       * \brief Bulk access to all time coordinates as Julian day values.
       * \details The span has timeCount() elements, is ordered with the time dimension,
       * and remains valid until the time dimension changes. This is the accessor IO
       * writers and interpolating adapters must use; time(int64_t) is a per-element
       * convenience for spot queries.
       * \return A span of Julian day values.
       */
      [[nodiscard]] virtual std::span<const double> times() const = 0;

      /*!
       * \brief Gets the ITimeSpan associated with this data item.
       * \return A pointer to the ITimeSpan.
       */
      [[nodiscard]] virtual ITimeSpan *timeSpan() const = 0;

      /*!
       * \brief Gets the IDimension of the times.
       * \details Canonical dimension ordering: the time dimension is dimension 0 of
       * shape(); any additional dimensions follow. Data access uses the inherited
       * getValuesInto()/setValuesFrom() hyperslab API with the time index as start[0],
       * so "current time step, all entities" is a contiguous slab.
       * \return A pointer to the IDimension.
       */
      [[nodiscard]] virtual IDimension *timeDimension() const = 0;
    };

    /*!
     * \brief ITimeIdBasedComponentDataItem is an IComponentDataItem with both
     * temporal and identifier-based dimensions for accessing multi-dimensional time-series data.
     */
    class ITimeIdBasedComponentDataItem : public virtual ITimeSeriesComponentDataItem
    {

    public:
      /*!
       * \brief ~ITimeIdBasedComponentDataItem destructor.
       */
      virtual ~ITimeIdBasedComponentDataItem() = default;

      /*!
       * \brief identifiers associated with the identifier dimension.
       * \return vector<string> of identifiers
       */
      [[nodiscard]] virtual std::vector<std::string> identifiers() const = 0;

      /*!
       * \brief identifierDimension associated with this data item.
       * \details Canonical dimension ordering: time is dimension 0, the identifier
       * dimension is dimension 1; any additional dimensions follow. Data access uses
       * the inherited getValuesInto()/setValuesFrom() hyperslab API.
       * \return IDimension of the identifiers associated with this data item.
       */
      [[nodiscard]] virtual IDimension *identifierDimension() const = 0;
    };
  }
}

#endif // HYDROCOUPLETEMPORAL_H
