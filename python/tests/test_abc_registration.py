"""
Tests that Cython core wrapper types are registered as ABC subclasses.

Verifies that each CppXxxWrapper in _hydrocouple._core is a proper
subclass of the corresponding HydroCouple ABC interface.
"""

from hydrocouple.core import (
    IDimension,
    IValueDefinition,
    IComponentDataItem,
    IArgument,
    IInput,
    IOutput,
    IModelComponentInfo,
    IModelComponent,
    IWorkflowComponent,
)


class TestABCRegistration:
    def test_wrapper_subclass_of_abc(self):
        from _hydrocouple._core import (
            CppDimensionWrapper,
            CppValueDefinitionWrapper,
            CppComponentDataItemWrapper,
            CppArgumentWrapper,
            CppInputWrapper,
            CppOutputWrapper,
            CppModelComponentInfoWrapper,
            CppModelComponentWrapper,
            CppWorkflowComponentWrapper,
        )

        assert issubclass(CppDimensionWrapper, IDimension)
        assert issubclass(CppValueDefinitionWrapper, IValueDefinition)
        assert issubclass(CppComponentDataItemWrapper, IComponentDataItem)
        assert issubclass(CppArgumentWrapper, IArgument)
        assert issubclass(CppInputWrapper, IInput)
        assert issubclass(CppOutputWrapper, IOutput)
        assert issubclass(CppModelComponentInfoWrapper, IModelComponentInfo)
        assert issubclass(CppModelComponentWrapper, IModelComponent)
        assert issubclass(CppWorkflowComponentWrapper, IWorkflowComponent)
