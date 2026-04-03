set(DIRS
    usdrecord
    usdview
    testusdview
)

foreach(d ${DIRS})
    add_subdirectory(${d})
endforeach()

# nv-begin usdBakeMtlx is disabled due to X11 linkage in monolothic builds
if (NOT PXR_BUILD_MONOLITHIC) 

if (${PXR_ENABLE_MATERIALX_SUPPORT})
    add_subdirectory(usdBakeMtlx)
endif()

endif()
# nv-end
