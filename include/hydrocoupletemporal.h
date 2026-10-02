/*!
 * \file   hydrocoupletemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version   2.0.0-alpha.2
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

#include <cstdint>
#include "hydrocouple.h"


/*!
 * These interfaces cross shared-library boundaries by design — a component
 * in one image hands its data items to an SDK or host living in another —
 * and dynamic_cast across images only works when a class's type_info is one
 * entity. Projects routinely build with -fvisibility=hidden, which would
 * give every image its own private copy of these typeinfos and make such
 * casts fail silently; forcing default visibility here keeps the RTTI
 * shared regardless of the including project's flags.
 */
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility push(default)
#endif

namespace HydroCouple
{
  //! HydroCouple's interfaces that have a time varying component.
  namespace Temporal
  {
    /*!
     * \brief What a value's time coordinate refers to.
     *
     * A time-series item carries times and values but does not say whether a
     * value is a reading at that instant, a mean over the interval that ended
     * there, or an accumulation. Those are three different numbers, and coupling
     * a model that reports means to one that expects instants is wrong in a way
     * that produces plausible output and no error.
     *
     * It is not a hypothetical failure. A daily report in this ecosystem drifted
     * from noon to five in the afternoon over a simulated year, aliasing a
     * diurnal cycle into a seasonal signal, and it was found by checking a figure
     * caption against its own timestamps rather than by anything in the data
     * saying what the timestamps meant.
     */
    enum class TimeKind : uint8_t
    {
      Unknown = 0,      //!< Not declared. Consumers must not assume Instantaneous.
      Instantaneous,    //!< A reading at the time coordinate.
      IntervalMean,     //!< Mean over the interval ending at the time coordinate.
      IntervalMinimum,  //!< Minimum over that interval.
      IntervalMaximum,  //!< Maximum over that interval.
      Accumulated       //!< Total accumulated over that interval.
    };

    /*!
     * \brief How a provider produces a value at an instant that is between two it holds.
     */
    enum class TimeInterpolation : uint8_t
    {
      Unknown = 0, //!< Not declared. validate() must treat a connection whose two ends both say Unknown as an error.
      None,        //!< Exact match required; a query at an instant not held is refused.
      Previous,    //!< Hold the most recent earlier value (step function).
      Nearest,     //!< The nearer of the two neighbours.
      Linear       //!< Linear in time between neighbours.
    };

    /*!
     * \brief How a provider produces a value at an instant beyond the last (or before the first) it holds.
     */
    enum class TimeExtrapolation : uint8_t
    {
      Unknown = 0, //!< Not declared. validate() must treat a connection whose two ends both say Unknown as an error.
      Refuse,      //!< A query outside the held range is refused; the provider must be advanced first.
      HoldLast,    //!< Repeat the boundary value.
      Linear       //!< Extend the last two values linearly.
    };

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
       * \brief Date and time as a Julian day value (days since 4713 BC January 1, 12:00 UTC).
       */
      [[nodiscard]] virtual double julianDay() const = 0;

      /*!
       * \brief Modified Julian day value: julianDay() - 2400000.5 (days since 1858-11-17 00:00 UTC).
       */
      [[nodiscard]] virtual double modifiedJulianDay() const = 0;

      /*!
       * \brief Serial date number in the MATLAB `datenum` convention: days since
       * 0000-01-00 in the proleptic Gregorian calendar, i.e. julianDay() - 1721058.5.
       * (Excel's 1900 serial differs from this by 693960 days and is not what is meant.)
       */
      [[nodiscard]] virtual double serialDate() const = 0;
    };

    /*!
     * \brief ITimeSpan specifies an interval of time.
     * \details Inherits IDateTime deliberately: the inherited instant is the *start*
     * of the interval, so a span can be handed to anything that wants an instant and
     * it reads as "when this begins". The end is start + duration().
     */
    class ITimeSpan : public virtual IDateTime
    {

    public:
      /*!
       * \brief ~ITimeSpan destructor.
       */
      virtual ~ITimeSpan() = default;

      /*!
       * \brief Duration of the timespan in days; non-negative.
       * \return double value of the duration.
       */
      [[nodiscard]] virtual double duration() const = 0;

      /*!
       * \brief End of the interval as a Julian day: julianDay() + duration().
       */
      [[nodiscard]] virtual double endJulianDay() const = 0;
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

      /*!
       * \brief The time (Julian day) the next update() will advance currentDateTime() to.
       *
       * \details What a time-stepped orchestrator needs to build a schedule and what a
       * provider needs to answer "can I serve this instant yet". For a fixed-step model
       * it is current + the step; for an adaptive-step model it is the model's own
       * estimate and may be revised by the update that follows, but must never be
       * earlier than currentDateTime(). Equals currentDateTime() when the model is
       * Done or before prepare().
       */
      [[nodiscard]] virtual double nextDateTimeJulianDay() const = 0;
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
       * shape() and reports IDimension::DimensionRole::Time; any additional dimensions follow.
       * Data access uses the inherited getValuesInto()/setValuesFrom() hyperslab API
       * with the time index as start[0], so "current time step, all entities" is a
       * contiguous slab.
       * \return A pointer to the IDimension.
       */
      [[nodiscard]] virtual IDimension *timeDimension() const = 0;

      /*!
       * \brief What each value's time coordinate refers to.
       * \details Declared on the item rather than discovered through a side interface:
       * a time series without this is an array whose timestamps mean something the
       * consumer has to guess. Returning TimeKind::Unknown is legal; a consumer that
       * needs the distinction must then refuse rather than assume Instantaneous.
       */
      [[nodiscard]] virtual TimeKind timeKind() const = 0;

      /*!
       * \brief Length of the averaging or accumulation interval (days).
       * \details Zero for Instantaneous. Meaningless unless timeKind() names an
       * interval, and a consumer that needs it should treat zero as "not declared"
       * rather than as an instant.
       */
      [[nodiscard]] virtual double intervalLength() const = 0;

      /*!
       * \brief What this item does — or, for an input, accepts — between held instants.
       * \details On an output or adapted output: what the provider will do when a
       * consumer's times() fall between this item's times(). On an input: the least
       * the consumer will accept (a consumer declaring Linear accepts Linear or None;
       * one declaring None accepts only exact matches). IWorkflowComponent::validate()
       * compares the two ends of every temporal connection.
       */
      [[nodiscard]] virtual TimeInterpolation timeInterpolation() const = 0;

      /*!
       * \brief What this item does — or, for an input, accepts — outside its held range.
       * \details Same provider/consumer reading as timeInterpolation(). A provider that
       * answers Refuse must set its owning component to WaitingForData (or advance it)
       * rather than fabricate a value.
       */
      [[nodiscard]] virtual TimeExtrapolation timeExtrapolation() const = 0;
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
       * \details Canonical dimension ordering: time is dimension 0 (DimensionRole::Time), the
       * identifier dimension is dimension 1 (DimensionRole::Entity); any additional dimensions
       * follow. Data access uses the inherited getValuesInto()/setValuesFrom()
       * hyperslab API.
       * \return IDimension of the identifiers associated with this data item.
       */
      [[nodiscard]] virtual IDimension *identifierDimension() const = 0;
    };

  }
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility pop
#endif

#endif // HYDROCOUPLETEMPORAL_H
