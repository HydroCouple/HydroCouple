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
#ifndef ISPATIAL_H
#define ISPATIAL_H

namespace HydroCouple
{
	namespace Data
	{
		//!Geospatial related entities
		namespace Spatial
		{
			enum GeometryType
			{
				Geometry = 0,
				Point = 1,
				LineString = 2,
				Polygon = 3,
				MultiPoint = 4,
				MultiLineString = 5,
				MultiPolygon = 6,
				GeometryCollection = 7,
				CircularString  = 8,
				CompoundCurve = 9,
				CurvePolygon = 10,
				MultiCurve = 11,
				MultiSurface = 12,
				Curve = 13,
				Surface = 14,
				PolyhedralSurface = 15,
				TIN =16,


				GeometryZ = 1000,
				PointZ = 1001,
				LineStringZ = 1002,
				PolygonZ = 1003,
				MultiPointZ = 1004,
				MultiLineStringZ = 1005,
				MultiPolygonZ = 1006,
				GeometryCollectionZ = 1007,
				CircularStringZ = 1008,
				CompoundCurveZ = 1009,
				CurvePolygonZ = 1010,
				MultiCurveZ = 1011,
				MultiSurfaceZ = 1012,
				CurveZ = 1013,
				SurfaceZ = 1014,
				PolyhedralSurfaceZ = 1015,
				TINZ = 1016,


				GeometryM = 2000,
				PointM = 2001,
				LineStringM = 2002,
				PolygonM = 2003,
				MultiPointM = 2004,
				MultiLineStringM = 2005,
				MultiPolygonM = 2006,
				GeometryCollectionM = 2007,
				CircularStringM = 2008,
				CompoundCurveM = 2009,
				CurvePolygonM = 2010,
				MultiCurveM = 2011,
				MultiSurfaceM = 2012,
				CurveM = 2013,
				SurfaceM = 2014,
				PolyhedralSurfaceM = 2015,
				TINM = 2016,


				GeometryZM = 3000,
				PointZM = 3001,
				LineStringZM = 3002,
				PolygonZM = 3003,
				MultiPointZM = 3004,
				MultiLineStringZM = 3005,
				MultiPolygonZM = 3006,
				GeometryCollectionZM = 3007,
				CircularStringZM = 3008,
				CompoundCurveZM = 3009,
				CurvePolygonZM = 3010,
				MultiCurveZM = 3011,
				MultiSurfaceZM = 3012,
				CurveZM = 3013,
				SurfaceZM = 3014,
				PolyhedralSurfaceZM = 3015,
				TINZM = 3016,
			};


			enum RasterDataType
			{
				//!Unknown or unspecified type
				Unknown,
				//!Eight bit unsigned integer
				Byte,
				//!Sixteen bit unsigned integer
				UInt16,
				//!Sixteen bit signed integer
				Int16,
				//!Thirty two bit unsigned integer
				UInt32,
				//!Thirty two bit signed integer
				Int32,
				//!Thirty two bit floating point
				Float32,
				//!Sixty four bit floating point
				Float64,
				//!Complex Int16
				CInt16,
				//!Complex Int32
				CInt32,
				//!Complex Float32
				CFloat32,
				//!Complex Float64
				CFloat64
			};

			Q_ENUMS(GeometryType);

			Q_ENUMS(RasterDataType);
		}
	}
}

#endif // ISPATIAL_H
