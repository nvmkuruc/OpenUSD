/******************************************************************************
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/
#pragma once
#include "pxr/pxr.h"
#include "pxr/usd/plugin/usdMdl/api.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/base/tf/declarePtrs.h"
#include <string>

namespace mi
{
    namespace neuraylib
    {
        class Module;

    } // namespace neuraylib

} // namespace mi

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_WEAK_PTRS(UsdStage);

/// Translate the MDL document in \p mdl into the stage \p stage.
/// \p internalPath is a namespace path where converted MDL objects
/// will live. \p externalPath is a namespace path of a prim that will
/// have all of the look variants.  It will have references into
/// \p internalPath.  Clients are expected to reference the prim at
/// \p externalPath to apply looks.
USDMDL_LOCAL
void UsdMdlRead(mi::neuraylib::Module& mdl,
                 const UsdStagePtr& stage,
                 const SdfPath& internalPath = SdfPath("/MDL"),
                 const SdfPath& externalPath = SdfPath("/ModelRoot"));

PXR_NAMESPACE_CLOSE_SCOPE
