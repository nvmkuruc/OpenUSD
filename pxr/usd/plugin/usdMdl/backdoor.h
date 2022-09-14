/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
 
#include "pxr/pxr.h"
#include "pxr/base/tf/declarePtrs.h"
#include "pxr/usd/plugin/usdMdl/api.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_WEAK_AND_REF_PTRS(UsdStage);

/// Test distilling/baking and build a test stage using distilled output and UsdPreviewSurface shader.
/// Input a material qualified name.
/// A new stage "preview.usda" and baked images are saved in current working directory.
USDMDL_API UsdStageRefPtr UsdMdl_TestDistill(const std::string& materialQualifiedName);

/// Set MDL SDK verbosity level.
/// verbosity should be in the range [0..6]. 0 suppress all messages, 6 enable all messages.
/// Here is the list of severity levels in increasing order: Fatal, Error, Warning, Info, Verbose, Debug.
USDMDL_API bool UsdMdl_SetVerbosity(int verbosity);

/// If the input stage is valid and the prim is a USD Material corresponding to an MDL,
/// convert the USD to MDL and create a corresponding MDL material instance.
USDMDL_API bool UsdMdl_CreateMdl(const std::string& stageName, const std::string& prim);

/// Enable/Disable UsdMdl Discovery plugin
USDMDL_API bool UsdMdl_EnableDiscoveryPlugin(bool enable);

PXR_NAMESPACE_CLOSE_SCOPE
