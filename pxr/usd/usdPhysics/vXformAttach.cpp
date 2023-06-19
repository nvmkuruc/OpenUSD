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
#include "pxr/usd/usdPhysics/vXformAttach.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsVXformAttach,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsVtxXformAttachment")
    // to find TfType<UsdPhysicsVXformAttach>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsVXformAttach>("PhysicsVtxXformAttachment");
}

/* virtual */
UsdPhysicsVXformAttach::~UsdPhysicsVXformAttach()
{
}

/* static */
UsdPhysicsVXformAttach
UsdPhysicsVXformAttach::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVXformAttach();
    }
    return UsdPhysicsVXformAttach(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsVXformAttach
UsdPhysicsVXformAttach::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsVtxXformAttachment");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsVXformAttach();
    }
    return UsdPhysicsVXformAttach(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsVXformAttach::_GetSchemaKind() const
{
    return UsdPhysicsVXformAttach::schemaKind;
}

/* static */
const TfType &
UsdPhysicsVXformAttach::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsVXformAttach>();
    return tfType;
}

/* static */
bool 
UsdPhysicsVXformAttach::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsVXformAttach::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsVXformAttach::GetAttachmentEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsAttachmentEnabled);
}

UsdAttribute
UsdPhysicsVXformAttach::CreateAttachmentEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsAttachmentEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVXformAttach::GetPhysicsVtxIndicesAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsVtxIndices);
}

UsdAttribute
UsdPhysicsVXformAttach::CreatePhysicsVtxIndicesAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsVtxIndices,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVXformAttach::GetPhysicsLocalPositionsAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsLocalPositions);
}

UsdAttribute
UsdPhysicsVXformAttach::CreatePhysicsLocalPositionsAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsLocalPositions,
                       SdfValueTypeNames->Point3fArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVXformAttach::GetStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsStiffness);
}

UsdAttribute
UsdPhysicsVXformAttach::CreateStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsVXformAttach::GetDampingAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsDamping);
}

UsdAttribute
UsdPhysicsVXformAttach::CreateDampingAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsDamping,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsVXformAttach::GetVtxSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsVtxSrc);
}

UsdRelationship
UsdPhysicsVXformAttach::CreateVtxSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsVtxSrc,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsVXformAttach::GetXformSrcRel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsXformSrc);
}

UsdRelationship
UsdPhysicsVXformAttach::CreateXformSrcRel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsXformSrc,
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
UsdPhysicsVXformAttach::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsAttachmentEnabled,
        UsdPhysicsTokens->physicsVtxIndices,
        UsdPhysicsTokens->physicsLocalPositions,
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
