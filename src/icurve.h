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

#ifndef ICURVE_H
#define ICURVE_H

#include <igeometry.h>
#include <ipoint.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A Curve is a 1-dimensional geometric object usually stored as a sequence of Points, with the subtype of Curve specifying the form of the interpolation between Points.
			/*!
			<para>
			This standard defines only one subclass of Curve, LineString, which uses linear interpolation between Points.
			</para>

			<para>
			A Curve is a 1-dimensional geometric object that is the homeomorphic image of a real, closed, interval:
			D = [a, b] = {t∈ℜ⏐ a ≤ t ≤ b}
			under a mapping
			f :[a, b] → ℜn
			where n is the coordinate dimension of the underlying Spatial Reference System.
			A Curve is simple if it does not pass through the same Point twice with the possible exception of the two end
			points (Reference [1], section 3.12.7.3):
			∀ c ∈ Curve, [a, b] = c.Domain, c =: f :[a, b] → ℜ n
			c.IsSimple ⇔ ∀ x1, x2 ∈ [a, b]: [ f(x1)=f(x2) ∧ x1<x2] ⇒ [x1=a ∧ x2=b]
			A Curve is closed if its start Point is equal to its end Point (Reference [1], section 3.12.7.3).
			c.IsClosed ⇔ [f(a) = f(b)]
			The boundary of a closed Curve is empty.
			c.IsClosed ⇔ [c.boundary = ∅]
			A Curve that is simple and closed is a Ring.
			The boundary of a non-closed Curve consists of its two end Points (Reference [1], section 3.12.3.2).
			A Curve is defined as topologically closed, that is, it contains its endpoints f(a) and f(b).
			</para>

			*/
			class ICurve : public virtual IGeometry
			{
				Q_PROPERTY(double Length READ length)
				Q_PROPERTY(IPoint* StartPoint READ startPoint)
				Q_PROPERTY(IPoint* EndPoint READ endPoint)
				Q_PROPERTY(bool IsClosed READ isClosed)
				Q_PROPERTY(bool IsRing READ isRing)

			public:
				virtual ~ICurve(){}

				//!The length of this IHCurve in its associated spatial reference.
				virtual double length() const = 0;

				//!The start Point of this IHCurve.
				virtual IPoint* startPoint() const = 0;

				//!The end Point of this IHCurve.
				virtual IPoint* endPoint() const = 0;

				//!Returns 1 (TRUE) if this IHCurve is closed[startPoint() = endPoint()].
				virtual bool isClosed() const = 0;

				//!Returns 1 (TRUE) if this Curve is closed [startPoint ( ) = endPoint ( )] and this Curve is simple(does not pass through the same Point more than once).
				virtual bool isRing() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(ICurve, "HydroCouple::Data::Spatial::ICurve/1.0")

#endif // ICURVE_H
