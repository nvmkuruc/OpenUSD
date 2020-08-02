/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"

#include "pxr/usd/plugin/usdMdl/backdoor.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/base/tf/makePyConstructor.h"

#include <boost/python/def.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

void wrapUsdMdlBackdoor()
{
    def("TestDistill", UsdMdl_TestDistill,
        (arg("materialQualifiedName")),
        return_value_policy<TfPyRefPtrFactory<>>());

    def("SetVerbosity", UsdMdl_SetVerbosity,
        (arg("verbosity")),
        return_value_policy<return_by_value>());

    def("CreateMdl", UsdMdl_CreateMdl,
        (arg("stageName"), arg("prim")),
        return_value_policy<return_by_value>());

    def("EnableDiscoveryPlugin", UsdMdl_EnableDiscoveryPlugin,
        (arg("enable")),
        return_value_policy<return_by_value>());
}
