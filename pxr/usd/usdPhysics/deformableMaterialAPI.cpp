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
#include "pxr/usd/usdPhysics/deformableMaterialAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/tokens.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsDeformableMaterialAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (PhysicsDeformableMaterialAPI)
);

/* virtual */
UsdPhysicsDeformableMaterialAPI::~UsdPhysicsDeformableMaterialAPI()
{
}

/* static */
UsdPhysicsDeformableMaterialAPI
UsdPhysicsDeformableMaterialAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsDeformableMaterialAPI();
    }
    return UsdPhysicsDeformableMaterialAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPhysicsDeformableMaterialAPI::_GetSchemaKind() const
{
    return UsdPhysicsDeformableMaterialAPI::schemaKind;
}

/* static */
bool
UsdPhysicsDeformableMaterialAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPhysicsDeformableMaterialAPI>(whyNot);
}

/* static */
UsdPhysicsDeformableMaterialAPI
UsdPhysicsDeformableMaterialAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPhysicsDeformableMaterialAPI>()) {
        return UsdPhysicsDeformableMaterialAPI(prim);
    }
    return UsdPhysicsDeformableMaterialAPI();
}

/* static */
const TfType &
UsdPhysicsDeformableMaterialAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsDeformableMaterialAPI>();
    return tfType;
}

/* static */
bool 
UsdPhysicsDeformableMaterialAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsDeformableMaterialAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsDeformableMaterialAPI::GetYoungsModulusAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsYoungsModulus);
}

UsdAttribute
UsdPhysicsDeformableMaterialAPI::CreateYoungsModulusAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsYoungsModulus,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsDeformableMaterialAPI::GetPoissonsRatioAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsPoissonsRatio);
}

UsdAttribute
UsdPhysicsDeformableMaterialAPI::CreatePoissonsRatioAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsPoissonsRatio,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
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
UsdPhysicsDeformableMaterialAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsYoungsModulus,
        UsdPhysicsTokens->physicsPoissonsRatio,
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
