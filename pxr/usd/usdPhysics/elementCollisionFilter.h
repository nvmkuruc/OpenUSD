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
#ifndef USDPHYSICS_GENERATED_ELEMENTCOLLISIONFILTER_H
#define USDPHYSICS_GENERATED_ELEMENTCOLLISIONFILTER_H

/// \file usdPhysics/elementCollisionFilter.h

#include "pxr/pxr.h"
#include "pxr/usd/usdPhysics/api.h"
#include "pxr/usd/usdGeom/imageable.h"
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
// PHYSICSELEMENTCOLLISIONFILTER                                              //
// -------------------------------------------------------------------------- //

/// \class UsdPhysicsElementCollisionFilter
///
/// Describes a collision filter based on vertices, segments or edges and triangles
/// for two suitable GPrims which are the source of the elements. At least one source primitive needs to be a deformable body's simulation mesh, and one can be either a rigid body or collider.
/// 
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdPhysicsTokens.
/// So to set an attribute to the value "rightHanded", use UsdPhysicsTokens->rightHanded
/// as the value.
///
class UsdPhysicsElementCollisionFilter : public UsdGeomImageable
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

    /// Construct a UsdPhysicsElementCollisionFilter on UsdPrim \p prim .
    /// Equivalent to UsdPhysicsElementCollisionFilter::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdPhysicsElementCollisionFilter(const UsdPrim& prim=UsdPrim())
        : UsdGeomImageable(prim)
    {
    }

    /// Construct a UsdPhysicsElementCollisionFilter on the prim held by \p schemaObj .
    /// Should be preferred over UsdPhysicsElementCollisionFilter(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdPhysicsElementCollisionFilter(const UsdSchemaBase& schemaObj)
        : UsdGeomImageable(schemaObj)
    {
    }

    /// Destructor.
    USDPHYSICS_API
    virtual ~UsdPhysicsElementCollisionFilter();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDPHYSICS_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdPhysicsElementCollisionFilter holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdPhysicsElementCollisionFilter(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDPHYSICS_API
    static UsdPhysicsElementCollisionFilter
    Get(const UsdStagePtr &stage, const SdfPath &path);

    /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
    /// is defined (according to UsdPrim::IsDefined()) on this stage.
    ///
    /// If a prim adhering to this schema at \p path is already defined on this
    /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
    /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
    /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
    /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
    /// current EditTarget for any nonexistent, or existing but not \a Defined
    /// ancestors.
    ///
    /// The given \a path must be an absolute prim path that does not contain
    /// any variant selections.
    ///
    /// If it is impossible to author any of the necessary PrimSpecs, (for
    /// example, in case \a path cannot map to the current UsdEditTarget's
    /// namespace) issue an error and return an invalid \a UsdPrim.
    ///
    /// Note that this method may return a defined prim whose typeName does not
    /// specify this schema class, in case a stronger typeName opinion overrides
    /// the opinion at the current EditTarget.
    ///
    USDPHYSICS_API
    static UsdPhysicsElementCollisionFilter
    Define(const UsdStagePtr &stage, const SdfPath &path);

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
    // FILTERENABLED 
    // --------------------------------------------------------------------- //
    /// Determines if this PhysicsElementCollisionFilter is enabled.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `bool physics:filterEnabled = 1` |
    /// | C++ Type | bool |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Bool |
    USDPHYSICS_API
    UsdAttribute GetFilterEnabledAttr() const;

    /// See GetFilterEnabledAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreateFilterEnabledAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // FILTERTYPE 
    // --------------------------------------------------------------------- //
    /// Describes how elements in elemIndices0 and elemIndices1 are combined to define element pairs that are filtered. The type "pairwise" indicates that each Nth entry in elemIndices0 pairs up with the Nth entry in elemIndices1. The type "group" indicates that each entry in elemIndices0 pairs up with every entry in elemIndices1.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token physics:filterType = "group"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdPhysicsTokens "Allowed Values" | pairwise, group |
    USDPHYSICS_API
    UsdAttribute GetFilterTypeAttr() const;

    /// See GetFilterTypeAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreateFilterTypeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // ELEMTYPE0 
    // --------------------------------------------------------------------- //
    /// Defines the element type in elemIndices0. Type "vtx" refers to deformable body vertices. Type "seg" refers to deformable curve segments or deformable triangle edges, each described by two indices. Type "tri" refers to deformable triangular faces (surface triangles in the case of volumetric deformable bodies). Type "all" refers to the whole geometry of src0, in which case elemIndices0 is assumed to be empty.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token physics:elemType0 = "vtx"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdPhysicsTokens "Allowed Values" | vtx, seg, tri, all |
    USDPHYSICS_API
    UsdAttribute GetElemType0Attr() const;

    /// See GetElemType0Attr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreateElemType0Attr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // ELEMTYPE1 
    // --------------------------------------------------------------------- //
    /// Defines the element type in elemIndices1. Type "vtx" refers to deformable body vertices. Type "seg" refers to either deformable curve segments or deformable triangle edges. Type "tri" refers to deformable triangular faces (surface triangles in the case of volumetric deformable bodies). Type "all" refers to the whole geometry of src1.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token physics:elemType1 = "vtx"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdPhysicsTokens "Allowed Values" | vtx, seg, tri, all |
    USDPHYSICS_API
    UsdAttribute GetElemType1Attr() const;

    /// See GetElemType1Attr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreateElemType1Attr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // PHYSICSELEMINDICES0 
    // --------------------------------------------------------------------- //
    /// Array of element indices of type elemType0 from src0. Type "seg" implies that each entry consists of two indices - (curve index, segment index) or (vertex index, vertex index) for triangle edges. Type "all" implies an empty list.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `int[] physics:elemIndices0` |
    /// | C++ Type | VtArray<int> |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->IntArray |
    USDPHYSICS_API
    UsdAttribute GetPhysicsElemIndices0Attr() const;

    /// See GetPhysicsElemIndices0Attr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreatePhysicsElemIndices0Attr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // PHYSICSELEMINDICES1 
    // --------------------------------------------------------------------- //
    /// Array of element indices of type elemType1 from src1. Type "seg" implies that each entry consists of two indices - (curve index, segment index) or (vertex index, vertex index) for triangle edges. Type "all" implies an empty list.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `int[] physics:elemIndices1` |
    /// | C++ Type | VtArray<int> |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->IntArray |
    USDPHYSICS_API
    UsdAttribute GetPhysicsElemIndices1Attr() const;

    /// See GetPhysicsElemIndices1Attr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDPHYSICS_API
    UsdAttribute CreatePhysicsElemIndices1Attr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // SRC0 
    // --------------------------------------------------------------------- //
    /// Relationship to a single deformable body's simulation mesh which is the source of the elements described by elemType0 and elemIndices0 or a rigid body/collider.
    ///
    USDPHYSICS_API
    UsdRelationship GetSrc0Rel() const;

    /// See GetSrc0Rel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDPHYSICS_API
    UsdRelationship CreateSrc0Rel() const;

public:
    // --------------------------------------------------------------------- //
    // SRC1 
    // --------------------------------------------------------------------- //
    /// Relationship to a single deformable body's simulation mesh which is the source of the elements described by elemType1 and elemIndices1 or a rigid body/collider.
    ///
    USDPHYSICS_API
    UsdRelationship GetSrc1Rel() const;

    /// See GetSrc1Rel(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create
    USDPHYSICS_API
    UsdRelationship CreateSrc1Rel() const;

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
