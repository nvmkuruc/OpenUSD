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
#include "pxr/usd/usdPhysics/curvesDeformableMaterialAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/tokens.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsCurvesDeformableMaterialAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (PhysicsCurvesDeformableMaterialAPI)
);

/* virtual */
UsdPhysicsCurvesDeformableMaterialAPI::~UsdPhysicsCurvesDeformableMaterialAPI()
{
}

/* static */
UsdPhysicsCurvesDeformableMaterialAPI
UsdPhysicsCurvesDeformableMaterialAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsCurvesDeformableMaterialAPI();
    }
    return UsdPhysicsCurvesDeformableMaterialAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPhysicsCurvesDeformableMaterialAPI::_GetSchemaKind() const
{
    return UsdPhysicsCurvesDeformableMaterialAPI::schemaKind;
}

/* static */
bool
UsdPhysicsCurvesDeformableMaterialAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPhysicsCurvesDeformableMaterialAPI>(whyNot);
}

/* static */
UsdPhysicsCurvesDeformableMaterialAPI
UsdPhysicsCurvesDeformableMaterialAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPhysicsCurvesDeformableMaterialAPI>()) {
        return UsdPhysicsCurvesDeformableMaterialAPI(prim);
    }
    return UsdPhysicsCurvesDeformableMaterialAPI();
}

/* static */
const TfType &
UsdPhysicsCurvesDeformableMaterialAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsCurvesDeformableMaterialAPI>();
    return tfType;
}

/* static */
bool 
UsdPhysicsCurvesDeformableMaterialAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsCurvesDeformableMaterialAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::GetCurveThicknessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsCurveThickness);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::CreateCurveThicknessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsCurveThickness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::GetCurveStretchStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsCurveStretchStiffness);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::CreateCurveStretchStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsCurveStretchStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::GetCurveBendStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsCurveBendStiffness);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::CreateCurveBendStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsCurveBendStiffness,
                       SdfValueTypeNames->Float,
                       /* custom = */ false,
                       SdfVariabilityVarying,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::GetCurveTwistStiffnessAttr() const
{
    return GetPrim().GetAttribute(UsdPhysicsTokens->physicsCurveTwistStiffness);
}

UsdAttribute
UsdPhysicsCurvesDeformableMaterialAPI::CreateCurveTwistStiffnessAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsCurveTwistStiffness,
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
UsdPhysicsCurvesDeformableMaterialAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdPhysicsTokens->physicsCurveThickness,
        UsdPhysicsTokens->physicsCurveStretchStiffness,
        UsdPhysicsTokens->physicsCurveBendStiffness,
        UsdPhysicsTokens->physicsCurveTwistStiffness,
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
