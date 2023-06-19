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
#ifndef USDGEOM_GENERATED_BASISCURVESNETWORK_H
#define USDGEOM_GENERATED_BASISCURVESNETWORK_H

/// \file usdGeom/basisCurvesNetwork.h

#include "pxr/pxr.h"
#include "pxr/usd/usdGeom/api.h"
#include "pxr/usd/usdGeom/curves.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/tokens.h"

#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// BASISCURVESNETWORK                                                         //
// -------------------------------------------------------------------------- //

/// \class UsdGeomBasisCurvesNetwork
///
/// Encodes a set of curves which may have shared points.
/// This can be used to represent curves attached in a tree structure or with cycles (e.g. netting).
/// 
/// Each curve is defined by a set of curve-vertices using indices into the curve's points array (inherited from
/// PointBased) and laid out in a single flat curveVertexIndices array. A companion curveVertexCounts array
/// (inherited from Curves) provides, for each curve, the number of consecutive curve-vertices in
/// curveVertexIndices that define the curve.
/// No additional connectivity information is required or constructed, so no adjacency or neighborhood queries are
/// available.
/// 
/// Primvar interpolation is defined identically to \ref UsdGeomBasisCurves_PrimvarInterpolation.
/// 
///
/// For any described attribute \em Fallback \em Value or \em Allowed \em Values below
/// that are text/tokens, the actual token is published and defined in \ref UsdGeomTokens.
/// So to set an attribute to the value "rightHanded", use UsdGeomTokens->rightHanded
/// as the value.
///
class UsdGeomBasisCurvesNetwork : public UsdGeomCurves
{
public:
    /// Compile time constant representing what kind of schema this class is.
    ///
    /// \sa UsdSchemaKind
    static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

    /// Construct a UsdGeomBasisCurvesNetwork on UsdPrim \p prim .
    /// Equivalent to UsdGeomBasisCurvesNetwork::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit UsdGeomBasisCurvesNetwork(const UsdPrim& prim=UsdPrim())
        : UsdGeomCurves(prim)
    {
    }

    /// Construct a UsdGeomBasisCurvesNetwork on the prim held by \p schemaObj .
    /// Should be preferred over UsdGeomBasisCurvesNetwork(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit UsdGeomBasisCurvesNetwork(const UsdSchemaBase& schemaObj)
        : UsdGeomCurves(schemaObj)
    {
    }

    /// Destructor.
    USDGEOM_API
    virtual ~UsdGeomBasisCurvesNetwork();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    USDGEOM_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a UsdGeomBasisCurvesNetwork holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// UsdGeomBasisCurvesNetwork(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    USDGEOM_API
    static UsdGeomBasisCurvesNetwork
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
    USDGEOM_API
    static UsdGeomBasisCurvesNetwork
    Define(const UsdStagePtr &stage, const SdfPath &path);

protected:
    /// Returns the kind of schema this class belongs to.
    ///
    /// \sa UsdSchemaKind
    USDGEOM_API
    UsdSchemaKind _GetSchemaKind() const override;

private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    USDGEOM_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    USDGEOM_API
    const TfType &_GetTfType() const override;

public:
    // --------------------------------------------------------------------- //
    // CURVEVERTEXINDICES 
    // --------------------------------------------------------------------- //
    /// Flat list of the index into the points attribute of each vertex of each curve.
    /// If this attribute has more than one time sample, the curves are considered to be topologically varying.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `int[] curveVertexIndices` |
    /// | C++ Type | VtArray<int> |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->IntArray |
    USDGEOM_API
    UsdAttribute GetCurveVertexIndicesAttr() const;

    /// See GetCurveVertexIndicesAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDGEOM_API
    UsdAttribute CreateCurveVertexIndicesAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // TYPE 
    // --------------------------------------------------------------------- //
    /// Linear curves interpolate linearly between two vertices.
    /// Cubic curves use a basis matrix with four vertices to interpolate a segment.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token type = "cubic"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdGeomTokens "Allowed Values" | linear, cubic |
    USDGEOM_API
    UsdAttribute GetTypeAttr() const;

    /// See GetTypeAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDGEOM_API
    UsdAttribute CreateTypeAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // BASIS 
    // --------------------------------------------------------------------- //
    /// The basis specifies the vstep and matrix used for cubic
    /// interpolation.
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token basis = "bezier"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdGeomTokens "Allowed Values" | bezier, bspline, catmullRom |
    USDGEOM_API
    UsdAttribute GetBasisAttr() const;

    /// See GetBasisAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDGEOM_API
    UsdAttribute CreateBasisAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

public:
    // --------------------------------------------------------------------- //
    // WRAP 
    // --------------------------------------------------------------------- //
    /// If wrap is set to periodic, the curve when rendered will
    /// repeat the initial vertices (dependent on the vstep) to close the
    /// curve. If wrap is set to 'pinned', phantom points may be created
    /// to ensure that the curve interpolation starts at P[0] and ends at P[n-1].
    /// 
    ///
    /// | ||
    /// | -- | -- |
    /// | Declaration | `uniform token wrap = "nonperiodic"` |
    /// | C++ Type | TfToken |
    /// | \ref Usd_Datatypes "Usd Type" | SdfValueTypeNames->Token |
    /// | \ref SdfVariability "Variability" | SdfVariabilityUniform |
    /// | \ref UsdGeomTokens "Allowed Values" | nonperiodic, periodic, pinned |
    USDGEOM_API
    UsdAttribute GetWrapAttr() const;

    /// See GetWrapAttr(), and also 
    /// \ref Usd_Create_Or_Get_Property for when to use Get vs Create.
    /// If specified, author \p defaultValue as the attribute's default,
    /// sparsely (when it makes sense to do so) if \p writeSparsely is \c true -
    /// the default for \p writeSparsely is \c false.
    USDGEOM_API
    UsdAttribute CreateWrapAttr(VtValue const &defaultValue = VtValue(), bool writeSparsely=false) const;

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
