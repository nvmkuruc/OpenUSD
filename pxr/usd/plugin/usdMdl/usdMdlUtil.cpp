/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/usd/plugin/usdMdl/usdMdlUtil.h"

PXR_NAMESPACE_OPEN_SCOPE

USDMDL_API bool SetMdlSdk(mi::neuraylib::INeuray * neuray)
{
    return mi::neuraylib::Mdl::Get().SetNeuray(neuray);
}

PXR_NAMESPACE_CLOSE_SCOPE
