/*!
 * \file hydrocouplespatiotemporal.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0-alpha.1
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
     * selected by location() is dimension 1 of shape(); a Component dimension follows
     * when networkDataType() is not Scalar. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
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
     * selected by location() is dimension 1 of shape(); a Component dimension follows
     * when meshDataType() is not Scalar. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
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
     * \brief A layered mesh item whose values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, the entity (face or
     * volume) dimension is dimension 1, the layer dimension is dimension 2 of
     * shape(); a Component dimension follows when meshDataType() is not Scalar. The
     * vertical coordinate's geometryEpoch() tells a consumer whether the elevations it
     * cached still describe the current time step. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
     */
    class ITimeLayeredMeshComponentDataItem : public virtual ITimeSeriesPolyhedralSurfaceComponentDataItem,
                                              public virtual HydroCouple::Spatial::ILayeredMeshComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeLayeredMeshComponentDataItem destructor.
       */
      virtual ~ITimeLayeredMeshComponentDataItem() = default;
    };

    /*!
     * \brief A layered network item whose values also vary in time.
     * \details Canonical dimension ordering: time is dimension 0, the entity (node or
     * edge) dimension is dimension 1, the layer dimension is dimension 2 of shape();
     * a Component dimension follows when networkDataType() is not Scalar. Data access
     * uses the inherited getValuesInto()/setValuesFrom() hyperslab API.
     */
    class ITimeLayeredNetworkComponentDataItem : public virtual ITimeNetworkComponentDataItem,
                                                 public virtual HydroCouple::Spatial::ILayeredNetworkComponentDataItem
    {
    public:
      /*!
       * \brief ~ITimeLayeredNetworkComponentDataItem destructor.
       */
      virtual ~ITimeLayeredNetworkComponentDataItem() = default;
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

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility pop
#endif

#endif // HYDROCOUPLESPATIOTEMPORAL_H
