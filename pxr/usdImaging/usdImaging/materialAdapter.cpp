//
// Copyright 2016 Pixar
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
#include "pxr/usdImaging/usdImaging/materialAdapter.h"
#include "pxr/usdImaging/usdImaging/delegate.h"
#include "pxr/usdImaging/usdImaging/indexProxy.h"
#include "pxr/usdImaging/usdImaging/tokens.h"
#include "pxr/usdImaging/usdImaging/materialParamUtils.h"

#include "pxr/imaging/hd/material.h"
#include "pxr/imaging/hd/perfLog.h"

#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/shader.h"

#include "pxr/usd/ar/resolverScopedCache.h"
#include "pxr/usd/ar/resolverContextBinder.h"

// #nv begin #new-MDL-schema
#include "pxr/usd/ar/resolver.h"
#include "pxr/usd/sdr/registry.h"
// nv end

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    typedef UsdImagingMaterialAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingPrimAdapterFactory<Adapter> >();
}
UsdImagingMaterialAdapter::~UsdImagingMaterialAdapter()
{
}

bool
UsdImagingMaterialAdapter::IsSupported(UsdImagingIndexProxy const* index) const
{
    return index->IsSprimTypeSupported(HdPrimTypeTokens->material);
}

SdfPath
UsdImagingMaterialAdapter::Populate(
    UsdPrim const& prim,
    UsdImagingIndexProxy* index,
    UsdImagingInstancerContext const* instancerContext)
{
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();
    // Since material are populated by reference, they need to take care not to
    // be populated multiple times.
    SdfPath cachePath = prim.GetPath();
    if (index->IsPopulated(cachePath)) {
        return cachePath;
    }

    UsdShadeMaterial material(prim);
    if (!material) return SdfPath::EmptyPath();

    // Skip materials that do not match renderDelegate supported types.
    // XXX We can further improve filtering by combining the below descendants
    // gather and validate the Sdr node types are supported by render delegate.
    const TfTokenVector contextVector = _GetMaterialRenderContexts();
    UsdShadeShader surface = material.ComputeSurfaceSource(contextVector);
    if (!surface) {
        UsdShadeShader volume = material.ComputeVolumeSource(contextVector);
        if (!volume) return SdfPath::EmptyPath();
    }

    index->InsertSprim(HdPrimTypeTokens->material,
                       cachePath,
                       prim, shared_from_this());
    HD_PERF_COUNTER_INCR(UsdImagingTokens->usdPopulatedPrimCount);

    // Also register dependencies on behalf of any descendent
    // UsdShadeShader prims, since they are consumed to
    // create the material network.
    for (UsdPrim const& child: prim.GetDescendants()) {
        if (child.IsA<UsdShadeShader>()) {
            index->AddDependency(cachePath, child);
        }
    }

    return prim.GetPath();
}

/* virtual */
void
UsdImagingMaterialAdapter::TrackVariability(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    HdDirtyBits* timeVaryingBits,
    UsdImagingInstancerContext const* instancerContext,
    // #nv begin fast-updates
    bool checkVariability) const
    // nv end
{
    // #nv begin fast-updates
    // Early out, as there are no initial values to populate into the value cache.
    if (!checkVariability)
        return;
    // nv end

    TRACE_FUNCTION();
    UsdShadeMaterial material(prim);
    if (!material) {
        TF_RUNTIME_ERROR("Expected material prim at <%s> to be of type "
                         "'UsdShadeMaterial', not type '%s'; ignoring",
                         prim.GetPath().GetText(),
                         prim.GetTypeName().GetText());
        return;
    }

    const TfTokenVector contextVector = _GetMaterialRenderContexts();
    if (UsdShadeShader s = material.ComputeSurfaceSource(contextVector)) {
        if (UsdImaging_IsHdMaterialNetworkTimeVarying(s.GetPrim())) {
            *timeVaryingBits |= HdMaterial::DirtyResource;
            return;
        }
        // Only check if displacement is timeVarying if we also have a surface 
        if (UsdShadeShader d = 
                material.ComputeDisplacementSource(contextVector)) {
            if (UsdImaging_IsHdMaterialNetworkTimeVarying(d.GetPrim())) {
                *timeVaryingBits |= HdMaterial::DirtyResource;
            }
        }
        return;
    }

    if (UsdShadeShader v = material.ComputeVolumeSource(contextVector)) {
        if (UsdImaging_IsHdMaterialNetworkTimeVarying(v.GetPrim())) {
            *timeVaryingBits |= HdMaterial::DirtyResource;
        }
        return;
    }
}

