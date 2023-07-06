========================================
Deformable Body Physics in USD Proposal
========================================


.. include:: rolesAndUtils.rst
.. include:: <isonum.txt>

.. note::
   Add Notes here.

Copyright |copy| 2023, Adobe, Algorx, Apple, Microsoft, NVIDIA, Pixar Animation Studios, SideFX, Weta, *version 0.1*

.. contents:: :local:
    :class: twocolumn

Purpose and Scope
===================

After extending USD with the ability to express Rigid Body physics simulation concepts in 2021, the USD physics working group has decided to prioritize further extending USD with concepts to express Deformable body dynamics.  Our goal here is to create a similarly basic set of abstractions for expressing the initial conditions for the simulation of elastic deformation, including that of volumes, sheets and strands.  We decided to leave concerns around plastic deformation for a future extension.

There exist fewer generally available and commonly used tools and libraries for deformable simulation than for rigid body simulation, which made our search for a common set of abstractions that we can encode more difficult.  We make the same assumptions about our target applications as the previous USD Physics extension: That this is primarily for interactive 3D graphics applications, and that users will be adding simulation capabilities to existing graphics-only USD content.  We would like to continue to extend USD as seamlessly as possible and let the new simulation capabilities interoperate with the existing rigid body abstractions, to let users also create soft objects, cloth, hair or fur with believable behavior.

Guiding Principles
===================

Like the previous USD Physics schema, we wanted to primarily reason about real world concepts rather than simulation specific abstractions, unless those abstractions were overwhelmingly common and helpful.  In particular we stick to real world engineering quantities for material properties.  We continue to support USD's preference for unit independence.  We also continue the pattern established in the rigid body extension that material properties are added to materials rather than the simulated objects directly.  The concept of physics scenes is retained.  We continue to adhere to the USD design principle to always prefer leveraging existing classes or adding APIs to existing classes before we introduce new prims.

As with rigid bodies, we expect all physics objects, now including deformables, to automatically collide and interact by default.

USD Implementation
===================

Scenes
----------

The USD Physics extension established the concept of physics scenes to enable multiple independent simulations within a single stage with different fundamental properties like gravity.  Like rigid bodies, deformables conceptually simulate in a scene.  Everything in a scene interacts by default, and things in different scenes do not interact.  The gravity of a scene is applied to all the deformables in the scene.  Deformables are associated with a scene in the same manner as rigid bodies, using the simulationOwner attribute.

Rigid Body Refactor
-----------------------

The USD Physics extension introduced a PhysicsRigidBodyAPI class.  We would like to reuse the member attributes bodyEnabled, kinematicEnabled, simulationOwner and startsAsleep from this class for deformables; however we cannot reuse the velocity attributes.  We decided to deal with this problem by splitting out a new pseudo-base class called PhysicsBodyAPI from PhysicsRigidBodyAPI, and move the shared attributes to it.  We say pseudo-base class because in practice API inheritance has been removed from USD.  In practice PhysicsRigidBodyAPI and PhysicsBodyAPI are independent APIs which are both be applied to a rigid body.

For legacy schema support, we recommend implementations continue to function whether the shared attributes are either in PhysicsRigidBodyAPI or the new PhysicsBodyAPI.

Physics Material Refactor
----------------------------

The USD Physics extension introduced a PhysicsMaterialAPI class that carried density, surface friction and collision restitution attributes.  We can reuse the density and surface friction attributes for deformables but the restitution coefficient doesn't make any sense for deformable abstractions since their elasticity behavior is inherently simulated.

For this reason we again split restitution out of the PhysicsMaterialAPI class and move it to a new PhysicsRigidMaterialAPI class of which PhysicsMaterialAPI is the pseudo-base class.  This means that for rigid body materials both PhysicsRigidMaterialAPI and PhysicsMaterialAPI are applied.

For legacy schema support, we recommend implementations continue to function whether the density attributes is found either in PhysicsMaterialAPI or the new PhysicsRigidMaterialAPI.

Deformable Materials
-----------------------------

