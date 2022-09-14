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
#include "./mdlAPI.h"
#include "pxr/usd/usd/schemaRegistry.h"
#include "pxr/usd/usd/typed.h"
#include "pxr/usd/usd/tokens.h"

#include "pxr/usd/sdf/types.h"
#include "pxr/usd/sdf/assetPath.h"

PXR_NAMESPACE_OPEN_SCOPE

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdMdlMdlAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (MdlAPI)
);

/* virtual */
UsdMdlMdlAPI::~UsdMdlMdlAPI()
{
}

/* static */
UsdMdlMdlAPI
UsdMdlMdlAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdMdlMdlAPI();
    }
    return UsdMdlMdlAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaType UsdMdlMdlAPI::_GetSchemaType() const {
    return UsdMdlMdlAPI::schemaType;
}

/* static */
UsdMdlMdlAPI
UsdMdlMdlAPI::Apply(const UsdPrim &prim)
{
    return UsdAPISchemaBase::_ApplyAPISchema<UsdMdlMdlAPI>(
            prim, _schemaTokens->MdlAPI);
}

/* static */
const TfType &
UsdMdlMdlAPI::_GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdMdlMdlAPI>();
    return tfType;
}

/* static */
bool 
UsdMdlMdlAPI::_IsTypedSchema()
{
    static bool isTyped = _GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdMdlMdlAPI::_GetTfType() const
{
    return _GetStaticTfType();
}

UsdAttribute
UsdMdlMdlAPI::GetInfoMdlSourceAssetAttr() const
{
    return GetPrim().GetAttribute(UsdMdlTokens->infoMdlSourceAsset);
}

UsdAttribute
UsdMdlMdlAPI::CreateInfoMdlSourceAssetAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdMdlTokens->infoMdlSourceAsset,
                       SdfValueTypeNames->Asset,
                       /* custom = */ false,
                       SdfVariabilityUniform,
                       defaultValue,
                       writeSparsely);
}

UsdAttribute
UsdMdlMdlAPI::GetInfoMdlSourceAssetSubIdentifierAttr() const
{
    return GetPrim().GetAttribute(UsdMdlTokens->infoMdlSourceAssetSubIdentifier);
}

UsdAttribute
UsdMdlMdlAPI::CreateInfoMdlSourceAssetSubIdentifierAttr(VtValue const &defaultValue, bool writeSparsely) const
{
    return UsdSchemaBase::_CreateAttr(UsdMdlTokens->infoMdlSourceAssetSubIdentifier,
                       SdfValueTypeNames->Token,
                       /* custom = */ false,
                       SdfVariabilityUniform,
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
UsdMdlMdlAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames = {
        UsdMdlTokens->infoMdlSourceAsset,
        UsdMdlTokens->infoMdlSourceAssetSubIdentifier,
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
