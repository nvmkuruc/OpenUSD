set(PXR_PREFIX pxr/usdImaging)
set(PXR_PACKAGE usdImaging)

pxr_library(usdImaging
    LIBRARIES
        gf
        tf
        plug
        trace
        vt
        work
        hd
        pxOsd
        sdf
        usd
        usdGeom
        usdSkel
        usdLux
        usdShade
        usdVol
        ar
        ${TBB_tbb_LIBRARY}

    INCLUDE_DIRS
        ${TBB_INCLUDE_DIRS}

    PUBLIC_CLASSES
        adapterRegistry
        collectionCache
        debugCodes
        delegate
        indexProxy
        resolvedAttributeCache
        instancerContext
        tokens
        valueCache
        basisCurvesAdapter
        capsuleAdapter
        cameraAdapter
        coneAdapter
        coordSysAdapter
        cubeAdapter
        cylinderAdapter
        cylinderLightAdapter
        diskLightAdapter
        distantLightAdapter
        domeLightAdapter
        fieldAdapter
        gprimAdapter
        hermiteCurvesAdapter
        implicitSurfaceMeshUtils
        instanceAdapter
        lightAdapter
        lightFilterAdapter
        materialAdapter
        meshAdapter
        nurbsCurvesAdapter
        nurbsPatchAdapter
        pointsAdapter
        pointInstancerAdapter
        primAdapter
        rectLightAdapter
        sphereAdapter
        sphereLightAdapter
        textureUtils
        volumeAdapter

    PRIVATE_CLASSES
        materialParamUtils

    PUBLIC_HEADERS
        api.h
        unitTestHelper.h
        version.h

    RESOURCE_FILES
        plugInfo.json
)

# #nv begin #fast-updates
pxr_build_test(testUsdImagingFastUpdates
    LIBRARIES
        trace
        sdf
        hd
        usd
        usdImaging
        ${GLEW_LIBRARY}
    CPPFILES
        testenv/testUsdImagingFastUpdates.cpp
)

pxr_build_test(testUsdImagingPopulate_Perf
    LIBRARIES
        trace
        hd
        usd
        usdImaging
        usdUtils
        ${GLEW_LIBRARY}
    CPPFILES
        testenv/testUsdImagingPopulate_Perf.cpp
)

pxr_register_test(testUsdImagingFastUpdates
    COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testUsdImagingFastUpdates"
)

pxr_register_test(testUsdImagingPopulate_Perf
    COMMAND "${CMAKE_INSTALL_PREFIX}/tests/testUsdImagingPopulate_Perf"
)
# nv end
