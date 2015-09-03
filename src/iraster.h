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

#ifndef IRASTER_H
#define IRASTER_H

#include <irasterband.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!Raster spatial feature
			class IRaster : public virtual IIdentity
			{
				//Q_PROPERTY(int XSize READ xSize)
				//Q_PROPERTY(int YSize READ ySize)
				//Q_PROPERTY(int RasterBandCount READ rasterBandCount)
				//Q_PROPERTY(IHSpatialReferenceSystem*  SpatialReferenceSystem READ spatialReferenceSystem)

			public:
				virtual ~IRaster(){}

				//!Number of pixels in the x direction
				virtual int xSize() const = 0;

				//!Number of pixels in y direction
				virtual int ySize() const = 0;

				//!Number of raster bands
				virtual int rasterBandCount() const = 0;

				//!Adds a new IHRasterBand
				virtual void addRasterBand(RasterDataType dataType) = 0;

				//!IHSpatialReferenceSystem represents the spatial reference system of goemetric object
				virtual ISpatialReferenceSystem* spatialReferenceSystem() const = 0;

				//!Fetches the affine transformation coefficients. It is an array of size 6.
				/*!
				Fetches the coefficients for transforming between pixel/line (P,L) raster space, and projection coordinates (Xp,Yp) space.
				Xp = transformationMatrix[0] + P*transformationMatrix[1] + L*transformationMatrix[2];
				Yp = transformationMatrix[3] + P*transformationMatrix[4] + L*transformationMatrix[5];
				In a north up image, transformationMatrix[1] is the pixel width, and transformationMatrix[5] is the pixel height. The upper left corner of the upper left pixel is at position (transformationMatrix[0],transformationMatrix[3]).
				*/
				virtual void getTransformationMatrix(double* transformationMatrix) = 0;

				//!Gets the IHRasterBand for the band with index bandIndex
				virtual IRasterBand* getRasterBand(int bandIndex) const = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IRaster, "HydroCouple::Data::Spatial::IRaster/1.0")

#endif // IRASTER_H
