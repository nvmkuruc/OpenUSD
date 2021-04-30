//
// Copyright 2020 Pixar
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

// #nv begin #material-updates
#include "pxr/usdImaging/usdImaging/debugCodes.h"
// nv end

#include "pxr/usdImaging/usdImaging/materialParamUtils.h"

#include "pxr/imaging/hd/material.h"
#include "pxr/usd/ar/resolver.h"
#include "pxr/usd/sdf/layerUtils.h"
#include "pxr/usd/sdr/registry.h"
#include "pxr/usd/sdr/shaderNode.h"
#include "pxr/usd/sdr/shaderProperty.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usdShade/connectableAPI.h"
// #nv begin #materialx-backport
#include "pxr/usd/usdShade/shader.h"
// #nv end

PXR_NAMESPACE_OPEN_SCOPE

static const char UDIM_PATTERN[] = "<UDIM>";
static const int UDIM_START_TILE = 1001;
static const int UDIM_END_TILE = 1100;
static const std::string::size_type UDIM_TILE_NUMBER_LENGTH = 4;

// We need to find the first layer that changes the value
// of the parameter so that we anchor relative paths to that.
static
SdfLayerHandle 
_FindLayerHandle(const UsdAttribute& attr, const UsdTimeCode& time) {
    for (const auto& spec: attr.GetPropertyStack(time)) {
        if (spec->HasDefaultValue() ||
            spec->GetLayer()->GetNumTimeSamplesForPath(
                spec->GetPath()) > 0) {
            return spec->GetLayer();
        }
    }
    return TfNullPtr;
}

// Given the prefix (e.g., //SHOW/myImage.) and suffix (e.g., .exr),
// add integer between them and try to resolve. Iterate until
// resolution succeeded.
static
std::string
_ResolvedPathForFirstTile(
    const std::pair<std::string, std::string> &splitPath,
    SdfLayerHandle const &layer)
{
    TRACE_FUNCTION();

    ArResolver& resolver = ArGetResolver();
    
    for (int i = UDIM_START_TILE; i < UDIM_END_TILE; i++) {
        // Fill in integer
        std::string path =
            splitPath.first + std::to_string(i) + splitPath.second;
        if (layer) {
            // Deal with layer-relative paths.
            path = SdfComputeAssetPathRelativeToLayer(layer, path);
        }
        // Resolve
        path = resolver.Resolve(path);
        if (!path.empty()) {
            return path;
        }
    }
    return std::string();
}

// Split a udim file path such as /someDir/myFile.<UDIM>.exr into a
// prefix (/someDir/myFile.) and suffix (.exr).
//
// We might support other patterns such as /someDir/myFile._MAPID_.exr
// in the future.
static
std::pair<std::string, std::string>
_SplitUdimPattern(const std::string &path)
{
    static const std::vector<std::string> patterns = { UDIM_PATTERN };

    for (const std::string &pattern : patterns) {
        const std::string::size_type pos = path.find(pattern);
        if (pos != std::string::npos) {
            return { path.substr(0, pos), path.substr(pos + pattern.size()) };
        }
    }

    return { std::string(), std::string() };
}

// If given assetPath contains UDIM pattern, resolve the UDIM pattern.
// Otherwise, leave assetPath untouched.
static
SdfAssetPath
_ResolveAssetAttribute(
    const SdfAssetPath& assetPath,
    const UsdAttribute& attr,
    const UsdTimeCode& time)
{
    TRACE_FUNCTION();

    // See whether the asset path contains UDIM pattern.
    const std::pair<std::string, std::string>
        splitPath = _SplitUdimPattern(assetPath.GetAssetPath());

    if (splitPath.first.empty() && splitPath.second.empty()) {
        // Leave untouched.
        return assetPath;
    }

    // Find first tile.
    const std::string firstTilePath =
        _ResolvedPathForFirstTile(splitPath, _FindLayerHandle(attr, time));

    if (firstTilePath.empty()) {
        return assetPath;
    }
    
    // Construct the file path /filePath/myImage.<UDIM>.exr by using
    // the first part from the first resolved tile, "<UDIM>" and the
    // suffix.

    const std::string &suffix = splitPath.second;

    // Sanity check that the part after <UDIM> did not change.
    if (!TfStringEndsWith(firstTilePath, suffix)) {
        TF_WARN(
            "Resolution of first udim tile gave ambigious result. "
            "First tile for '%s' is '%s'.",
            assetPath.GetAssetPath().c_str(), firstTilePath.c_str());
        return assetPath;
    }

    // Length of the part /filePath/myImage.<UDIM>.exr.
    const std::string::size_type prefixLength =
        firstTilePath.size() - suffix.size() - UDIM_TILE_NUMBER_LENGTH;

    return
        SdfAssetPath( 
            assetPath.GetAssetPath(),
            firstTilePath.substr(0, prefixLength) + UDIM_PATTERN + suffix);
}

