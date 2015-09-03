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
#ifndef IHPOINT_H
#define IHPOINT_H

#include <igeometry.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A IHPoint is a 0-dimensional geometric object and represents a single location in coordinate space.
			/**
			<para>
			A IHPoint has an x-coordinate value, a y-coordinate value. If called for by the associated Spatial Reference System, it may also
			have coordinate values for z and m.
			</para>

			<para>
			The boundary of a Point is the empty set.
			</para>

			*/
			class IPoint : public virtual IGeometry
			{
				Q_PROPERTY(double X READ x)
				Q_PROPERTY(double Y READ y)
				Q_PROPERTY(double Z READ z)
				Q_PROPERTY(double M READ m)

			public:
				virtual ~IPoint(){}

				//!The x-coordinate value for this IHPoint.
				virtual double x() const = 0;

				//!The y-coordinate value for this IHPoint.
				virtual double y() const = 0;

				//!The z-coordinate value for this IHPoint, if it has one. Returns NIL otherwise.
				virtual double z() const = 0;

				//!The m-coordinate value for this IHPoint, if it has one. Returns NIL otherwise.
				virtual double m() const = 0;


			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IPoint, "HydroCouple::Data::Spatial::IPoint/1.0")

#endif // IHPOINT_H
