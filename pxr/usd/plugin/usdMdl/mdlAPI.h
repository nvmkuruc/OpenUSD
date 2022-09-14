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
#ifndef USDMDL_GENERATED_MDLAPI_H
#define USDMDL_GENERATED_MDLAPI_H

/// \file usdMdl/mdlAPI.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdShade/material.h"
#include "./tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// MDLAPI                                                                     //
// -------------------------------------------------------------------------- //

/// \class UsdMdlMdlAPI
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdMdlTokens.
/// So to set an attribute to the value "rightHanded", use UsdMdlTokens->rightHanded
/// as the value.
///
class UsdMdlMdlAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaType
    static const UsdSchemaType schemaType = UsdSchemaType::SingleApplyAPI;

    /// Construct a UsdMdlMdlAPI on UsdPrim \p prim .
    /// Equivalent to UsdMdlMdlAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdMdlMdlAPI(const UsdPrim& prim=UsdPrim())
        : UsdAPISchemaBase(prim)
    {
    }

    /// Construct a UsdMdlMdlAPI on the prim held by \p schemaObj .
    /// Should be preferred over UsdMdlMdlAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdMdlMdlAPI(const UsdSchemaBase& schemaObj)
        : UsdAPISchemaBase(schemaObj)
    {
    }

    /// Destructor.
    USDMDL_API
    virtual ~UsdMdlMdlAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDMDL_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdMdlMdlAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdMdlMdlAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDMDL_API
    static UsdMdlMdlAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


    /// Applies this <b>single-apply</b> API schema to the given \p prim.
    /// This information is stored by adding "MdlAPI" to the 
    /// token-valued, listOp metadata \em apiSchemas on the prim.
    /// 
    /// \return A valid UsdMdlMdlAPI object is returned upon success. 
    /// An invalid (or empty) UsdMdlMdlAPI object is returned upon 
    /// failure. See \ref UsdAPISchemaBase::_ApplyAPISchema() for conditions 
    /// resulting in failure. 
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    ///
    USDMDL_API
    static UsdMdlMdlAPI 
    Apply(const UsdPrim &prim);

protected:
    /// Returns the type of schema this class belongs to.
    ///
    /// \sa UsdSchemaType
    USDMDL_API
    UsdSchemaType _GetSchemaType() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDMDL_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDMDL_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // INFOMDLSOURCEASSET 
    // --------------------------------------------------------------------- //
    /// Asset value denoting MDL module
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform asset info:mdl:sourceAsset` |
    /// | C++ Type | SdfAssetPath |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Asset |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    USDMDL_API
    UsdAttribute GetInfoMdlSourceAssetAttr() const;

    /// See GetInfoMdlSourceAssetAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDMDL_API
    UsdAttribute CreateInfoMdlSourceAssetAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // INFOMDLSOURCEASSETSUBIDENTIFIER 
    // --------------------------------------------------------------------- //
    /// Asset value denoting MDL material fully qualified name
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token info:mdl:sourceAsset:subIdentifier` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    USDMDL_API
    UsdAttribute GetInfoMdlSourceAssetSubIdentifierAttr() const;

    /// See GetInfoMdlSourceAssetSubIdentifierAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDMDL_API
    UsdAttribute CreateInfoMdlSourceAssetSubIdentifierAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--

    static std::vector<std::string> ApplyAndValidate(const UsdPrim &prim)
    {
        Apply(prim);
        return Validate(prim);
    }

    static std::vector<std::string> Validate(const UsdPrim& prim)
    {
        std::vector<std::string> errors;

        bool isShader = prim.IsA<UsdShadeShader>();
        bool isMaterial = prim.IsA<UsdShadeMaterial>();

        if (isMaterial)
        {
            // TODO
        }
        else if (isShader)
        {
            UsdShadeShader shader(prim);
            if (shader.GetImplementationSource() != UsdShadeTokens->sourceAsset)
            {
                std::ostringstream ss;
                ss << "[UsdMdl.MdlAPI] :" <<
                    prim.GetPath().GetString() << ": info:implementationSource should be set to \"sourceAsset\"";
                errors.push_back(ss.str());
            }
            
            UsdMdlMdlAPI MdlAPI(prim);
            UsdAttribute sourceAssetAttr(MdlAPI.GetInfoMdlSourceAssetAttr());
            if (!sourceAssetAttr.IsValid())
            {
                std::ostringstream ss;
                ss << "[UsdMdl.MdlAPI] :" <<
                    prim.GetPath().GetString() << ": info:mdl:sourceAsset attribute not set";
                errors.push_back(ss.str());
            }
            UsdAttribute sourceAssetSubIdAttr(MdlAPI.GetInfoMdlSourceAssetSubIdentifierAttr());
            if (!sourceAssetSubIdAttr.IsValid())
            {
                std::ostringstream ss;
                ss << "[UsdMdl.MdlAPI] :" <<
                    prim.GetPath().GetString() << ": info:mdl:sourceAsset:subIdentifier attribute not set";
                errors.push_back(ss.str());
            }
        }

        for (auto childPrim : prim.GetAllChildren())
        {
            ValidateChild(prim, childPrim, errors);
        }

        return errors;
    }

private:
    static void ValidateChild(const UsdPrim& scenePrim, const UsdPrim& childPrim, std::vector<std::string>& errors)
    {
        std::vector<std::string> newErrors = UsdMdlMdlAPI::Validate(childPrim);
        errors.insert(errors.end(), newErrors.begin(), newErrors.end());
    }
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
