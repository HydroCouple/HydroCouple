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

#ifndef IHMULTICURVE_H
#define IHMULTICURVE_H
     
#include <igeometrycollection.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!An IHMultiCurve is a 1-dimensional IHGeometryCollection whose elements are Curves.
			/*!
			<para>
			IHMultiCurve is a non-instantiable class in this standard; it defines a set of methods for its subclasses and is
			included for reasons of extensibility.
			</para>

			<para>
			An IHMultiCurve is simple if and only if all of its elements are simple and the only intersections between any two
			elements occur at Points that are on the boundaries of both elements.
			</para>

			<para>
			The boundary of a IHMultiCurve is obtained by applying the “mod 2” union rule: A Point is in the boundary of a
			IHMultiCurve if it is in the boundaries of an odd number of elements of the IHMultiCurve
			</para>

			<para>
			A MultiCurve is closed if all of its elements are closed. The boundary of a closed MultiCurve is always empty.
			</para>

			<para>
			A MultiCurve is defined as topologically closed.
			</para>

			*/
			class IMultiCurve : public virtual IGeometryCollection
			{
				Q_PROPERTY(bool IsClosed READ isClosed)
				Q_PROPERTY(bool Length READ length)

			public:
				virtual ~IMultiCurve(){}

				//!Returns 1 (TRUE) if this MultiCurve is closed [startPoint ( ) = endPoint ( ) for each IHCurve in this IHMultiCurve].
				virtual bool isClosed() const = 0;

				//!The Length of this IHMultiCurve which is equal to the sum of the lengths of the element IHCurves.
				virtual double length() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IMultiCurve, "HydroCouple::Data::Spatial::IMultiCurve/1.0")

#endif // IHMULTICURVE_H
