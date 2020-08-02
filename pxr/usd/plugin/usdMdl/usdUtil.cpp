/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/usdUtil.h"
#include "pxr/usd/plugin/usdMdl/usdToMdl.h"
#include "pxr/usd/plugin/usdMdl/neuray.h"

PXR_NAMESPACE_OPEN_SCOPE

USDMDL_API int UsdShadeMaterialToMdl(const UsdShadeMaterial & usdMaterial, const char * mdlMaterialInstanceName)
{
    return ConvertUsdMaterialToMdl(usdMaterial, mi::neuraylib::Mdl::Get().GetNeuray(), mdlMaterialInstanceName);
}

PXR_NAMESPACE_CLOSE_SCOPE
