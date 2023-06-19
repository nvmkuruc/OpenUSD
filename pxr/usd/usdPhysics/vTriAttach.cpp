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
#include "pxr/usd/usdPhysics/vTriAttach.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsVTriAttach,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsVtxTriAttachment")
    // to find TfType<UsdPhysicsVTriAttach>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsVTriAttach>("PhysicsVtxTriAttachment");
}

/* virtual */
UsdPhysicsVTriAttach::~UsdPhysicsVTriAttach()
{
}

/* static */
UsdPhysicsVTriAttach
UsdPhysicsVTriAttach::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVTriAttach();
    }
    return UsdPhysicsVTriAttach(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsVTriAttach
UsdPhysicsVTriAttach::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsVtxTriAttachment");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVTriAttach();
    }
    return UsdPhysicsVTriAttach(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsVTriAttach::_GetSchemaKind() const
{
    return UsdPhysicsVTriAttach::schemaKind;
}

/* static */
const TfType &
UsdPhysicsVTriAttach::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsVTriAttach>();
    return tfType;
}

/* static */
bool 
UsdPhysicsVTriAttach::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsVTriAttach::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsVTriAttach::GetAttachmentEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsAttachmentEnabled);
}

UsdAttribute
UsdPhysicsVTriAttach::CreateAttachmentEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsAttachmentEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVTriAttach::GetPhysicsVtxIndicesAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsVtxIndices);
}

UsdAttribute
UsdPhysicsVTriAttach::CreatePhysicsVtxIndicesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsVtxIndices,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVTriAttach::GetPhysicsTriIndicesAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTriIndices);
}

UsdAttribute
UsdPhysicsVTriAttach::CreatePhysicsTriIndicesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTriIndices,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVTriAttach::GetPhysicsFaceCoordsAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsFaceCoords);
}

UsdAttribute
UsdPhysicsVTriAttach::CreatePhysicsFaceCoordsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsFaceCoords,
                       SdfValueTypeNames->Vector3fArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVTriAttach::GetStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStiffness);
}

UsdAttribute
UsdPhysicsVTriAttach::CreateStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVTriAttach::GetDampingAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsDamping);
}

UsdAttribute
UsdPhysicsVTriAttach::CreateDampingAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsDamping,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsVTriAttach::GetVtxSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsVtxSrc);
}

UsdRelationship
UsdPhysicsVTriAttach::CreateVtxSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsVtxSrc,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsVTriAttach::GetTriSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsTriSrc);
}

UsdRelationship
UsdPhysicsVTriAttach::CreateTriSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsTriSrc,
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
UsdPhysicsVTriAttach::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsAttachmentEnabled,
        UsdPhysicsTokens->physicsVtxIndices,
        UsdPhysicsTokens->physicsTriIndices,
        UsdPhysicsTokens->physicsFaceCoords,
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
