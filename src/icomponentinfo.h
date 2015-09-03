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

#include <iidentity.h>

namespace HydroCouple
{
		//!IComponentInfo interface class provides information about a component either a IHModelComponent or a IHAdaptedProducerItemFactoryComponent 
		/*!
		   IComponentInfo used to provide extended metadata on a component
		*/
		class IComponentInfo : public virtual IIdentity
		{ 
			//!Properties must be implemented in derived classes
			//Q_PROPERTY(QString Name READ name)
			//Q_PROPERTY(QString FilePath READ filePath SET setFilePath)
			//Q_PROPERTY(QString IconfilePath READ iconfilePath)
			//Q_PROPERTY(QString Vendor READ vendor)
			//Q_PROPERTY(QString License READ license)
			//Q_PROPERTY(QString Copyright READ copyright)
			//Q_PROPERTY(QString Url READ url)
			//Q_PROPERTY(QString Email READ email)
			//Q_PROPERTY(QString Version READ version)
			//Q_PROPERTY(QString Category READ category)

		public:
			virtual ~IComponentInfo(){}

			//!Name of IHModelComponent
			/*!
			*/
			virtual QString name() const = 0;

			//!File path to IHModelComponent 
			/*!
			*/
			virtual QString libraryFilePath() const = 0;

			//!File path to IHModelComponent 
			/*!
			*/
			virtual void setLibraryFilePath(const QString& filePath) = 0;

			//!File path to IHModelComponent icon. Must be specified relative to the component library.
			/*!
			*/
			virtual QString iconFilePath() const = 0;

			//!IHModelComponent developer info
			/*!
			*/
			virtual QString vendor() const = 0;

			//!IHModelComponent license info
			/*!
			*/
			virtual QString license() const = 0;

			//!IHModelComponent copyright info
			/*!
			*/
			virtual QString copyright() const = 0;

			//!IHModelComponent url
			/*!
			*/
			virtual QString url() const = 0;

			//!IHModelComponent url
			/*!
			*/
			virtual QString email() const = 0;

			//!IHModelComponent version info
			/*!
			*/
			virtual QString version() const = 0;

			//!Item category
			/*!
			Sub-categories can be specified by separating categories using the backslash eg. Hydrology\Snowmelt
			*/
			virtual QString category() const = 0;

			//!Checks if valid is license
			virtual bool validateLicense(QString& validationMessage) const = 0;
		};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IComponentInfo, "HydroCouple::IComponentInfo/1.0")

#endif // ICOMPONENTINFO_H
