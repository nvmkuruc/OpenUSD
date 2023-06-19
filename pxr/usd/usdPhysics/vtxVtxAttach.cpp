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
#include "pxr/usd/usdPhysics/vtxVtxAttach.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsVtxVtxAttach,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsVtxVtxAttachment")
    // to find TfType<UsdPhysicsVtxVtxAttach>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsVtxVtxAttach>("PhysicsVtxVtxAttachment");
}

/* virtual */
UsdPhysicsVtxVtxAttach::~UsdPhysicsVtxVtxAttach()
{
}

/* static */
UsdPhysicsVtxVtxAttach
UsdPhysicsVtxVtxAttach::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVtxVtxAttach();
    }
    return UsdPhysicsVtxVtxAttach(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsVtxVtxAttach
UsdPhysicsVtxVtxAttach::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsVtxVtxAttachment");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVtxVtxAttach();
    }
    return UsdPhysicsVtxVtxAttach(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsVtxVtxAttach::_GetSchemaKind() const
{
    return UsdPhysicsVtxVtxAttach::schemaKind;
}

/* static */
const TfType &
UsdPhysicsVtxVtxAttach::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsVtxVtxAttach>();
    return tfType;
}

/* static */
bool 
UsdPhysicsVtxVtxAttach::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsVtxVtxAttach::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsVtxVtxAttach::GetAttachmentEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsAttachmentEnabled);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::CreateAttachmentEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsAttachmentEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::GetPhysicsVtxIndices0Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsVtxIndices0);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::CreatePhysicsVtxIndices0Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsVtxIndices0,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::GetPhysicsVtxIndices1Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsVtxIndices1);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::CreatePhysicsVtxIndices1Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsVtxIndices1,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::GetStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStiffness);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::CreateStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::GetDampingAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsDamping);
}

UsdAttribute
UsdPhysicsVtxVtxAttach::CreateDampingAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsDamping,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsVtxVtxAttach::GetVtxSrc0Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsVtxSrc1);
}

UsdRelationship
UsdPhysicsVtxVtxAttach::CreateVtxSrc0Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsVtxSrc1,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsVtxVtxAttach::GetVtxSrc1Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsVtxSrc2);
}

UsdRelationship
UsdPhysicsVtxVtxAttach::CreateVtxSrc1Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsVtxSrc2,
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
UsdPhysicsVtxVtxAttach::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsAttachmentEnabled,
        UsdPhysicsTokens->physicsVtxIndices0,
        UsdPhysicsTokens->physicsVtxIndices1,
        UsdPhysicsTokens->physicsStiffness,
        UsdPhysicsTokens->physicsDamping,
    };
    static TfTokenVector allNames =
        _ConcatenateAttributeNames(
            UsdGeomImageable::GetSchemaAttributeNames(true),
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
