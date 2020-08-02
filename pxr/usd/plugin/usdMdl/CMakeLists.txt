set(PXR_PREFIX pxr/usd)
set(PXR_PACKAGE usdMdl)

pxr_plugin(usdMdl
    LIBRARIES
        arch
        gf
        ndr
        sdf
        sdr
        tf
        vt
        usd
        usdGeom
        usdShade
        usdUI
        usdUtils

    INCLUDE_DIRS
        ${MDL_SDK_INCLUDE_DIRS}

    PUBLIC_CLASSES
        usdToMdl
		usdUtil
		usdMdlUtil
		mdlAPI
		tokens

    CPPFILES
        discovery.cpp
        parser.cpp

    PRIVATE_CLASSES
        backdoor
        fileFormat
        neuray
        reader
        utils
        mdlToUsd
        distiller

    PRIVATE_HEADERS
        api.h
        neuray.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapBackdoor.cpp
		wrapMdlAPI.cpp
		wrapTokens.cpp

    PYMODULE_FILES
        __init__.py

    RESOURCE_FILES
        generatedSchema.usda
        plugInfo.json
        schema.usda:usdMdl/schema.usda
)

pxr_test_scripts(
    test/testUsdMdlDiscovery.py
    test/testUsdMdlDistiller.py
)

pxr_install_test_dir(
    SRC test/testUsdMdlDiscovery
    DEST testUsdMdlDiscovery
)

pxr_install_test_dir(
    SRC test/testUsdMdlDistiller
    DEST testUsdMdlDistiller
)

pxr_register_test(testUsdMdlDiscovery
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testUsdMdlDiscovery"
    EXPECTED_RETURN_CODE 0
)

pxr_register_test(testUsdMdlDistiller
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testUsdMdlDistiller"
    EXPECTED_RETURN_CODE 0
)