We introduce three new material APIs: PhysicsDeformableMaterialAPI, PhysicsCurvesDeformableMaterialAPI and PhysicsSurfaceDeformableMaterialAPI.

PhysicsDeformableMaterialAPI extends PhysicsMaterialAPI and introduces the important engineering concepts of Young's Modulus and Poisson Ratio as attributes.  Young's modulus has no default value because its units are dependent on the base mass and distance units used, and different defaults would make sense depending on these units.

PhysicsCurvesDeformableMaterialAPI extends PhysicsDeformableMaterialAPI with additional properties for deformable curves, including a thickness and three different stiffness parameters along different dimensions.  These allow a curve to use not the homogenous Young's Modulus but rather different stiffnesses in the stretch, bend and twist directions.  These stiffnesses have the same units as Young's Modulus, and default to zero, which means that behavior should be governed by the Young's Modulus and the curve thickness.

PhysicsSurfaceDeformableMaterialAPI extends PhysicsDeformableMaterialAPI with additional properties for thin sheets, including a thickness and two different stiffness parameters along three different dimensions.  These allow a sheet to use not the homogenous Young's Modulus but rather different stiffnesses in the stretch, shear and bend dimensions.  These stiffnesses have the same units as Young's Modulus, and default to zero, which means that behavior should be governed by the Young's Modulus and the sheet thickness.

It is permitted to apply multiple physics material APIs to a single material.  This makes it possible to use the material on e.g. both rigid and deformable objects, or both sheets and curves::

Material rubberMaterial with PhysicsMaterialAPI, PhysicsRigidMaterialAPI, PhysicsDeformableMaterialAPI, PhysicsCurvesDeformableMaterialAPI, PhysicsSurfaceDeformableMaterialAPI

This means that e.g. friction parameters have to be the same for rigid and deformable use cases.  If this is a problem, the workaround is to create e.g. separate rigidRubber and deformableRubber materials.

Deformable Bodies
----------------------------

We introduce a class PhysicsDeformableBodyAPI to denote that a prim is to be a deformable, whether volume, surface or curve.  Its pseudo-base class PhysicsBodyAPI is also applied to any prim that has PhysicsDeformableBodyAPI applied.

To create a volume deformable, apply this class to a GeomTetMesh (see upcoming section Tetrahedral Meshes).  Simulation properties for the volume deformable will be found in the PhysicsDeformableMaterialAPI and PhysicsMaterialAPI on the applied material object.

To create a surface deformable, apply this class to a GeomMesh.  Simulation properties for the surface deformable will be found in the PhysicsSurfaceDeformableMaterialAPI, PhysicsDeformableMaterialAPI and PhysicsMaterialAPI on the applied material object.

To create an array of curve deformables, apply this class to a GeomBasisCurves or GeomBasisCurvesNetwork (proposed).  Simulation properties for the curve deformables will be found in the PhysicsCurvesDeformableMaterialAPI, PhysicsDeformableMaterialAPI and PhysicsMaterialAPI on the applied material object.

For reasons explained in the upcoming Storing Simulation State section, it must also be possible to place the PhysicsDeformableBodyAPI on an immediate parent prim of the target GPrim types mentioned above, such as an Xform or a Scope.  The same section also describes how the rest configuration of a deformable is supplied as a PhysicsDeformableRestShapeAPI on the target GPrim. 

The PhysicsDeformableBodyAPI has a total mass attribute which if defined, overrides the mass properties derived from material density.  For deformables, the mechanism to compute the simulation mesh and therefore the total volume from the simulation mesh is not precisely defined, and therefore this is a way to assign a very specific total mass.

Deformable objects, unlike rigid bodies, do not have a single coordinate frame which would be seamlessly compatible with USD's concept of a transform hierarchy.  For this reason, after a simulation step, the Xform of a deformable body, unlike a rigid body, is not updated with a new pose.  Instead, deformables receive new per-vertex state.

We have discussed allowing the application of PhysicsDeformableBodyAPI to untessellated prims such as GeomSphere, GeomCube, etc.  We have decided that this would force implementations to replace these objects using arbitrarily tessellated versions with no user control for the purpose of deformable animation, and thus decided that this scenario is not desirable.  If a user wants to make a GeomSphere deformable, the sphere should first be replaced with a GeomMesh with the desired tessellation and then turned into a deformable.


