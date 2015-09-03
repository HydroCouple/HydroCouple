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
#ifndef ISPATIALREFERENCESYSTEM_H
#define ISPATIALREFERENCESYSTEM_H

#include "iidentity.h"

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!Spatial Reference System
			class ISpatialReferenceSystem : public virtual IIdentity
			{
				//Q_PROPERTY(int SRID READ authSRID)
				//Q_PROPERTY(QString AuthorityName READ authName)
				//Q_PROPERTY(QString WellknownText READ srText)

			public:
				virtual ~ISpatialReferenceSystem(){}
				
				//!Returns the Spatial Reference System ID for a geometric object. This will normally be a foreign key to an index of reference systems stored in either the same or some other datastore
				virtual int authSRID() const = 0;

				//!The Authority Specific Spatial Reference System Identifier
				virtual QString authName() const = 0;

				//!Well-known Text description of the Spatial Reference System
				virtual QString srText() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(ISpatialReferenceSystem, "HydroCouple::Data::Spatial::IHSpatialReferenceSystem/1.0")

#endif // ISPATIALREFERENCESYSTEM_H
