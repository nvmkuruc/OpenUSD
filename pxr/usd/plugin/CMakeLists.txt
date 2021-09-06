if (${PXR_BUILD_ALEMBIC_PLUGIN})
    add_subdirectory(usdAbc)
endif()
if (${PXR_BUILD_DRACO_PLUGIN})
    add_subdirectory(usdDraco)
endif()
if (${PXR_ENABLE_MATERIALX_SUPPORT})
    add_subdirectory(usdMtlx)
endif()
add_subdirectory(sdrOsl)

# TODO: Get usdMdl working with minimal builds (no-python, no-imaging, monolithic)
# XXX:aluk
# TODO: Get usdMdl compiling with USD 21.08
#if (${PXR_BUILD_IMAGING})
#    add_subdirectory(usdMdl)
#endif()