Tetrahedral Meshes
------------------------------

At this point we need to take a detour to introduce the UsdGeomTetMesh class which we believe is slated to be added to USD shortly. We will be depending on it being available.

Storing Simulation State
-----------------------------

The entire state vector of a rigid body is its pose in space, and its instantaneous velocity.  Both of these are captured in USD using the rigid body's Xform and velocity attributes.  This makes it possible to start simulating rigid bodies with nonzero initial velocities, or to store out the entire state of a rigid body during simulation to USD and continue the simulation later.

To simulate a deformable object we must first define its rest shape. If we were not able to do this, an elastic object draped over an object or suspended from somewhere would forget that it has already stretched out under its own weight due to gravity, and continue to stretch more and more every time the simulation is saved out to USD and then later started again. In more complex use cases we may also want the rest shape to change over time. To support this we have defined multiple RestShapeAPI schemas which need to be applied to the appropriate UsdGeom type. Refer to the section "Rest shape API design", later in this document to better understand the proposed rest shape schemas.

In addition to the current rest shape, the current dynamic state of a deformable is required to restart simulations. UsdGeomPointBased has support for position state through its "points" attributes and velocity through its "velocities" attribute. These attributes can be time-varying and as such if the previous velocity or previous position states are required to support higher order integration schemes, they can be obtained at a prior time sample.

We would like to recognize that often it is possible and desirable to simulate deformation at a lower discretization than we use for rendering. For example, we might have a very detailed surface GeomMesh of a car tyre to capture all the geometric details of the threads carved into the running surface, made up of tens of thousands of triangles, but an approximate simulation of the stiff rubber material can be done using a volumetric mesh of less than a thousand elements.  The simulation operates by deforming the volumetric mesh in response to gravity and collisions, and then deforming the surface mesh my maintaining its embedding in the simulation mesh.

For volume deformables, we use the above TetMesh class to encode simulation meshes.  Even though some implementations might use other discretizations, e.g. hexahedral meshes, it should always be possible to convert these to a tetrahedral mesh.  Because we would like to benefit from USD's basic feature of Xform inheritance and automatic rendering of GPrims parented in the scene graph, we elect to store the TetMeshes as children of the prim that has the PhysicsDeformableBodyAPI.  Because GeomMesh objects with children are not legal, we have to allow the placement of the PhysicsDeformableBodyAPI on a parent of the GeomMesh, such as a Scope or an Xform. Our tire example would look something like this:

	Xform or Scope deformable with PhysicsDeformableBodyAPI and material assignment with PhysicsDeformableMaterialAPI
		GeomMesh renderGeom (detailed render geometry to be deformed according to embedding in simGeom)
		TetMesh simGeom with PhysicsDeformableRestShapeAPI applied

For surface deformables, the simulation mesh is of type GeomMesh as well, but can still be discretized differently.  For similar reasons as for volume deformables, we permit the following parenting::

	Xform or Scope deformable with PhysicsDeformableBodyAPI and material assignment with PhysicsSurfaceDeformableMaterialAPI
		GeomMesh renderGeom
		GeomMesh simGeom with PhysicsSurfaceDeformableRestShapeAPI

For curve deformables, we again follow a similar pattern::

	Xform or Scope deformable with PhysicsDeformableBodyAPI and material assignment with PhysicsCurvesDeformableMaterialAPI
		UsdGeomCurves renderGeom
		UsdGeomBasisCurves or UsdGeomBasisCurvesNetwork (proposed) simGeom with PhysicsCurvesDeformableRestShapeAPI

It is also permissible to have the simulation material assignment on the renderGeom.

Providing a simulation state prim with corresponding PhysicsDeformableRestShapeAPI is mandatory.  It is assumed that tools will be responsible to create a suitable deformable body description from one or multiple render meshes.

It is common in the case of hair/fur simulation to simulate only a subset of curves and use the simulation results to deform a much larger number of curves used for rendering. One can imagine the simulated deformation of a UsdGeomBasisCurvesNetwork being used to deform other geometry types as well.

