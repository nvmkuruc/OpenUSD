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
#include "pxr/usd/usdPhysics/elementCollisionFilter.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsElementCollisionFilter,
        TfType::Bases< UsdGeomImageable > >();
    
    // Register the usd prim typename as an alias under UsdSchemaBase. This
    // enables one to call
    // TfType::Find<UsdSchemaBase>().FindDerivedByName("PhysicsElementCollisionFilter")
    // to find TfType<UsdPhysicsElementCollisionFilter>, which is how IsA queries are
    // answered.
    TfType::AddAlias<UsdSchemaBase, UsdPhysicsElementCollisionFilter>("PhysicsElementCollisionFilter");
}

/* virtual */
UsdPhysicsElementCollisionFilter::~UsdPhysicsElementCollisionFilter()
{
}

/* static */
UsdPhysicsElementCollisionFilter
UsdPhysicsElementCollisionFilter::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsElementCollisionFilter();
    }
    return UsdPhysicsElementCollisionFilter(stage->GetPrimAtPath(path));
}

/* static */
UsdPhysicsElementCollisionFilter
UsdPhysicsElementCollisionFilter::Define(
    const UsdStagePtr &stage, const SdfPath &path)
{
    static TfToken usdPrimTypeName("PhysicsElementCollisionFilter");
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsElementCollisionFilter();
    }
    return UsdPhysicsElementCollisionFilter(
        stage->DefinePrim(path, usdPrimTypeName));
}

/* virtual */
UsdSchemaKind UsdPhysicsElementCollisionFilter::_GetSchemaKind() const
{
    return UsdPhysicsElementCollisionFilter::schemaKind;
}

/* static */
const TfType &
UsdPhysicsElementCollisionFilter::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsElementCollisionFilter>();
    return tfType;
}

/* static */
bool 
UsdPhysicsElementCollisionFilter::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsElementCollisionFilter::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetFilterEnabledAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsFilterEnabled);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreateFilterEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsFilterEnabled,
                       SdfValueTypeNames->Bool,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetFilterTypeAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsFilterType);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreateFilterTypeAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsFilterType,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetElemType0Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsElemType0);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreateElemType0Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsElemType0,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetElemType1Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsElemType1);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreateElemType1Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsElemType1,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetPhysicsElemIndices0Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsElemIndices0);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreatePhysicsElemIndices0Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsElemIndices0,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::GetPhysicsElemIndices1Attr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsElemIndices1);
}

UsdAttribute
UsdPhysicsElementCollisionFilter::CreatePhysicsElemIndices1Attr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsElemIndices1,
                       SdfValueTypeNames->IntArray,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdRelationship
UsdPhysicsElementCollisionFilter::GetSrc0Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsSrc0);
}

UsdRelationship
UsdPhysicsElementCollisionFilter::CreateSrc0Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsSrc0,
                       /* custom = */ false);
}

UsdRelationship
UsdPhysicsElementCollisionFilter::GetSrc1Rel() const
{
    return GetPrim().GetRelationship(UsdPhysicsTokens->physicsSrc1);
}

UsdRelationship
UsdPhysicsElementCollisionFilter::CreateSrc1Rel() const
{
    return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsSrc1,
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
UsdPhysicsElementCollisionFilter::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsFilterEnabled,
        UsdPhysicsTokens->physicsFilterType,
        UsdPhysicsTokens->physicsElemType0,
        UsdPhysicsTokens->physicsElemType1,
        UsdPhysicsTokens->physicsElemIndices0,
        UsdPhysicsTokens->physicsElemIndices1,
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
