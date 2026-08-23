/*!
 * \file hydrocouplespatiotemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \brief Spatiotemporal interface definitions for the HydroCouple component-based modeling framework.
 * \details This header file contains the spatiotemporal interface definitions for the
 * HydroCouple component-based modeling framework. It defines component data items
 * whose values vary in both time and space by combining the temporal and spatial
 * data item interfaces.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
 */

#ifndef HYDROCOUPLESPATIOTEMPORAL_H
#define HYDROCOUPLESPATIOTEMPORAL_H

#include "hydrocoupletemporal.h"
#include "hydrocouplespatial.h"

namespace HydroCouple
{
  //! HydroCouple's interfaces that have both spatial and temporal components.
  namespace SpatioTemporal
  {
    /*!
     * \brief ITimeGeometryComponentDataItem is a geometry component data item whose
     * values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, the geometry
     * dimension is dimension 1 of shape(); any additional dimensions follow. Data
     * access uses the inherited getValuesInto()/setValuesFrom() hyperslab API, so
     * "current time step, all geometries" is a contiguous slab.
     */
    class ITimeGeometryComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                           public virtual HydroCouple::Spatial::IGeometryComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeGeometryComponentDataItem destructor.
       */
      virtual ~ITimeGeometryComponentDataItem() = default;
    };

    /*!
     * \brief ITimeNetworkComponentDataItem is a network component data item whose
     * values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, the entity dimension
     * selected by networkDataType() is dimension 1 of shape(); any additional
     * dimensions follow. Data access uses the inherited getValuesInto()/setValuesFrom()
     * hyperslab API.
     */
    class ITimeNetworkComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                          public virtual HydroCouple::Spatial::INetworkComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeNetworkComponentDataItem destructor.
       */
      virtual ~ITimeNetworkComponentDataItem() = default;
    };

    /*!
     * \brief ITimeSeriesPolyhedralSurfaceComponentDataItem is a polyhedral surface
     * component data item whose values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, the entity dimension
     * selected by meshDataType() is dimension 1 of shape(); any additional dimensions
     * follow. Data access uses the inherited getValuesInto()/setValuesFrom()
     * hyperslab API.
     */
    class ITimeSeriesPolyhedralSurfaceComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                                          public virtual HydroCouple::Spatial::IPolyhedralSurfaceComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeSeriesPolyhedralSurfaceComponentDataItem destructor.
       */
      virtual ~ITimeSeriesPolyhedralSurfaceComponentDataItem() = default;
    };

    /*!
     * \brief ITimeSeriesTINComponentDataItem is an
     * ITimeSeriesPolyhedralSurfaceComponentDataItem whose surface is an ITIN.
     */
    class ITimeSeriesTINComponentDataItem : public virtual ITimeSeriesPolyhedralSurfaceComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeSeriesTINComponentDataItem destructor.
       */
      virtual ~ITimeSeriesTINComponentDataItem() = default;

      /*!
       * \brief The ITIN this data item is associated with.
       */
      [[nodiscard]] virtual HydroCouple::Spatial::ITIN *TIN() const = 0;
    };

    /*!
     * \brief ITimeSeriesRasterComponentDataItem is a raster component data item whose
     * values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, band is dimension 1,
     * y (row) is dimension 2, and x (column) is dimension 3 of shape(). Data access
     * uses the inherited getValuesInto()/setValuesFrom() hyperslab API.
     */
    class ITimeSeriesRasterComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                               public virtual HydroCouple::Spatial::IRasterComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeSeriesRasterComponentDataItem destructor.
       */
      virtual ~ITimeSeriesRasterComponentDataItem() = default;
    };

    /*!
     * \brief ITimeRegularGrid2DComponentDataItem is a 2D regular grid component data
     * item whose values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, y-cell is
     * dimension 1, x-cell is dimension 2 of shape(); the optional cell edge and cell
     * vertex dimensions follow. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
     */
    class ITimeRegularGrid2DComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                                public virtual HydroCouple::Spatial::IRegularGrid2DComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeRegularGrid2DComponentDataItem destructor.
       */
      virtual ~ITimeRegularGrid2DComponentDataItem() = default;
    };

    /*!
     * \brief ITimeRegularGrid3DComponentDataItem is a 3D regular grid component data
     * item whose values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, z-cell is
     * dimension 1, y-cell is dimension 2, x-cell is dimension 3 of shape(); the
     * optional cell face and cell vertex dimensions follow. Data access uses the
     * inherited getValuesInto()/setValuesFrom() hyperslab API.
     */
    class ITimeRegularGrid3DComponentDataItem : public virtual HydroCouple::Temporal::ITimeSeriesComponentDataItem,
                                                public virtual HydroCouple::Spatial::IRegularGrid3DComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeRegularGrid3DComponentDataItem destructor.
       */
      virtual ~ITimeRegularGrid3DComponentDataItem() = default;
    };
  }
}

#endif // HYDROCOUPLESPATIOTEMPORAL_H