We expect implementations to differ regarding how exactly they perform collision detection.  We can anticipate high fidelity approaches to use the surfaceMesh directly, while others might use the lower fidelity collision mesh, or even an intermediate resolution transient approximation.

Multi-part Surface Geometries
------------------------------------

In all three of the above cases, the PhysicsDeformableBody must be on the immediate parent of the sibling geometries described.  It is however often the case that pre-existing graphical assets are split into multiple graphics meshes for e.g. easy assignment of graphics materials.  For example::

	Xform rubberChickenToy
		GeomMesh body
		GeomMesh eyeballs
		GeomMesh legs

If we would like to mark up this asset to behave as a volume deformable, the easiest way is to have it act as a single simulation object into which all three meshes are embedded:

	Xform rubberChickenToy with PhysicsDeformableBodyAPI with soft material assignment with PhysicsDeformableMaterialAPI
		GeomMesh body
		GeomMesh eyeballs
		GeomMesh legs with stiff material assignment with PhysicsDeformableMaterialAPI
		TetMesh simGeom with PhysicsDeformableRestShapeAPI

Here we also leverage the above mentioned freedom to put simulation material assignments on the graphics mesh in order to e.g. make the legs stiffer than the rest of the chicken.  Implementations that support per-tetrahedron material assignments may then derive correct per-tetrahedron material indices that they use internally.  Implementations who do not support this might only use the top level material and ignore additional per-geom materials.

Alternatively, when UsdGeomSubsets supports elemType supports per-element material assignment beyond faces we could apply PhysicsDeformableMaterialAPI directly to specific tetrahedra of the tetmesh as follows:

    Xform rubberChickenToy with PhysicsDeformableBodyAPI with soft material assignment with PhysicsDeformableMaterialAPI
        GeomMesh body
        GeomMesh eyeballs
        GeomMesh legs
        TetMesh simGeom with PhysicsDeformableRestShapeAPI and PhysicsDeformableMaterialAPI specified with future UsdGeomSubset support.

Despite this flexibility, we decided that it is not legal to add PhysicsDeformableBodyAPI to a subtree already marked with PhysicsRigidBodyAPI, or vice versa.  A subtree is to be either deformable or rigid but not both.


Rest Shape API Design
-----------------------------

Rest shape is a fundamental concept for simulating deformable objects. A simulator employs some constitutive material model that computes restoring forces on simplices based on the difference between the rest shape and the current shape.

In the case of PhysicsDeformableRestShapeAPI which is intended for tetmeshes, we specify restShapePoints and restTetIndices. If all adjacent tetrahedral elements of the tetmesh share the same restShapePoints, then restTetIndices would be identitical to the tetVertexIndices attribute of the tetmesh. We aren't limited to this though. Each element can have its rest shape defined by a spatially disjoint set of restShapePoints to allow for the rest shape of different portions of the tetmesh to change independently.

This capability is more important for PhysicsSurfaceDeformableRestShapeAPI which is meant to be applied to surface meshes for shells and cloth. In modeling cloth it is very common to define the planar rest shape of sections of the 3D mesh in a disjoint fashion in a 2D material "panel space." The restShapePoints attribute of PhysicsSurfaceDeformableRestShapeAPI has 3D points, but disjoint sets (w.r.t restFaceIndices) of these points can be coplanar to
support planar rest shape descriptions from panel based creation tools like Marvelous Designer. Furthermore, this representation allows the description of the planar rest shape that should be compatible with models for 3D shells.
Cloth/Shells based on triangular meshes also require the definition of the rest dihedral angle for the interior edges of the mesh. Using mesh of cloth pants as an example, we might want to describe a pleat along a consecutive edge run down the front of each leg. Besides increasing the bendStiffness for these edges, the edgeBendAngles for these edges could be set to something like 75 degrees. The assignment of edgeBendAngles is specified via edgeVertPairs. We are adopting the convention that if edgeBendAngles and edgeVertPairs are empty, that the default rest angle for interior edges is zero degrees. To describe the dihedral rest angles for a 3D shell, edgeBendAngles and edgeVertPairs would be the number of interior mesh edges. This format is explicit and more compact than having a separate restShapePoints array and restBendIndices array.