/* virtual */
void
UsdImagingMaterialAdapter::UpdateForTime(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    UsdTimeCode time,
    HdDirtyBits requestedBits,
    UsdImagingInstancerContext const*
    instancerContext) const
{
}

/* virtual */
HdDirtyBits
UsdImagingMaterialAdapter::ProcessPropertyChange(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    TfToken const& propertyName)
{
    if (propertyName == UsdGeomTokens->visibility) {
        // Materials aren't affected by visibility
        return HdChangeTracker::Clean;
    }

    // #nv begin #material-resync
    if (UsdGeomXformOp::IsXformOp(propertyName))
    {
        // Materials aren't affected by transform
        return HdChangeTracker::Clean;
    }
    // #nv end

    // The only meaningful change is to dirty the computed resource,
    // an HdMaterialNetwork.
    return HdMaterial::DirtyResource;
}

/* virtual */
void
UsdImagingMaterialAdapter::MarkDirty(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    HdDirtyBits dirty,
    UsdImagingIndexProxy* index)
{
    // If this is invoked on behalf of a Shader prim underneath a
    // Material prim, walk up to the enclosing Material.
    SdfPath materialCachePath = cachePath;
    UsdPrim materialPrim = prim;
    while (materialPrim && !materialPrim.IsA<UsdShadeMaterial>()) {
        materialPrim = materialPrim.GetParent();
        materialCachePath = materialCachePath.GetParentPath();
    }
    if (!TF_VERIFY(materialPrim)) {
        return;
    }

    index->MarkSprimDirty(materialCachePath, dirty);
}


/* virtual */
void
UsdImagingMaterialAdapter::MarkMaterialDirty(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    UsdImagingIndexProxy* index)
{
    MarkDirty(prim, cachePath, HdMaterial::DirtyResource, index);
}


/* virtual */
void
UsdImagingMaterialAdapter::_RemovePrim(
    SdfPath const& cachePath,
    UsdImagingIndexProxy* index)
{
    index->RemoveSprim(HdPrimTypeTokens->material, cachePath);
}

// XXX:aluk
// TODO: resolve merge conflict
#if 0
VtValue
UsdImagingMaterialAdapter::GetMaterialResource(UsdPrim const& prim,
    SdfPath const& cachePath,
    UsdTimeCode time) const
{
    HdMaterialNetworkMap map;
    TfToken const& networkSelector = _GetMaterialNetworkSelector();
    // #nv begin fast-updates
    _GetMaterialNetworkMap(prim, networkSelector, &map, time);
    // nv end
    return VtValue(map);
}

static TfToken
_GetPrimvarNameAttributeValue(
    SdrShaderNodeConstPtr const& sdrNode,
    HdMaterialNode const& node,
    TfToken const& propName)
{
    VtValue vtName;

    // If the name of the primvar was authored in parameters list.
    // The authored value is the strongest opinion

    auto const& paramIt = node.parameters.find(propName);
    if (paramIt != node.parameters.end()) {
        vtName = paramIt->second;
    }

    // If we didn't find an authored value consult Sdr for the default value.

    if (vtName.IsEmpty() && sdrNode) {
        if (SdrShaderPropertyConstPtr sdrPrimvarInput = 
                sdrNode->GetShaderInput(propName)) {
            vtName = sdrPrimvarInput->GetDefaultValue();
        }
    }

    if (vtName.IsHolding<TfToken>()) {
        return vtName.UncheckedGet<TfToken>();
    } else if (vtName.IsHolding<std::string>()) {
        return TfToken(vtName.UncheckedGet<std::string>());
    }

    return TfToken();
}

static void
_ExtractPrimvarsFromNode(
    HdMaterialNode const& node,
    HdMaterialNetwork *materialNetwork,
    TfToken const& networkSelector)
{
    SdrRegistry &shaderReg = SdrRegistry::GetInstance();
    SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifierAndType(
        node.identifier, networkSelector);

    if (sdrNode) {
        // GetPrimvars and GetAdditionalPrimvarProperties together give us the
        // complete set of primvars needed by this shader node.
        NdrTokenVec const& primvars = sdrNode->GetPrimvars();
        materialNetwork->primvars.insert( 
            materialNetwork->primvars.end(), primvars.begin(), primvars.end());

        for (TfToken const& p : sdrNode->GetAdditionalPrimvarProperties()) {
            TfToken name = _GetPrimvarNameAttributeValue(sdrNode, node, p);
            materialNetwork->primvars.push_back(name);
        }
    }
}

