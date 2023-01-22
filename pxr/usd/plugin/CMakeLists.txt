if (${PXR_BUILD_ALEMBIC_PLUGIN})
    add_subdirectory(usdAbc)
endif()
if (${PXR_BUILD_DRACO_PLUGIN})
    add_subdirectory(usdDraco)
endif()
add_subdirectory(sdrOsl)

#nv begin #usdmdl
# TODO: Get usdMdl working with minimal builds (no-python, no-imaging, monolithic)
#if (${PXR_BUILD_IMAGING} AND ${NV_BUILD_USDMDL})
#    add_subdirectory(usdMdl)
#endif()
#nv end