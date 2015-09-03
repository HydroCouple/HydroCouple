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

#ifndef HYDROCOUPLE_GLOBAL_H
#define HYDROCOUPLE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef HYDROCOUPLE_LIB
# define HYDROCOUPLE_EXPORT Q_DECL_EXPORT
#else
# define HYDROCOUPLE_EXPORT Q_DECL_IMPORT
#endif

//! A C++ Model Coupling Interface Specification Based on OpenMI
namespace HydroCouple
{

    enum ComponentStatus
    {
        Created, /*!< The IModelComponent instance has just been <see cref="Created"/>. This status must and will be followed by <see cref="Initializing"/> */
        Initializing, /*!<The IModelComponent is initializing itself. This status will end in a status change to <see cref="Initialized"/> or <see cref="Failed"/> */
        Initialized, /*!<The IModelComponent has succesfully initialized itself. The connections between its inputs/outputsand those of other components can be established*/
        Validating,	/*!< After links between the IModelComponent's inputs/outputs and those of other components have been established, the IModelComponent is validating whether its required input will be available when it updates itself, and whether indeed it will be able to provide the required output during this update. This <see cref= "Validating"/> status will end in a status change to <see cref="Valid"/> or <see cref="Invalid"/> */
        Valid, /*!<The IModelComponent is in a valid state. When updating itself its required input will be available, and it will be able to provide the required output.*/
        WaitingForData,	/*!<The IModelComponent wants to update itself, but is not yet able to perform the actual computation, because it is still waiting for input data from other components */
        Invalid, /*!<The IModelComponent is in an invalid state. When updating itself not all required input will be available, and/or it will not be able to provide the required output. After the user has modified the connections between the IModelComponent's inputs/outputs and those of other components, the <see cref="Validating"/> state can be entered again */
        Preparing, /*!<The IModelComponent is preparing itself for the first <see cref = "getValues()"/> call.This <see cref="Preparing"/> state will end in a status change to <see cref="Updated"/> or <see cref="Failed"/> */
        Updating, /*!<The IModelComponent is updating itself. It has received all required input data from other components, and is now performing the actual computation. This <code>Updating</code> state will end in a status change to <see cref="Updated"/>, <see cref="Done"/> or <see cref="Failed"/> */
        Updated, /*!<The IModelComponent has succesfully updated itself. */
        Done, /*!<The last update process that the IModelComponent performed was the final one. A next call to the Update method will leave the IModelComponent's internal state unchanged */
        Finishing,	/*!<The IModelComponent was requested to perform the actions to be performed before it will either be disposed or re-intialized again.Typical actions would be writing the final result files, close all open files, free memory, etc. When all required actions have been performed, the status switches to <see cref="Created"/> when re-initialization is possible. The status switches to <see cref="Finished"/> when the IModelComponent is to be disposed */
        Finished, /*!<The IModelComponent has successfully performed its finalization actions. Re-initialization of the IModelComponent instance is not possible and should not be attempted. Instead the instance should be disposed, e.g. through the garbage collection mechanism */
        Failed,	/*!<The IModelComponent was requested to perform the actions to be perform before it will either be disposed or re-initialized again. Typical actions would be writing the final result files, close all open files, free memory, etc. When all required actions have been performed, the status switches back to <see cref="Created"/> if the IModelComponent supports being re-initialized. If it cannot be re-initialized, it can be released from memory */
    };
}

#endif // HYDROCOUPLE_GLOBAL_H