// Walk the shader graph and emit nodes in topological order to avoid
// forward-references.
// This current implementation of _WalkGraph flattens the shading network into
// a single graph with connectivity and values. It does not try to identify
// NodeGraphs that can be processed once and shared, or even look for a
// pre-baked implementation. Currently neither the material processing in Hydra
// nor any of the back-ends (like HdPrman) can make use of this anyway.
using _PathSet = std::unordered_set<SdfPath, SdfPath::Hash>;

static
void _WalkGraph(
    UsdShadeShader const & shadeNode,
    HdMaterialNetwork* materialNetwork,
    TfToken const& networkSelector,
    _PathSet* visitedNodes,
    TfTokenVector const & shaderSourceTypes,
    UsdTimeCode time,
    bool* timeVarying)
{
    // Store the path of the node
    HdMaterialNode node;
    node.path = shadeNode.GetPath();
    if (!TF_VERIFY(node.path != SdfPath::EmptyPath())) {
        return;
    }

    // If this node has already been found via another path, we do
    // not need to add it again.
    if (visitedNodes->count(node.path) > 0) {
        return;
    }
    visitedNodes->emplace(node.path);

    // Visit the inputs of this node to ensure they are emitted first.
    const std::vector<UsdShadeInput> shadeNodeInputs = shadeNode.GetInputs();
    for (UsdShadeInput input: shadeNodeInputs) {

        TfToken inputName = input.GetBaseName();

        // Find the attribute this input is getting its value from, which might
        // be an output or an input, including possibly itself if not connected
        UsdShadeAttributeType attrType;
        UsdAttribute attr = input.GetValueProducingAttribute(&attrType);

        if (attrType == UsdShadeAttributeType::Output) {
            // If it is an output on a shading node we visit the node and also
            // create a relationship in the network
            _WalkGraph(UsdShadeShader(
                attr.GetPrim()),
                materialNetwork,
                networkSelector,
                visitedNodes,
                shaderSourceTypes,
                time,
                timeVarying);

            HdMaterialRelationship relationship;
            relationship.outputId = node.path;
            relationship.outputName = inputName;
            relationship.inputId = attr.GetPrim().GetPath();
            relationship.inputName = UsdShadeOutput(attr).GetBaseName();
            materialNetwork->relationships.push_back(relationship);
        } else if (attrType == UsdShadeAttributeType::Input) {
            // If it is an input attribute we get the authored value.
            //
            // If its type is asset and contains <UDIM>,
            // we resolve the asset path with the udim pattern to a file
            // path with a udim pattern, e.g.,
            // //SHOW/myImage.<UDIM>.exr to /filePath/myImage.<UDIM>.exr.

            // #nv begin material-updates
            const VtValue value =
                timeVarying == nullptr ? UsdImaging_ResolveMaterialParamValue(attr, time) : VtValue();
            // nv end
            if (!value.IsEmpty()) {
                node.parameters[inputName] = value;
            }

            // #nv begin material-updates
            if (timeVarying)
            // nv end
                *timeVarying |= attr.ValueMightBeTimeVarying();

// #nv begin #parameter-colorSpace-to-hydra
            TfToken colorSpaceToken = attr.GetColorSpace();
            if (!colorSpaceToken.IsEmpty())
            {
                node.paramColorSpace[inputName] = colorSpaceToken;
            }
// #nv end 
        }
    }

    // Extract the identifier of the node.
    // GetShaderNodeForSourceType will try to find/create an Sdr node for all
    // three info cases: info:id, info:sourceAsset and info:sourceCode.
    TfToken id;
    if (!shadeNode.GetShaderId(&id)) {
        for (auto const& sourceType : shaderSourceTypes) {
            if (SdrShaderNodeConstPtr sdrNode = 
                    shadeNode.GetShaderNodeForSourceType(sourceType)) {
                id = sdrNode->GetIdentifier();
                break;
            }
        }
    }

    if (!id.IsEmpty()) {
        node.identifier = id;

        // GprimAdapter can filter-out primvars not used by a material to reduce
        // the number of primvars send to the render delegate. We extract the
        // primvar names from the material node to ensure these primvars are
        // not filtered-out by GprimAdapter.
        _ExtractPrimvarsFromNode(node, materialNetwork, networkSelector);
    }

 // #nv begin #new-MDL-schema   
    if (timeVarying == nullptr && node.identifier.IsEmpty()) {
        _GetShaderNodeForSourceTypeFallbackNV(
            shadeNode, networkSelector, &node.identifier, &node.subIdentifier);
    }  
// #nv end

    materialNetwork->nodes.push_back(node);
}

