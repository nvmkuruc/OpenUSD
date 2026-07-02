set(PXR_PREFIX pxr/usd)
set(PXR_PACKAGE usdDraco)

pxr_plugin(${PXR_PACKAGE}
    LIBRARIES
        tf
        gf
        sdf
        usd
        usdGeom
        ${DRACO_LIBRARY}

    INCLUDE_DIRS
        ${DRACO_INCLUDES}
    
    PRIVATE_HEADERS
        exportAttribute.h
        flag.h
        importAttribute.h
    
    CPPFILES
        attributeDescriptor.cpp
        attributeFactory.cpp
        exportTranslator.cpp
        fileFormat.cpp
        importTranslator.cpp
        writer.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapDraco.cpp

    PYMODULE_FILES
        __init__.py

    RESOURCE_FILES
        plugInfo.json

    DOXYGEN_FILES
        overview.dox
)
