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
#ifndef ICOMPONENTINFO_H
#define ICOMPONENTINFO_H

#include "iidentity.h"

namespace HydroCouple
{
		//!IComponentInfo interface class provides information about a component either a IModelComponent or a IAdaptedOutputFactoryComponent 
		/*!
		   IComponentInfo used to provide extended metadata on a component
		*/
		class IComponentInfo : public virtual IIdentity
		{ 

		public:
			//!Name of Component
			/*!
			*/
			virtual QString name() const = 0;

			//!File path to Component library
			/*!
			*/
			virtual QString libraryFilePath() const = 0;

			//!Set file path to Component library
			/*!
			*/
			virtual void setLibraryFilePath(const QString& filePath) = 0;

			//!File path to Component icon. Must be specified relative to the component library.
			/*!
			*/
			virtual QString iconFilePath() const = 0;

			//!Component developer info
			/*!
			*/
			virtual QString vendor() const = 0;

			//!Component license info
			/*!
			*/
			virtual QString license() const = 0;

			//!Component copyright info
			/*!
			*/
			virtual QString copyright() const = 0;

			//!Component url
			/*!
			*/
			virtual QString url() const = 0;

			//!Component email
			/*!
			*/
			virtual QString email() const = 0;

			//!Component version info
			/*!
			*/
			virtual QString version() const = 0;

			//!Component category
			/*!
			Sub-categories can be specified by separating categories using the backslash eg. Hydrology\Snowmelt
			*/
			virtual QString category() const = 0;

			//!Checks if valid is license. Developer is responsible for implementing this validation based on a license.
			virtual bool hasValidLicense(QString& validationMessage) const = 0;
		};
}

Q_DECLARE_INTERFACE(HydroCouple::IComponentInfo, "HydroCouple::IComponentInfo/1.0")

#endif // ICOMPONENTINFO_H