static void
_BuildHdMaterialNetworkFromTerminal(
    UsdShadeShader const& usdTerminal,
    TfToken const& terminalIdentifier,
    TfToken const& networkSelector,
    TfTokenVector const& shaderSourceTypes,
    HdMaterialNetworkMap *materialNetworkMap,
    UsdTimeCode time,
    bool* timeVarying)
{
    HdMaterialNetwork& network = materialNetworkMap->map[terminalIdentifier];
    std::vector<HdMaterialNode>& nodes = network.nodes;
    _PathSet visitedNodes;

    _WalkGraph(
        usdTerminal,
        &network,
        networkSelector,
        &visitedNodes, 
        shaderSourceTypes,
        time,
        timeVarying);

    if (!TF_VERIFY(!nodes.empty())) return;

    // _WalkGraph() inserts the terminal last in the nodes list.
    HdMaterialNode& terminalNode = nodes.back();
    
    // Store terminals on material so backend can easily access them.
    materialNetworkMap->terminals.push_back(terminalNode.path);

    // #nv begin #new-MDL-schema
    if (!_UnregisteredShaderIDsAllowed()) {
    // nv end
        // Validate that idenfitier (info:id) is known to Sdr.
        // Return empty network if it fails so backend can use fallback material.
        SdrRegistry &shaderReg = SdrRegistry::GetInstance();
        if (!shaderReg.GetNodeByIdentifier(terminalNode.identifier)) {
            TF_WARN("Invalid info:id %s node: %s",
                terminalNode.identifier.GetText(),
                terminalNode.path.GetText());
            *materialNetworkMap = HdMaterialNetworkMap();
        }
    // #nv begin #new-MDL-schema
    }
    // nv end
};

void 
UsdImagingMaterialAdapter::_GetMaterialNetworkMap(
    UsdPrim const &usdPrim, 
    TfToken const& networkSelector,
    HdMaterialNetworkMap *networkMap,
    UsdTimeCode time,
    bool* timeVarying) const
#endif

/* virtual */
void
UsdImagingMaterialAdapter::ProcessPrimResync(
        SdfPath const& cachePath,
        UsdImagingIndexProxy *index)
{
    // Since we're resyncing a material, we can use the cache path as a
    // usd path.  We need to resync dependents to make sure rprims bound to
    // this material are resynced; this is necessary to make sure the material
    // is repopulated, since we don't directly populate materials.
    SdfPath const& usdPath = cachePath;
    _ResyncDependents(usdPath, index);

    UsdImagingPrimAdapter::ProcessPrimResync(cachePath, index);
}

VtValue 
UsdImagingMaterialAdapter::GetMaterialResource(UsdPrim const &prim,
                                               SdfPath const& cachePath, 
                                               UsdTimeCode time) const
{
    TRACE_FUNCTION();
    UsdShadeMaterial material(prim);
    if (!material) {
        TF_RUNTIME_ERROR("Expected material prim at <%s> to be of type "
                         "'UsdShadeMaterial', not type '%s'; ignoring",
                         prim.GetPath().GetText(),
                         prim.GetTypeName().GetText());
        return VtValue();
    }

    // Bind the usd stage's resolver context for correct asset resolution.
    ArResolverContextBinder binder(prim.GetStage()->GetPathResolverContext());
    ArResolverScopedCache resolverCache;

    HdMaterialNetworkMap networkMap;

    const TfTokenVector contextVector = _GetMaterialRenderContexts();
    TfTokenVector shaderSourceTypes = _GetShaderSourceTypes();

    if (UsdShadeShader surface = material.ComputeSurfaceSource(contextVector)) {
        UsdImaging_BuildHdMaterialNetworkFromTerminal(
            surface.GetPrim(), 
            HdMaterialTerminalTokens->surface,
            shaderSourceTypes,
            contextVector,
            &networkMap,
            time);

        // Only build a displacement materialNetwork if we also have a surface
        if (UsdShadeShader displacement = 
                    material.ComputeDisplacementSource(contextVector)) {
            UsdImaging_BuildHdMaterialNetworkFromTerminal(
                displacement.GetPrim(),
                HdMaterialTerminalTokens->displacement,
                shaderSourceTypes,
                contextVector,
                &networkMap,
                time);
        }
    }

    // Only build a volume materialNetwork if we do not have a surface
    else if (UsdShadeShader volume = 
                    material.ComputeVolumeSource(contextVector)) {
        UsdImaging_BuildHdMaterialNetworkFromTerminal(
            volume.GetPrim(),
            HdMaterialTerminalTokens->volume,
            shaderSourceTypes,
            contextVector,
            &networkMap,
            time);
    }

    return VtValue(networkMap);
}


PXR_NAMESPACE_CLOSE_SCOPE
