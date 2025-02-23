/*!
 * \file   hydrocoupletemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version   2.0.0
 * \description
 * This header file contains the temporal interface definitions for the
 * HydroCouple component-based modeling definitions.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * MIT License as published by the Free Software Foundation.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
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
      virtual ~IDateTime() = default;

      /*!
       * \brief Date and time as a julian day value.
       */
      virtual double julianDay() const = 0;

      /*!
       * \brief Modified Julian day value.
       */
      virtual double modifiedJulianDay() const = 0;

      /*!
       * \brief Serial date number.
       */
      virtual double serialDate() const = 0;
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
      virtual double duration() const = 0;
    };

    /*!
     * \brief The ITimeModelComponent class for time marching models.
     */
    class ITimeModelComponent : public virtual HydroCouple::IModelComponent
    {
    public:
      /*!
       * \brief ~ITimeModelComponent
       */
      virtual ~ITimeModelComponent() = default;

      /*!
       * \brief currentDateTime
       * \return IDateTime pointer. The current date and time.
       */
      virtual IDateTime *currentDateTime() const = 0;

      /*!
       * \brief simulationPeriod of the model.
       * \return ITimeSpan pointer. The time horizon of the model.
       */
      virtual ITimeSpan *simulationPeriod() const = 0;
    };

    /*!
     * \brief ITimeComponentItem is an IComponentItem with a temporal attribute.
     * This class cannot be directly instantiated and must be implemented as
     * an abstract class that can be inherited by its specializations e.g.,
     * ITimeSeriesComponentDataItem, ITimeSeriesArgument, ITimeIdBasedComponentDataItem,
     * ITimeIdBasedExchangeItem, ITimeIdBasedArgument, or other geotemporal datasets
     */
    class ITimeSeriesComponentDataItem : public virtual IComponentDataItem
    {
    
    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief ~ITimeComponentItem destructor.
       */
      virtual ~ITimeSeriesComponentDataItem() = default;

      /*!
       * \brief time
       * \param timeIndex
       * \return
       */
      virtual const IDateTime *time(int timeIndex) const = 0;

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

      /*!
      * \brief Gets a single value for given time dimension index.
      * \param[out] data is a pointer to the location data is to be written.
      * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
      * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
      */
      virtual void getValue(
        hydrocouple_variant& data,
        int timeDimensionIndex,
        const initializer_list<int>& dimensionIndexes = {}) const = 0;

      /*!
      * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
      * \param[out] data is a pointer to the location data is to be written.
      * \param[in] timeDimensionIndex is the time dimension index from where to obtain the requested data.
      * \param[in] dimensionIndexes indexes to use for the other dimensions to get the data if they exist. Otherwise an empty vector.
      * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
      * \param[in] dimensionLengths are the lengths of the dimensions for the data to be set. If empty a single value is set,
      * otherwise the length of the vector must be equal to the number of dimensions.
      */
      virtual void getValues(
        hydrocouple_variant *data,
        int timeDimensionIndex,
        const initializer_list<int> &dimensionIndexes = {},
        int timeDimensionIndexLength = 1,
        const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
      * \brief Sets a single value for given time dimension index.
      * \param[in] data is the pointer to the input data to be set.
      * \param[in] timeDimensionIndex is the time dimension index from where to write data.
      * \param[in] dimensionIndexes indexes to use for the other dimensions to get the data if they exist. Otherwise an empty vector.
      */
      virtual void setValue(
        const hydrocouple_variant &data,
        int timeDimensionIndex,
        const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
      * \brief Sets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
      * \param[in] data is the pointer to the input data to be set.
      * \param[in] timeDimensionIndex is the time dimension index from where to write data.
      * \param[in] dimensionIndexes indexes to use for the other dimensions to get the data if they exist. Otherwise an empty vector.
      * \param[in] timeDimensionIndexLength is the length of the time dimension for the data to be obtained.
      * \param[in] dimensionLengths are the lengths of the dimensions for the data to be set. If empty a single value is set,
      * otherwise the length of the vector must be equal to the number of dimensions.
      */
      virtual void setValues(
        const hydrocouple_variant *data,
        int timeDimensionIndex,
        const initializer_list<int> &dimensionIndexes = {},
        int timeDimensionIndexLength = 1,
        const initializer_list<int> &dimensionLengths = {}) = 0;

    };

    /*!
     * \brief The ITimeSeriesComponentDataItem class
     */
    class ITimeSeriesComponentDataItem : public virtual IComponentDataItem
    {


    public:

      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief ~ITimeSeriesComponentDataItem destructor.
       */
      virtual ~ITimeSeriesComponentDataItem() = default;

      /*!
       * \brief Gets a single value for given time dimension index.
       * \param[out] data is pointer to the location where data is to be written.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       */
      virtual void getValue(
        hydrocouple_variant &data,
        int timeIndex, 
        const vector<int> &dimensionIndexes = {}  
      ) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given time dimension index and size for a hyperslab.
       * \param[out] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * \param[in] data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       */
      virtual void getValues(
        hydrocouple_variant *data,
        int timeIndex, 
        const initializer_list<int> &dimensionIndexes = {},
        int timeIndexLength = 1, 
        const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief Sets a single value for given time dimension index.
       * \param[in] data is pointer to the data to be written.
       * \param[in] timeIndex is the time dimension index where data is to be written.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       */
      virtual void setValue(
        const hydrocouple_variant& data,
        int timeIndex,
        const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given
       * time dimension index and size for a hyperslab.
       * \param[in] data is the input multi dimensional array to be written.
       * \param[in] timeIndex is the time dimension index where data is to be written.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       */
      virtual void setValues(
        const hydrocouple_variant *data,
        int timeIndex, 
        const initializer_list<int> &dimensionIndexes = {},
        int timeIndexLength = 1, 
        const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The ITimeIdBasedComponentDataItem class
     */
    class ITimeIdBasedComponentDataItem : public virtual IComponentDataItem,
                                          public virtual ITimeSeriesComponentDataItem
    {



    public:
        
        using IComponentDataItem::getValue;
        using IComponentDataItem::getValues;
        using IComponentDataItem::setValue;
        using IComponentDataItem::setValues;
        using ITimeSeriesComponentDataItem::getValue;
        using ITimeSeriesComponentDataItem::getValues;
        using ITimeSeriesComponentDataItem::setValue;
        using ITimeSeriesComponentDataItem::setValues;

      /*!
       * \brief ~ITimeIdBasedComponentDataItem destructor.
       */
      virtual ~ITimeIdBasedComponentDataItem() = default;

      /*!
       * \brief identifiers associated with this dimension.
       * \return vector<string> of identifiers
       */
      virtual vector<string> identifiers() const = 0;

      /*!
       * \brief idDimensions associated with this dimension.
       * \return IDimension of the identifiers associated with this dimension.
       */
      virtual IDimension *identifierDimension() const = 0;

      /*!
       * \brief Gets a single value for given id dimension index.
       * \param[out] data is a pre-allocated pointer to the location data is to be written.
       * \param[in] idIndex is the id dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       */
      virtual void getValue(
        hydrocouple_variant &data,
        int timeIndex, 
        int idIndex, 
        const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given id dimension index and size for a hyperslab.
       * \param[out] data is pre-allocated pointer to the location data is to be written.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] idIndex is the id dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained.
       * \param[in] idIndexLength is the length of the id dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       */
      virtual void getValues(
        hydrocouple_variant *data,
        int timeIndex, 
        int idIndex, 
        const initializer_list<int> &dimensionIndexes = {},
        int timeIndexLength = 1,
        int idIndexLength = 1,
        const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief Sets a single value for given id dimension index.
       * \param[in] data is the pointer to a pre-allocated location where data to be written.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] idIndex is the id dimension index where data is to be written.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       */
      virtual void setValue(
        const hydrocouple_variant &data,
        int timeIndex, 
        int idIndex, 
        const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given
       * id dimension index and size for a hyperslab.
       * \param[in] data is the input multi dimensional array to be written.
       * \param[in] timeIndex is the time dimension index from where to obtain the requested data.
       * \param[in] idIndex is the id dimension index where data is to be written.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       * \param[in] timeIndexLength is the length of the time dimension for the data to be obtained. 
       * \param[in] idIndexLength is the length of the id dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       */
      virtual void setValues(
        const hydrocouple_variant *data,
        int timeIndex,
        int idIndex,
        const initializer_list<int> &dimensionIndexes = {},
        int timeIndexLength = 1,
        int idIndexLength = 1,
        const vector<int> &dimensionLengths = {}) = 0;
    };
  }
}

#endif // HYDROCOUPLETEMPORAL_H
