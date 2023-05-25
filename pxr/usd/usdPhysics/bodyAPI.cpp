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
#include "pxr/usd/usdPhysics/bodyAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/tokens.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsBodyAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (PhysicsBodyAPI)
);

/* virtual */
UsdPhysicsBodyAPI::~UsdPhysicsBodyAPI()
{
}

/* static */
UsdPhysicsBodyAPI
UsdPhysicsBodyAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsBodyAPI();
    }
    return UsdPhysicsBodyAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPhysicsBodyAPI::_GetSchemaKind() const
{
    return UsdPhysicsBodyAPI::schemaKind;
}

/* static */
bool
UsdPhysicsBodyAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPhysicsBodyAPI>(whyNot);
}

/* static */
UsdPhysicsBodyAPI
UsdPhysicsBodyAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPhysicsBodyAPI>()) {
        return UsdPhysicsBodyAPI(prim);
    }
    return UsdPhysicsBodyAPI();
}

/* static */
const TfType &
UsdPhysicsBodyAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsBodyAPI>();
    return tfType;
}

/* static */
bool 
UsdPhysicsBodyAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsBodyAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsBodyAPI::GetBodyEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsBodyEnabled);
}

UsdAttribute
UsdPhysicsBodyAPI::CreateBodyEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsBodyEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsBodyAPI::GetKinematicEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsKinematicEnabled);
}

UsdAttribute
UsdPhysicsBodyAPI::CreateKinematicEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsKinematicEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsBodyAPI::GetStartsAsleepAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStartsAsleep);
}

UsdAttribute
UsdPhysicsBodyAPI::CreateStartsAsleepAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStartsAsleep,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsBodyAPI::GetSimulationOwnerRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsSimulationOwner);
}

UsdRelationship
UsdPhysicsBodyAPI::CreateSimulationOwnerRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsSimulationOwner,
                       /* custom = */ false);
}

namespace {
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector& left,const TfTokenVector& right)
{
    TfTokenVector result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
}
}

/*static*/
const TfTokenVector&
UsdPhysicsBodyAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsBodyEnabled,
        UsdPhysicsTokens->physicsKinematicEnabled,
        UsdPhysicsTokens->physicsStartsAsleep,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdAPISchemaBase::GetSchemaAttributeNames(true),
            localNames);

    if (includeInherited)
        return allNames;
    else
        return localNames;
}

PXR_NAMESPACE_CLOSE_SCOPE

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'PXR_NAMESPACE_OPEN_SCOPE', 'PXR_NAMESPACE_CLOSE_SCOPE'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