Kinematic Deformables
-------------------------------

USD physics permits a rigid body to be set to a kinematic state, and have its motion be hand-animated, such that an instantaneous velocity based on the movement can be inferred.  Such a body will act as an infinitely massive physics object and can push other dynamic physics objects out of the way.

The same feature is also desirable for deformables.  Kinematic deformables are marked using their kinematicEnabled attribute.  Most likely animators will prefer to time sample the vertices of the graphics GeomMesh surfaceMesh for volume and surface deformables, in which case the deformable body's simulation mesh should follow the animated motion.


Attachments
----------------
We support attachement of two deformable bodies or a deformable body and an xformable (e.g.: a rigid body or collider). Attachments relationships and index attribute arrays pertain to the simulation meshes, not the associated render geometry of a deformable.
Self attachment is also allowed, provided the supplied indices are suitably disjoint. Ex. Don't attach a vertex to a face that it belongs to or a vertex to itself. Attachments may be hard or soft. The stiffness attribute specifies the strength of the attachment. A value of inf implies that the simulator should treat the constraint as hard if it is possible. The damping attribute only applies if the constraint isn't hard.

PhysicsVtxVtxAttachment provides a simple attachment mechanism between two corresponding sets of vertices of the same length. This can be used to support the common case of stitching together panels of cloth to create garments. If multiple vertices
are to be attached/merged together, this can be achieved by employing multiple instances of PhysicsVtxVtxAttachment.

PhysicsVtxTriAttachment attaches the vertices of one deformable to the faces of another. Each vertex index in the vtxIndices array must have a corresponding triangle face index in triIndices. The array faceCoords is used to describe the vertex-face binding. The first two entries of the vector3f are the U&V barycentric coordinates wrt. the triangle. The third coordinate is the normal offset.

PhysicsVtxTetAttachment is similar, but is meant to attach vertices to tetrahedra of deformables. Instead of faceCoords, it has baryCoords which is simply the three barycentric coordinates of the vertex wrt. the tetrahedra. The fourth coordinate can be determined by the three provided. We don't make any restriction to the vertex location wrt. a tetrahedra. It can be inside or outside.

PhysicsVtxCurveAttachment provides a mechanism to attach vertices to curves which may be deformable or kinematic. For each vertex specified in vtxIndices, there is a corresponding entry in curveSegmentIndices and curveSegmentCoords. curveSegmentIndices indentifies the segment within the curve and curveSegmentCoords describes the parametric value of the attachment point for the curve segment.

PhysicsVtxXformAttachment attaches the vertices of one deformable to an arbitrary xformable coordinate frame with support for spatial offsets defined within the referenced coordinate frame. If the referenced xformable is a rigid body or a prim in the subtree of a rigid body, then the attachment is expected to be treated by the solver as a two way constraint.


Element Collision Filtering
---------------------------

While PhysicsCollisionGroup and PhysicsFilteredPairsAPI allow for primitive level collision filtering which is also supported for deformable bodies, PhysicsElementCollisionFilter supports collision filtering at the level of vertices, segments or edges and faces. This is needed particularly in the context of attachments where usually collisions need to be suppressed to avoid conflicting constraints.
Element filter relationships and index attribute arrays pertain to the simulation meshes, not the associated render geometry of a deformable.
The element based collision filters support collision filtering between two deformables and between deformables and rigid bodies/colliders. Not supported is filtering based on tetrahedral elements. It is assumed filtering collisions at the tetrahedral mesh surface is sufficient.


Branching Curves
----------------

Curves are often used to simulate structures such as trees or netting, which can be described as a set of curves with
shared points. This cannot be expressed with existing subclasses of UsdGeomCurves (duplicating the shared points may be sufficient for rendering, but for simulation this introduces additional degrees of freedom that would require additional attachment constraints to eliminate). We are proposing a new type in usdGeom called BasisCurvesNetwork to support this.