VtValue
UsdImaging_ResolveMaterialParamValue(
    const UsdAttribute& attr,
    const UsdTimeCode& time)
{
    TRACE_FUNCTION();

    // #nv begin #material-updates
    if (TfDebug::IsEnabled(USDIMAGING_TEXTURES) && (attr.GetTypeName() == SdfValueTypeNames->Asset)) {
        TF_DEBUG(USDIMAGING_TEXTURES).Msg("%s will resolve <%s> at %s\n",
            TF_FUNC_NAME().c_str(), attr.GetPath().GetText(), TfStringify(time).c_str());
    }
    // nv end

    VtValue value;

    if (!attr.Get(&value, time)) {
        return value;
    }
    
    if (!value.IsHolding<SdfAssetPath>()) {
        return value;
    }

    return VtValue(
        _ResolveAssetAttribute(
            value.UncheckedGet<SdfAssetPath>(), attr, time));
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
    UsdShadeShader const & shadeNode,
    HdMaterialNode const & node,
    HdMaterialNetwork *materialNetwork,
    TfTokenVector const & shaderSourceTypes)
{
    SdrRegistry &shaderReg = SdrRegistry::GetInstance();
    SdrShaderNodeConstPtr sdrNode = shaderReg.GetShaderNodeByIdentifier(
        node.identifier, shaderSourceTypes);

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

// #nv begin #new-MDL-schema
static void
_GetShaderNodeForSourceTypeFallbackNV(
    UsdShadeShader const& shader,
    TfTokenVector const& contexts,
    TfToken* identifier=nullptr,
    TfToken* subIdentifier=nullptr)
{
    TfToken implSource = shader.GetImplementationSource();

    if (implSource == UsdShadeTokens->id) {
        TfToken shaderId;

        for (auto& context : contexts) {
            if (shader.GetShaderId(&shaderId)) {
                auto &shaderReg = SdrRegistry::GetInstance();
                if (SdrShaderNodeConstPtr sdrNode = 
                        shaderReg.GetShaderNodeByIdentifierAndType(shaderId, 
                            context)) {
                    if (identifier) {
                        *identifier = TfToken(sdrNode->GetResolvedImplementationURI());
                    }
                    return;
                }
            }
        }
    } else if (implSource == UsdShadeTokens->sourceAsset) {
        SdfAssetPath sourceAsset;
        bool found = false;
        for (auto& context : contexts) {
            if (shader.GetSourceAsset(&sourceAsset, context)) {
                std::string path = sourceAsset.GetResolvedPath();
                if (path.empty()) {
                    path = ArGetResolver().Resolve(sourceAsset.GetAssetPath());
                }
                if (path.empty()) {
                    path = sourceAsset.GetAssetPath();
                }
                if (identifier) {
                    *identifier = TfToken(path);
                    found = true;
                }
            }
            if (subIdentifier) {
                if (shader.GetSourceAssetSubIdentifier(subIdentifier,
                                                       context)) {
                    found = true;
                }
            }
            if (found) {
                return;
            }
        }
    } else if (implSource == UsdShadeTokens->sourceCode) {
        std::string sourceCode;
        for (auto& context : contexts) {
            if (shader.GetSourceCode(&sourceCode, context)) {
                if (identifier) {
                    *identifier = TfToken(sourceCode);
                }
                return;
            }
        }
    }  
}

TF_DEFINE_ENV_SETTING(USDIMAGING_ALLOW_UNREGISTERED_SHADER_IDS, 0,
    "Allows material networks to be defined under IDs which are not known to SdrRegistry.");
static bool _UnregisteredShaderIDsAllowed() {
    static bool _v = TfGetEnvSetting(USDIMAGING_ALLOW_UNREGISTERED_SHADER_IDS) == 1;
    return _v;
}

// #nv end

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
    UsdShadeConnectableAPI const & shadeNode,
    HdMaterialNetwork* materialNetwork,
    _PathSet* visitedNodes,
    TfTokenVector const & shaderSourceTypes,
    UsdTimeCode time,
// #nv begin #materialx-backport
    TfTokenVector const* contexts)
