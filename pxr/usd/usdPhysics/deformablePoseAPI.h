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
#ifndef USDPHYSICS_GENERATED_DEFORMABLEPOSEAPI_H
#define USDPHYSICS_GENERATED_DEFORMABLEPOSEAPI_H

/// \file usdPhysics/deformablePoseAPI.h

#include "pxr/pxr.h"
#include "pxr/usd/usdPhysics/api.h"
#include "pxr/usd/usd/apiSchemaBase.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdPhysics/tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// PHYSICSDEFORMABLEPOSEAPI                                                   //
// -------------------------------------------------------------------------- //

/// \class UsdPhysicsDeformablePoseAPI
///
/// Defines a deformable mesh pose which can be used for one or multiple 
/// purposes. Multiple pose APIs can be specified for different purposes.
///
class UsdPhysicsDeformablePoseAPI : public UsdAPISchemaBase
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::MultipleApplyAPI;

    /// Construct a UsdPhysicsDeformablePoseAPI on UsdPrim \p prim with
    /// name \p name . Equivalent to
    /// UsdPhysicsDeformablePoseAPI::Get(
    ///    prim.GetStage(),
    ///    prim.GetPath().AppendProperty(
    ///        "pose:name"));
    ///
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdPhysicsDeformablePoseAPI(
        const UsdPrim& prim=UsdPrim(), const TfToken &name=TfToken())
        : UsdAPISchemaBase(prim, /*instanceName*/ name)
    { }

    /// Construct a UsdPhysicsDeformablePoseAPI on the prim held by \p schemaObj with
    /// name \p name.  Should be preferred over
    /// UsdPhysicsDeformablePoseAPI(schemaObj.GetPrim(), name), as it preserves
    /// SchemaBase state.
    explicit UsdPhysicsDeformablePoseAPI(
        const UsdSchemaBase& schemaObj, const TfToken &name)
        : UsdAPISchemaBase(schemaObj, /*instanceName*/ name)
    { }

    /// Destructor.
    USDPHYSICS_API
    virtual ~UsdPhysicsDeformablePoseAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDPHYSICS_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes for a given instance name.  Does not
    /// include attributes that may be authored by custom/extended methods of
    /// the schemas involved. The names returned will have the proper namespace
    /// prefix.
    USDPHYSICS_API
    static TfTokenVector
    GetSchemaAttributeNames(bool includeInherited, const TfToken &instanceName);

    /// Returns the name of this multiple-apply schema instance
    TfToken GetName() const {
        return _GetInstanceName();
    }

    /// Return a UsdPhysicsDeformablePoseAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  \p path must be of the format
    /// <path>.pose:name .
    ///
    /// This is shorthand for the following:
    ///
    /// \code
    /// TfToken name = SdfPath::StripNamespace(path.GetToken());
    /// UsdPhysicsDeformablePoseAPI(
    ///     stage->GetPrimAtPath(path.GetPrimPath()), name);
    /// \endcode
    ///
    USDPHYSICS_API
    static UsdPhysicsDeformablePoseAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);

    /// Return a UsdPhysicsDeformablePoseAPI with name \p name holding the
    /// prim \p prim. Shorthand for UsdPhysicsDeformablePoseAPI(prim, name);
    USDPHYSICS_API
    static UsdPhysicsDeformablePoseAPI
    Get(const UsdPrim &prim, const TfToken &name);

    /// Return a vector of all named instances of UsdPhysicsDeformablePoseAPI on the 
    /// given \p prim.
    USDPHYSICS_API
    static std::vector<UsdPhysicsDeformablePoseAPI>
    GetAll(const UsdPrim &prim);

    /// Checks if the given name \p baseName is the base name of a property
    /// of PhysicsDeformablePoseAPI.
    USDPHYSICS_API
    static bool
    IsSchemaPropertyBaseName(const TfToken &baseName);

    /// Checks if the given path \p path is of an API schema of type
    /// PhysicsDeformablePoseAPI. If so, it stores the instance name of
    /// the schema in \p name and returns true. Otherwise, it returns false.
    USDPHYSICS_API
    static bool
    IsPhysicsDeformablePoseAPIPath(const SdfPath &path, TfToken *name);

    /// Returns true if this <b>multiple-apply</b> API schema can be applied,
    /// with the given instance name, \p name, to the given \p prim. If this 
    /// schema can not be a applied the prim, this returns false and, if 
    /// provided, populates \p whyNot with the reason it can not be applied.
    /// 
    /// Note that if CanApply returns false, that does not necessarily imply
    /// that calling Apply will fail. Callers are expected to call CanApply
    /// before calling Apply if they want to ensure that it is valid to 
    /// apply a schema.
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    /// \sa UsdPrim::CanApplyAPI()
    /// \sa UsdPrim::ApplyAPI()
    /// \sa UsdPrim::RemoveAPI()
    ///
    USDPHYSICS_API
    static bool 
    CanApply(const UsdPrim &prim, const TfToken &name, 
             std::string *whyNot=nullptr);

    /// Applies this <b>multiple-apply</b> API schema to the given \p prim 
    /// along with the given instance name, \p name. 
    /// 
    /// This information is stored by adding "PhysicsDeformablePoseAPI:<i>name</i>" 
    /// to the token-valued, listOp metadata \em apiSchemas on the prim.
    /// For example, if \p name is 'instance1', the token 
    /// 'PhysicsDeformablePoseAPI:instance1' is added to 'apiSchemas'.
    /// 
    /// \return A valid UsdPhysicsDeformablePoseAPI object is returned upon success. 
    /// An invalid (or empty) UsdPhysicsDeformablePoseAPI object is returned upon 
    /// failure. See \ref UsdPrim::ApplyAPI() for 
    /// conditions resulting in failure. 
    /// 
    /// \sa UsdPrim::GetAppliedSchemas()
    /// \sa UsdPrim::HasAPI()
    /// \sa UsdPrim::CanApplyAPI()
    /// \sa UsdPrim::ApplyAPI()
    /// \sa UsdPrim::RemoveAPI()
    ///
    USDPHYSICS_API
    static UsdPhysicsDeformablePoseAPI 
    Apply(const UsdPrim &prim, const TfToken &name);

protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDPHYSICS_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDPHYSICS_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDPHYSICS_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // PHYSICSPURPOSES 
    // --------------------------------------------------------------------- //
    /// The set of purposes this pose is used for.
    /// "bindPose": The pose in which graphical UsdGeomPointBased and
    /// deformable simulation meshes are bound to each other. The purpose
    /// and pose needs to be specified on both graphical and simulation 
    /// meshes.
    /// "selfCollisionFilterPose": The pose used for self collision 
    /// filtering. Ovelapping domains of the mesh in this pose are excluded 
    /// from self collision.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `token[] physics:purposes` |
    /// | C++ Type | VtArray<TfToken> |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->TokenArray |
    /// | \ref UsdPhysicsTokens "Allowed Values" | bindPose, selfCollisionFilterPose |
    USDPHYSICS_API
    UsdAttribute GetPhysicsPurposesAttr() const;

    /// See GetPhysicsPurposesAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreatePhysicsPurposesAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // PHYSICSPOSEPOINTS 
    // --------------------------------------------------------------------- //
    /// The points defining the pose of a mesh. The number of
    /// points needs to match the number of points in the mesh.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `point3f[] physics:posePoints` |
    /// | C++ Type | VtArray<GfVec3f> |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Point3fArray |
    USDPHYSICS_API
    UsdAttribute GetPhysicsPosePointsAttr() const;

    /// See GetPhysicsPosePointsAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreatePhysicsPosePointsAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
