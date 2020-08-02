//
// Copyright 2017 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/usdImaging/usdImaging/domeLightAdapter.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/usdImaging/usdImaging/indexProxy.h"
#include "pxr/usdImaging/usdImaging/tokens.h"

#include "pxr/imaging/hd/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE


TF_REGISTRY_FUNCTION(TfType)
{
    typedef UsdImagingDomeLightAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingPrimAdapterFactory<Adapter> >();
}

UsdImagingDomeLightAdapter::~UsdImagingDomeLightAdapter() 
{
}

bool
UsdImagingDomeLightAdapter::IsSupported(UsdImagingIndexProxy const* index) const
{
    return UsdImagingLightAdapter::IsEnabledSceneLights() &&
           index->IsSprimTypeSupported(HdPrimTypeTokens->domeLight);
}

SdfPath
UsdImagingDomeLightAdapter::Populate(UsdPrim const& prim, 
                            UsdImagingIndexProxy* index,
                            UsdImagingInstancerContext const* instancerContext)
{
    index->InsertSprim(HdPrimTypeTokens->domeLight, prim.GetPath(), prim);
    HD_PERF_COUNTER_INCR(UsdImagingTokens->usdPopulatedPrimCount);

// #nv begin #bind-material-to-domelight
    // Copied from UsdImagingGprimAdapter
    auto materialUsdPath = GetMaterialUsdPath(prim);

    // Allow instancer context to override the material binding.
    SdfPath resolvedUsdMaterialPath = instancerContext ?
        instancerContext->instancerMaterialUsdPath : materialUsdPath;
    UsdPrim materialPrim =
        prim.GetStage()->GetPrimAtPath(resolvedUsdMaterialPath);

    if (materialPrim) {
        if (materialPrim.IsA<UsdShadeMaterial>()) {
            UsdImagingPrimAdapterSharedPtr materialAdapter =
                index->GetMaterialAdapter(materialPrim);
            if (materialAdapter) {
                materialAdapter->Populate(materialPrim, index, nullptr);
                // We need to register a dependency on the material prim so
                // that geometry is updated when the material is
                // (specifically, DirtyMaterialId).
                // XXX: Eventually, it would be great to push this into hydra.
                index->AddDependency(prim.GetPath(), materialPrim);
            }
        } else {
            TF_WARN("DomeLight <%s> has illegal material reference to "
                    "prim <%s> of type (%s)", prim.GetPath().GetText(),
                    materialPrim.GetPath().GetText(),
                    materialPrim.GetTypeName().GetText());
        }
    }
// #nv end

    return prim.GetPath();
}

void
UsdImagingDomeLightAdapter::_RemovePrim(SdfPath const& cachePath,
                                         UsdImagingIndexProxy* index)
{
    index->RemoveSprim(HdPrimTypeTokens->domeLight, cachePath);
}


PXR_NAMESPACE_CLOSE_SCOPE
