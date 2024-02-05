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
#include "pxr/usd/usdPhysics/vtxTetAttachment.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsVtxTetAttachment,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsVtxTetAttachment")
    // to find TfType<UsdPhysicsVtxTetAttachment>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsVtxTetAttachment>("PhysicsVtxTetAttachment");
}

/* virtual */
UsdPhysicsVtxTetAttachment::~UsdPhysicsVtxTetAttachment()
{
}

/* static */
UsdPhysicsVtxTetAttachment
UsdPhysicsVtxTetAttachment::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVtxTetAttachment();
    }
    return UsdPhysicsVtxTetAttachment(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsVtxTetAttachment
UsdPhysicsVtxTetAttachment::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsVtxTetAttachment");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVtxTetAttachment();
    }
    return UsdPhysicsVtxTetAttachment(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsVtxTetAttachment::_GetSchemaKind() const
{
    return UsdPhysicsVtxTetAttachment::schemaKind;
}

/* static */
const TfType &
UsdPhysicsVtxTetAttachment::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsVtxTetAttachment>();
    return tfType;
}

/* static */
bool 
UsdPhysicsVtxTetAttachment::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsVtxTetAttachment::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetAttachmentEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsAttachmentEnabled);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreateAttachmentEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsAttachmentEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetPhysicsVtxIndicesAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsVtxIndices);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreatePhysicsVtxIndicesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsVtxIndices,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetPhysicsTetIndicesAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTetIndices);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreatePhysicsTetIndicesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTetIndices,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetPhysicsTetCoordsAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTetCoords);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreatePhysicsTetCoordsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTetCoords,
                       SdfValueTypeNames->Vector3fArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStiffness);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreateStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::GetDampingAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsDamping);
}

UsdAttribute
UsdPhysicsVtxTetAttachment::CreateDampingAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsDamping,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsVtxTetAttachment::GetVtxSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsVtxSrc);
}

UsdRelationship
UsdPhysicsVtxTetAttachment::CreateVtxSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsVtxSrc,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsVtxTetAttachment::GetTetSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsTetSrc);
}

UsdRelationship
UsdPhysicsVtxTetAttachment::CreateTetSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsTetSrc,
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
UsdPhysicsVtxTetAttachment::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsAttachmentEnabled,
        UsdPhysicsTokens->physicsVtxIndices,
        UsdPhysicsTokens->physicsTetIndices,
        UsdPhysicsTokens->physicsTetCoords,
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
