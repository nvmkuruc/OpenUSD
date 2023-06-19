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
#ifndef USDPHYSICS_TOKENS_H
#define USDPHYSICS_TOKENS_H

/// \file usdPhysics/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "pxr/usd/usdPhysics/api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdPhysicsTokensType
///
/// \link UsdPhysicsTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdPhysicsTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdPhysicsTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdPhysicsTokens->acceleration);
/// \endcode
struct UsdPhysicsTokensType {
    USDPHYSICS_API UsdPhysicsTokensType();
    /// \brief "acceleration"
    /// 
    /// Possible value for UsdPhysicsDriveAPI::GetTypeAttr()
    const TfToken acceleration;
    /// \brief "all"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetElemType0Attr(), Possible value for UsdPhysicsElementCollisionFilter::GetElemType1Attr()
    const TfToken all;
    /// \brief "angular"
    /// 
    ///  This token represents the angular degree of freedom used in Revolute Joint Drive. 
    const TfToken angular;
    /// \brief "boundingCube"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken boundingCube;
    /// \brief "boundingSphere"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken boundingSphere;
    /// \brief "colliders"
    /// 
    ///  This token represents the collection name to use with UsdCollectionAPI to represent colliders of a CollisionGroup prim. 
    const TfToken colliders;
    /// \brief "convexDecomposition"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken convexDecomposition;
    /// \brief "convexHull"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken convexHull;
    /// \brief "distance"
    /// 
    ///  This token represents the distance limit used for generic D6 joint. 
    const TfToken distance;
    /// \brief "drive"
    /// 
    /// Property namespace prefix for the UsdPhysicsDriveAPI schema.
    const TfToken drive;
    /// \brief "drive:__INSTANCE_NAME__:physics:damping"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsDamping;
    /// \brief "drive:__INSTANCE_NAME__:physics:maxForce"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsMaxForce;
    /// \brief "drive:__INSTANCE_NAME__:physics:stiffness"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsStiffness;
    /// \brief "drive:__INSTANCE_NAME__:physics:targetPosition"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsTargetPosition;
    /// \brief "drive:__INSTANCE_NAME__:physics:targetVelocity"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsTargetVelocity;
    /// \brief "drive:__INSTANCE_NAME__:physics:type"
    /// 
    /// UsdPhysicsDriveAPI
    const TfToken drive_MultipleApplyTemplate_PhysicsType;
    /// \brief "force"
    /// 
    /// Possible value for UsdPhysicsDriveAPI::GetTypeAttr(), Default value for UsdPhysicsDriveAPI::GetTypeAttr()
    const TfToken force;
    /// \brief "group"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetFilterTypeAttr(), Default value for UsdPhysicsElementCollisionFilter::GetFilterTypeAttr()
    const TfToken group;
    /// \brief "kilogramsPerUnit"
    /// 
    /// Stage-level metadata that encodes a scene's linear unit of measure as kilograms per encoded unit.
    const TfToken kilogramsPerUnit;
    /// \brief "limit"
    /// 
    /// Property namespace prefix for the UsdPhysicsLimitAPI schema.
    const TfToken limit;
    /// \brief "limit:__INSTANCE_NAME__:physics:high"
    /// 
    /// UsdPhysicsLimitAPI
    const TfToken limit_MultipleApplyTemplate_PhysicsHigh;
    /// \brief "limit:__INSTANCE_NAME__:physics:low"
    /// 
    /// UsdPhysicsLimitAPI
    const TfToken limit_MultipleApplyTemplate_PhysicsLow;
    /// \brief "linear"
    /// 
    ///  This token represents the linear degree of freedom used in Prismatic Joint Drive. 
    const TfToken linear;
    /// \brief "meshSimplification"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken meshSimplification;
    /// \brief "none"
    /// 
    /// Possible value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr(), Default value for UsdPhysicsMeshCollisionAPI::GetApproximationAttr()
    const TfToken none;
    /// \brief "pairwise"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetFilterTypeAttr()
    const TfToken pairwise;
    /// \brief "physics:angularVelocity"
    /// 
    /// UsdPhysicsRigidBodyAPI
    const TfToken physicsAngularVelocity;
    /// \brief "physics:approximation"
    /// 
    /// UsdPhysicsMeshCollisionAPI
    const TfToken physicsApproximation;
    /// \brief "physics:attachmentEnabled"
    /// 
    /// UsdPhysicsVXformAttach, UsdPhysicsVCurveAttach, UsdPhysicsVTetAttach, UsdPhysicsVTriAttach, UsdPhysicsVtxVtxAttach
    const TfToken physicsAttachmentEnabled;
    /// \brief "physics:axis"
    /// 
    /// UsdPhysicsSphericalJoint, UsdPhysicsPrismaticJoint, UsdPhysicsRevoluteJoint
    const TfToken physicsAxis;
    /// \brief "physics:baryCoords"
    /// 
    /// UsdPhysicsVTetAttach
    const TfToken physicsBaryCoords;
    /// \brief "physics:body0"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsBody0;
    /// \brief "physics:body1"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsBody1;
    /// \brief "physics:bodyEnabled"
    /// 
    /// UsdPhysicsBodyAPI
    const TfToken physicsBodyEnabled;
    /// \brief "physics:breakForce"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsBreakForce;
    /// \brief "physics:breakTorque"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsBreakTorque;
    /// \brief "physics:centerOfMass"
    /// 
    /// UsdPhysicsMassAPI
    const TfToken physicsCenterOfMass;
    /// \brief "physics:collisionEnabled"
    /// 
    /// UsdPhysicsJoint, UsdPhysicsCollisionAPI
    const TfToken physicsCollisionEnabled;
    /// \brief "physics:coneAngle0Limit"
    /// 
    /// UsdPhysicsSphericalJoint
    const TfToken physicsConeAngle0Limit;
    /// \brief "physics:coneAngle1Limit"
    /// 
    /// UsdPhysicsSphericalJoint
    const TfToken physicsConeAngle1Limit;
    /// \brief "physics:curveBendStiffness"
    /// 
    /// UsdPhysicsCurvesDeformableMaterialAPI
    const TfToken physicsCurveBendStiffness;
    /// \brief "physics:curveSegmentCoords"
    /// 
    /// UsdPhysicsVCurveAttach
    const TfToken physicsCurveSegmentCoords;
    /// \brief "physics:curveSegmentIndices"
    /// 
    /// UsdPhysicsVCurveAttach
    const TfToken physicsCurveSegmentIndices;
    /// \brief "physics:curveSrc"
    /// 
    /// UsdPhysicsVCurveAttach
    const TfToken physicsCurveSrc;
    /// \brief "physics:curveStretchStiffness"
    /// 
    /// UsdPhysicsCurvesDeformableMaterialAPI
    const TfToken physicsCurveStretchStiffness;
    /// \brief "physics:curveThickness"
    /// 
    /// UsdPhysicsCurvesDeformableMaterialAPI
    const TfToken physicsCurveThickness;
    /// \brief "physics:curveTwistStiffness"
    /// 
    /// UsdPhysicsCurvesDeformableMaterialAPI
    const TfToken physicsCurveTwistStiffness;
    /// \brief "physics:damping"
    /// 
    /// UsdPhysicsVXformAttach, UsdPhysicsVCurveAttach, UsdPhysicsVTetAttach, UsdPhysicsVTriAttach, UsdPhysicsVtxVtxAttach
    const TfToken physicsDamping;
    /// \brief "physics:density"
    /// 
    /// UsdPhysicsMaterialAPI, UsdPhysicsMassAPI
    const TfToken physicsDensity;
    /// \brief "physics:diagonalInertia"
    /// 
    /// UsdPhysicsMassAPI
    const TfToken physicsDiagonalInertia;
    /// \brief "physics:dynamicFriction"
    /// 
    /// UsdPhysicsMaterialAPI
    const TfToken physicsDynamicFriction;
    /// \brief "physics:edgeBendAngles"
    /// 
    /// UsdPhysicsSurfaceDeformableRestShapeAPI
    const TfToken physicsEdgeBendAngles;
    /// \brief "physics:edgeVertPairs"
    /// 
    /// UsdPhysicsSurfaceDeformableRestShapeAPI
    const TfToken physicsEdgeVertPairs;
    /// \brief "physics:elemIndices0"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsElemIndices0;
    /// \brief "physics:elemIndices1"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsElemIndices1;
    /// \brief "physics:elemType0"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsElemType0;
    /// \brief "physics:elemType1"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsElemType1;
    /// \brief "physics:excludeFromArticulation"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsExcludeFromArticulation;
    /// \brief "physics:faceCoords"
    /// 
    /// UsdPhysicsVTriAttach
    const TfToken physicsFaceCoords;
    /// \brief "physics:filteredGroups"
    /// 
    /// UsdPhysicsCollisionGroup
    const TfToken physicsFilteredGroups;
    /// \brief "physics:filteredPairs"
    /// 
    /// UsdPhysicsFilteredPairsAPI
    const TfToken physicsFilteredPairs;
    /// \brief "physics:filterEnabled"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsFilterEnabled;
    /// \brief "physics:filterType"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsFilterType;
    /// \brief "physics:gravityDirection"
    /// 
    /// UsdPhysicsScene
    const TfToken physicsGravityDirection;
    /// \brief "physics:gravityMagnitude"
    /// 
    /// UsdPhysicsScene
    const TfToken physicsGravityMagnitude;
    /// \brief "physics:invertFilteredGroups"
    /// 
    /// UsdPhysicsCollisionGroup
    const TfToken physicsInvertFilteredGroups;
    /// \brief "physics:jointEnabled"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsJointEnabled;
    /// \brief "physics:kinematicEnabled"
    /// 
    /// UsdPhysicsBodyAPI
    const TfToken physicsKinematicEnabled;
    /// \brief "physics:localPos0"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsLocalPos0;
    /// \brief "physics:localPos1"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsLocalPos1;
    /// \brief "physics:localPositions"
    /// 
    /// UsdPhysicsVXformAttach
    const TfToken physicsLocalPositions;
    /// \brief "physics:localRot0"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsLocalRot0;
    /// \brief "physics:localRot1"
    /// 
    /// UsdPhysicsJoint
    const TfToken physicsLocalRot1;
    /// \brief "physics:lowerLimit"
    /// 
    /// UsdPhysicsPrismaticJoint, UsdPhysicsRevoluteJoint
    const TfToken physicsLowerLimit;
    /// \brief "physics:mass"
    /// 
    /// UsdPhysicsMassAPI, UsdPhysicsDeformableBodyAPI
    const TfToken physicsMass;
    /// \brief "physics:maxDistance"
    /// 
    /// UsdPhysicsDistanceJoint
    const TfToken physicsMaxDistance;
    /// \brief "physics:mergeGroup"
    /// 
    /// UsdPhysicsCollisionGroup
    const TfToken physicsMergeGroup;
    /// \brief "physics:minDistance"
    /// 
    /// UsdPhysicsDistanceJoint
    const TfToken physicsMinDistance;
    /// \brief "physics:poissonsRatio"
    /// 
    /// UsdPhysicsDeformableMaterialAPI
    const TfToken physicsPoissonsRatio;
    /// \brief "physics:principalAxes"
    /// 
    /// UsdPhysicsMassAPI
    const TfToken physicsPrincipalAxes;
    /// \brief "physics:restCurveVertexIndices"
    /// 
    /// UsdPhysicsCurveDeformableRestShapeAPI
    const TfToken physicsRestCurveVertexIndices;
    /// \brief "physics:restFaceIndices"
    /// 
    /// UsdPhysicsSurfaceDeformableRestShapeAPI
    const TfToken physicsRestFaceIndices;
    /// \brief "physics:restitution"
    /// 
    /// UsdPhysicsRigidMaterialAPI
    const TfToken physicsRestitution;
    /// \brief "physics:restNormals"
    /// 
    /// UsdPhysicsCurveDeformableRestShapeAPI
    const TfToken physicsRestNormals;
    /// \brief "physics:restPoints"
    /// 
    /// UsdPhysicsCurveDeformableRestShapeAPI
    const TfToken physicsRestPoints;
    /// \brief "physics:restShapePoints"
    /// 
    /// UsdPhysicsSurfaceDeformableRestShapeAPI, UsdPhysicsDeformableRestShapeAPI
    const TfToken physicsRestShapePoints;
    /// \brief "physics:restTetIndices"
    /// 
    /// UsdPhysicsDeformableRestShapeAPI
    const TfToken physicsRestTetIndices;
    /// \brief "physics:simulationOwner"
    /// 
    /// UsdPhysicsCollisionAPI, UsdPhysicsBodyAPI
    const TfToken physicsSimulationOwner;
    /// \brief "physics:src0"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsSrc0;
    /// \brief "physics:src1"
    /// 
    /// UsdPhysicsElementCollisionFilter
    const TfToken physicsSrc1;
    /// \brief "physics:startsAsleep"
    /// 
    /// UsdPhysicsBodyAPI
    const TfToken physicsStartsAsleep;
    /// \brief "physics:staticFriction"
    /// 
    /// UsdPhysicsMaterialAPI
    const TfToken physicsStaticFriction;
    /// \brief "physics:stiffness"
    /// 
    /// UsdPhysicsVXformAttach, UsdPhysicsVCurveAttach, UsdPhysicsVTetAttach, UsdPhysicsVTriAttach, UsdPhysicsVtxVtxAttach
    const TfToken physicsStiffness;
    /// \brief "physics:surfaceBendStiffness"
    /// 
    /// UsdPhysicsSurfaceDeformableMaterialAPI
    const TfToken physicsSurfaceBendStiffness;
    /// \brief "physics:surfaceShearStiffness"
    /// 
    /// UsdPhysicsSurfaceDeformableMaterialAPI
    const TfToken physicsSurfaceShearStiffness;
    /// \brief "physics:surfaceStretchStiffness"
    /// 
    /// UsdPhysicsSurfaceDeformableMaterialAPI
    const TfToken physicsSurfaceStretchStiffness;
    /// \brief "physics:surfaceThickness"
    /// 
    /// UsdPhysicsSurfaceDeformableMaterialAPI
    const TfToken physicsSurfaceThickness;
    /// \brief "physics:tetIndices"
    /// 
    /// UsdPhysicsVTetAttach
    const TfToken physicsTetIndices;
    /// \brief "physics:tetSrc"
    /// 
    /// UsdPhysicsVTetAttach
    const TfToken physicsTetSrc;
    /// \brief "physics:triIndices"
    /// 
    /// UsdPhysicsVTriAttach
    const TfToken physicsTriIndices;
    /// \brief "physics:triSrc"
    /// 
    /// UsdPhysicsVTriAttach
    const TfToken physicsTriSrc;
    /// \brief "physics:upperLimit"
    /// 
    /// UsdPhysicsPrismaticJoint, UsdPhysicsRevoluteJoint
    const TfToken physicsUpperLimit;
    /// \brief "physics:velocity"
    /// 
    /// UsdPhysicsRigidBodyAPI
    const TfToken physicsVelocity;
    /// \brief "physics:vtxIndices"
    /// 
    /// UsdPhysicsVXformAttach, UsdPhysicsVCurveAttach, UsdPhysicsVTetAttach, UsdPhysicsVTriAttach
    const TfToken physicsVtxIndices;
    /// \brief "physics:vtxIndices0"
    /// 
    /// UsdPhysicsVtxVtxAttach
    const TfToken physicsVtxIndices0;
    /// \brief "physics:vtxIndices1"
    /// 
    /// UsdPhysicsVtxVtxAttach
    const TfToken physicsVtxIndices1;
    /// \brief "physics:vtxSrc"
    /// 
    /// UsdPhysicsVXformAttach, UsdPhysicsVCurveAttach, UsdPhysicsVTetAttach, UsdPhysicsVTriAttach
    const TfToken physicsVtxSrc;
    /// \brief "physics:vtxSrc1"
    /// 
    /// UsdPhysicsVtxVtxAttach
    const TfToken physicsVtxSrc1;
    /// \brief "physics:vtxSrc2"
    /// 
    /// UsdPhysicsVtxVtxAttach
    const TfToken physicsVtxSrc2;
    /// \brief "physics:xformSrc"
    /// 
    /// UsdPhysicsVXformAttach
    const TfToken physicsXformSrc;
    /// \brief "physics:youngsModulus"
    /// 
    /// UsdPhysicsDeformableMaterialAPI
    const TfToken physicsYoungsModulus;
    /// \brief "rotX"
    /// 
    ///  This token represents the rotate around X axis degree of freedom used in Joint Limits and Drives. 
    const TfToken rotX;
    /// \brief "rotY"
    /// 
    ///  This token represents the rotate around Y axis degree of freedom used in Joint Limits and Drives. 
    const TfToken rotY;
    /// \brief "rotZ"
    /// 
    ///  This token represents the rotate around Z axis degree of freedom used in Joint Limits and Drives. 
    const TfToken rotZ;
    /// \brief "seg"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetElemType0Attr(), Possible value for UsdPhysicsElementCollisionFilter::GetElemType1Attr()
    const TfToken seg;
    /// \brief "transX"
    /// 
    ///  This token represents the translate around X axis degree of freedom used in Joint Limits and Drives. 
    const TfToken transX;
    /// \brief "transY"
    /// 
    ///  This token represents the translate around Y axis degree of freedom used in Joint Limits and Drives. 
    const TfToken transY;
    /// \brief "transZ"
    /// 
    ///  This token represents the translate around Z axis degree of freedom used in Joint Limits and Drives. 
    const TfToken transZ;
    /// \brief "tri"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetElemType0Attr(), Possible value for UsdPhysicsElementCollisionFilter::GetElemType1Attr()
    const TfToken tri;
    /// \brief "vtx"
    /// 
    /// Possible value for UsdPhysicsElementCollisionFilter::GetElemType0Attr(), Default value for UsdPhysicsElementCollisionFilter::GetElemType0Attr(), Possible value for UsdPhysicsElementCollisionFilter::GetElemType1Attr(), Default value for UsdPhysicsElementCollisionFilter::GetElemType1Attr()
    const TfToken vtx;
    /// \brief "X"
    /// 
    /// Possible value for UsdPhysicsSphericalJoint::GetAxisAttr(), Default value for UsdPhysicsSphericalJoint::GetAxisAttr(), Possible value for UsdPhysicsPrismaticJoint::GetAxisAttr(), Default value for UsdPhysicsPrismaticJoint::GetAxisAttr(), Possible value for UsdPhysicsRevoluteJoint::GetAxisAttr(), Default value for UsdPhysicsRevoluteJoint::GetAxisAttr()
    const TfToken x;
    /// \brief "Y"
    /// 
    /// Possible value for UsdPhysicsSphericalJoint::GetAxisAttr(), Possible value for UsdPhysicsPrismaticJoint::GetAxisAttr(), Possible value for UsdPhysicsRevoluteJoint::GetAxisAttr()
    const TfToken y;
    /// \brief "Z"
    /// 
    /// Possible value for UsdPhysicsSphericalJoint::GetAxisAttr(), Possible value for UsdPhysicsPrismaticJoint::GetAxisAttr(), Possible value for UsdPhysicsRevoluteJoint::GetAxisAttr()
    const TfToken z;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdPhysicsTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdPhysicsTokensType
extern USDPHYSICS_API TfStaticData<UsdPhysicsTokensType> UsdPhysicsTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
