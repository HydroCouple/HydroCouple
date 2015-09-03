#ifndef IDIMENSION_H
#define IDIMENSION_H

#include <idescription.h>

namespace HydroCouple
{
	namespace Data
	{
		enum DimensionType
		{
			Constant,
			Unlimited
		};

		class IDimension : public virtual IDescription
		{

		public:
			virtual ~IDimension(){}

			//! Gets length of dimension
			/*!
			*/
			virtual int length() const = 0;

			//! Gets the dimension length type
			/*!
			*/
			virtual DimensionType lengthType() const = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IDimension, "HydroCouple::Data::IDimension/1.0")
#endif // IDIMENSION_H
