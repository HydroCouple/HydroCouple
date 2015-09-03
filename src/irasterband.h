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

#ifndef IRASTERBAND_H
#define IRASTERBAND_H

#include <ispatial.h>
#include <ispatialreferencesystem.h>


namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			class IHRaster;

			//! IHRasterBand is a single raster band within an IHRaster
			class IRasterBand : public virtual IIdentity
			{
				//Q_PROPERTY(int XSize READ xSize)
				//Q_PROPERTY(int YSize READ ySize)
				//Q_PROPERTY(IHRaster* Raster READ raster)
				//Q_PROPERTY(RasterDataType DataType READ dataType)
				//Q_PROPERTY(double NoDataValue READ getNoDataValue)

			public:
				virtual ~IRasterBand(){}

				//!Number of pixels in the x direction
				virtual int xSize() const = 0;

				//!Number of pixels in y direction
				virtual int ySize() const = 0;

				//!Parent IHRaster of this IHRasterBand
				virtual IHRaster* raster() const = 0;

				//!Raster data type
				virtual RasterDataType dataType() const = 0;

				//!Reads data into the image block
				/*!
				<param name="xOffset">The pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
				<param name="yOffset">The line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
				<param name="xSize">The width of the region of the band to be accessed in pixels.</param>
				<param name="ySize">The height of the region of the band to be accessed in lines.</param>
				<param name="image">Pointer to where data is to be written.</param>
				*/
				virtual void read(int xOffset, int yOffset, int xSize, int ySize, void* const image) const = 0;

				//!Writes image into the raster band
				/*!
				<param name="xOffset">The pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
				<param name="yOffset">The line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
				<param name="xSize">The width of the region of the band to be accessed in pixels.</param>
				<param name="ySize">The height of the region of the band to be accessed in lines.</param>
				<param name="image">Pointer to the data to be written.</param>
				*/
				virtual void write(int xOffset, int yOffset, int xSize, int ySize, void* const image) = 0;

				//!The nodata value for this band.
				virtual double getNoDataValue() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IRasterBand, "HydroCouple::Data::Spatial::IRasterBand/1.0")

#endif // IRASTERBAND_H