// #nv end
{
    // Store the path of the node
    HdMaterialNode node;
    node.path = shadeNode.GetPath();
    if (!TF_VERIFY(node.path != SdfPath::EmptyPath())) {
        return;
    }

    // If this node has already been found via another path, we do
    // not need to add it again.
    if (!visitedNodes->insert(shadeNode.GetPath()).second) {
        return;
    }

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
            _WalkGraph(UsdShadeConnectableAPI(
                attr.GetPrim()),
                materialNetwork,
                visitedNodes,
                shaderSourceTypes,
                time,
// #nv begin #materialx-backport
                contexts);
// #nv end

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

             const VtValue value =
                UsdImaging_ResolveMaterialParamValue(attr, time);
            if (!value.IsEmpty()) {
                node.parameters[inputName] = value;
            }

// #nv begin #parameter-colorSpace-to-hydra
            TfToken colorSpaceToken = attr.GetColorSpace();
            if (!colorSpaceToken.IsEmpty())
            {
                node.paramColorSpace[inputName] = colorSpaceToken;
            }
// #nv end 
        }
    }

// #nv begin #materialx-backport
    UsdShadeShader shader = UsdShadeShader(shadeNode.GetPrim());
// #nv end

    // Extract the identifier of the node.
    // GetShaderNodeForSourceType will try to find/create an Sdr node for all
    // three info cases: info:id, info:sourceAsset and info:sourceCode.
    TfToken id;
// #nv begin #materialx-backport    
    if (shader) {
// #nv end
        if (!shader.GetShaderId(&id)) {
            for (auto const& sourceType : shaderSourceTypes) {
                if (SdrShaderNodeConstPtr sdrNode = 
                        shader.GetShaderNodeForSourceType(sourceType)) {
                    id = sdrNode->GetIdentifier();
                    break;
                }
            }
        }
// #nv begin #materialx-backport
    } else {
        // For lights
        id = shadeNode.GetPrim().GetTypeName();
    }
// #nv end

    if (!id.IsEmpty()) {
        node.identifier = id;

        // GprimAdapter can filter-out primvars not used by a material to reduce
        // the number of primvars send to the render delegate. We extract the
        // primvar names from the material node to ensure these primvars are
        // not filtered-out by GprimAdapter.
        _ExtractPrimvarsFromNode(
            shadeNode, node, materialNetwork, shaderSourceTypes);
    }

 // #nv begin #new-MDL-schema   
    if (node.identifier.IsEmpty() && shader && contexts) {
        _GetShaderNodeForSourceTypeFallbackNV(
            shader, *contexts, &node.identifier, &node.subIdentifier);
    }
// #nv end
    materialNetwork->nodes.push_back(node);
}

void
UsdImaging_BuildHdMaterialNetworkFromTerminal(
    UsdPrim const& usdTerminal,
    TfToken const& terminalIdentifier,
    TfTokenVector const& shaderSourceTypes,
    HdMaterialNetworkMap *materialNetworkMap,
    UsdTimeCode time,
// #nv begin #materialx-backport
    TfTokenVector const* contexts)
// #nv end
{
    HdMaterialNetwork& network = materialNetworkMap->map[terminalIdentifier];
    std::vector<HdMaterialNode>& nodes = network.nodes;
    _PathSet visitedNodes;

    _WalkGraph(
        UsdShadeConnectableAPI(usdTerminal),
        &network,
        &visitedNodes, 
        shaderSourceTypes,
        time,
// #nv begin #materialx-backport
        contexts);
// #nv end

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

static
bool _IsGraphTimeVarying(UsdShadeConnectableAPI const & shadeNode,
                         _PathSet* visitedNodes)
{
    // Store the path of the node
    if (!TF_VERIFY(shadeNode.GetPath() != SdfPath::EmptyPath())) {
        return false;
    }

    // If this node has already been found via another path, we do
    // not need to add it again.
    if (!visitedNodes->insert(shadeNode.GetPath()).second) {
        return false;
    }

    // Visit the inputs of this node to ensure they are emitted first.
    const std::vector<UsdShadeInput> shadeNodeInputs = shadeNode.GetInputs();
    for (UsdShadeInput input: shadeNodeInputs) {

        // Find the attribute this input is getting its value from, which might
        // be an output or an input, including possibly itself if not connected
        UsdShadeAttributeType attrType;
        UsdAttribute attr = input.GetValueProducingAttribute(&attrType);

        if (attrType == UsdShadeAttributeType::Output) {
            // If it is an output on a shading node we visit the node and also
            // create a relationship in the network
            if (_IsGraphTimeVarying(
                    UsdShadeConnectableAPI(attr.GetPrim()), visitedNodes)) {
                return true;
            }
        } else if (attrType == UsdShadeAttributeType::Input) {
            // If it is an input attribute we get the authored value.
            if (attr.ValueMightBeTimeVarying()) {
                return true;
            }
        }
    }

    return false;
}

bool
UsdImaging_IsHdMaterialNetworkTimeVarying(
    UsdPrim const& usdTerminal)
{
    _PathSet visitedNodes;
    return _IsGraphTimeVarying(
        UsdShadeConnectableAPI(usdTerminal), &visitedNodes);
};

PXR_NAMESPACE_CLOSE_SCOPE
