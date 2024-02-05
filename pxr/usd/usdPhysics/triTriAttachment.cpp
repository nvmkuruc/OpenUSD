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
#include "pxr/usd/usdPhysics/triTriAttachment.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsTriTriAttachment,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsTriTriAttachment")
    // to find TfType<UsdPhysicsTriTriAttachment>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsTriTriAttachment>("PhysicsTriTriAttachment");
}

/* virtual */
UsdPhysicsTriTriAttachment::~UsdPhysicsTriTriAttachment()
{
}

/* static */
UsdPhysicsTriTriAttachment
UsdPhysicsTriTriAttachment::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsTriTriAttachment();
    }
    return UsdPhysicsTriTriAttachment(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsTriTriAttachment
UsdPhysicsTriTriAttachment::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsTriTriAttachment");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsTriTriAttachment();
    }
    return UsdPhysicsTriTriAttachment(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsTriTriAttachment::_GetSchemaKind() const
{
    return UsdPhysicsTriTriAttachment::schemaKind;
}

/* static */
const TfType &
UsdPhysicsTriTriAttachment::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsTriTriAttachment>();
    return tfType;
}

/* static */
bool 
UsdPhysicsTriTriAttachment::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsTriTriAttachment::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetAttachmentEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsAttachmentEnabled);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreateAttachmentEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsAttachmentEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetPhysicsTriIndices0Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTriIndices0);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreatePhysicsTriIndices0Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTriIndices0,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetPhysicsTriIndices1Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTriIndices1);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreatePhysicsTriIndices1Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTriIndices1,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetPhysicsTriCoords0Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTriCoords0);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreatePhysicsTriCoords0Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTriCoords0,
                       SdfValueTypeNames->Vector3fArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetPhysicsTriCoords1Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsTriCoords1);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreatePhysicsTriCoords1Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsTriCoords1,
                       SdfValueTypeNames->Vector3fArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStiffness);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreateStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsTriTriAttachment::GetDampingAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsDamping);
}

UsdAttribute
UsdPhysicsTriTriAttachment::CreateDampingAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsDamping,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsTriTriAttachment::GetTriSrc0Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsTriSrc0);
}

UsdRelationship
UsdPhysicsTriTriAttachment::CreateTriSrc0Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsTriSrc0,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsTriTriAttachment::GetTriSrc1Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsTriSrc1);
}

UsdRelationship
UsdPhysicsTriTriAttachment::CreateTriSrc1Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsTriSrc1,
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
UsdPhysicsTriTriAttachment::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsAttachmentEnabled,
        UsdPhysicsTokens->physicsTriIndices0,
        UsdPhysicsTokens->physicsTriIndices1,
        UsdPhysicsTokens->physicsTriCoords0,
        UsdPhysicsTokens->physicsTriCoords1,
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
