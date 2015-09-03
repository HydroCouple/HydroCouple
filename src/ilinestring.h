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

#ifndef IHLINESTRING_H
#define IHLINESTRING_H

#include <icurve.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!An IHLineString is a Curve with linear interpolation between Points. Each consecutive pair of Points defines a Line	segment.
			/*!
			*/
			class ILineString : public virtual ICurve
			{
				Q_PROPERTY(int PointCount READ pointCount)

			public:
				virtual ~ILineString(){}

				//!The number of Points in this IHLineString.
				virtual int pointCount() const = 0;

				//!Returns the specified Point at index in this IHLineString.
				virtual IPoint* point(int index) const = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(ILineString, "HydroCouple::Data::Spatial::ILineString/1.0")

#endif // IHLINESTRING_H
