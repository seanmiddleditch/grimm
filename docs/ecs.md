Entity Component System Architecture
====================================

Concepts
--------

The Potato ECS is based on the use of Archetypes. Key definitions for this design:

- **Entity**: an abstract representation of a "game object" that exists
 in the simulation
- **Component**: a discrete chunk of plain data used to describe some aspect of an Entity
- **Tag**: a Component with no data
- **System**: a function that operates on Components to mutate data and progress the simulation
- **Archetype**: a classification of Entity with a specific set of Component types; all Entities with the same types of Components will share the same Archetype
- **Query**: a selector to find all Archetypes which include a particular set of Components
- **World**: a container for Components, Entities, and Archetypes; Entities in different Worlds cannot interact
- **Chunk**: the storage arena for Components associated with a particular Archetype

Abstract Description
--------------------

Potato ECS follows the "pure" definition of Entity Component System architecture. In Potato, a **Component** is just a plain holder of data, typically a `struct` with some fields. Some Components may not contain any data at all and are called **Tag** Components.

Components contain no logic at all. For operating on Component data, a **System** is used. Systems are functions or classes which operation on Components, grouped together by logical **Entity**. An Entity is just a way of associating Components together.

A System will typically use a **Query** asking to find Entities with some specific set of Components. For example, a rendering System might want to find all Entities with a `Transform` Component and a `Mesh` Component, independent of whatever other Components those Entities might have. A Query is used to express this requirement and searches for compatible Entities.

A Query does not actually search for Entities. Instead, it searches for **Archetypes**. An Archetype is a grouping of Entities with identical sets of Components. For example, Entities which represent static props in a game simulation might have the Components `{Transform,Mesh}`; a set of Entities for dynamic props might have `{Transform,Physics,Mesh}`; and a set of Entities for static invisible trigger volumes might have `{Transform,Collider,Script}` Components. The aforementioned rendering System would use a Query looking for `{Transform,Mesh}`, which would then find the first two archetypes but not the third (it has no `Mesh` Component). Because there will typically be many Entities with the same Components, there will be far fewer Archetypes than Entities, and searching for Archetypes will be much faster than searching all Entities.

The real benefit of this approach is enabled by the concept of **Chunks**. All the Components belonging to Entities in a single Archetype are allocated together in a series of **Chunks**. The placement of Components without a **Chunk** is determined by the Archetype's specific layout, which can be thought of similarly to Input Assemblies used in graphics shaders. All Components of a given type are allocated contiguously in each Chunk, with the specific offsets/strides described by the layout.

Because Components are allocated together in their associated Archetype, actually operating on Components can be done very efficiently. After a Query finds the set of compatible Archetypes, it iterates the Chunks for each Archetype to pass pointers based on the layout to the System for performing work. Because multiple Chunks might be present, a System might be invoked multiple times with different pointers. We can ensure that System is invoked once for each Chunk that contains Components belonging to Entities compatible with the System's requested Query.

Continuing the prior example, the render System would be invoked (at least) twice: once with pointers for the `Transform` and `Mesh` Components stored in the first Archetype's Chunk(s), and then again with the pointers for the Components stored in the second Archetype's Chunk(s).

When operating on large numbers of similar Entities, this approach will result in very efficient use of the CPU and memory caches. When operating on relatively "unique" Entities, efficiency will be lower, but should be no worse than more traditional component-based engine designs.

Archetypes and Chunks
---------------------

For traditional ECS, the architecture calls for all Components to be allocated contiguously. This allows operating on a specific Component to be highly cache-efficient. However, difficulties arise when needing to operate on multiple Components simultaneously; the engine needs to iterate multiple contiguous Component allocations while associating those Components together. Traditional approaches have involved hash tables, sorted containers, and other algorithmic solutions.

The solution used by Potato instead relies on allocating associated Components separately. A single Component type, such as `Transform`, might then exist in multiple contiguous arrays, depending on their Entity's associated Components. This ensures a direct mapping between associated Components via index into their respective arrays.

The concept of an Archetype is used to determine which Components are asociated and require their arrays to be colocated. The set of Components belonging to an Entity is used to identify the Entity's Archetype. The layout defines the offsets and strides for arrays containing associated non-Tag Components.

The actual Components are stored in Chunks. A Chunk is just a hunk of memory (e.g. a 64kb block) owned by a particular Archetype; it is sliced into arrays of Components as directed by the layout.

For example, let's assume some Entities with the Components `{Static,Mesh,Transform}`. That set of Components defines an Archetype, which for convenience we'll call the *StaticMeshTransformArchetype*. Let's further assume that `Static` is a Tag and holds no data. Since the layout only includes the non-Tag Components `{Mesh,Transform}`. The Chunks allocated for this Archetype would look something like this in memory:

    |-----------------------------------------|
    | Mesh      | Mesh      | ... | Mesh      |
    | Transform | Transform | ... | Transform |
    |-----------------------------------------|

We see that it says its Chunks contain the `Mesh` Component at offset 0 and the `Transform` Component at offset `sizeof(Mesh)`, and notes how many of these can be allocated in each Chunk (something like `sizeof(Chunk)/(sizeof(Mesh)+sizeof(Transform))`).

Let us assume then that the game wants to find all static meshes in the game and bake them into a static vertex batch. It would execute a query for `{Static,Mesh,Tranform}`. In this case that's exactly identical to our *StaticMeshTransformArchetype*, so it will find that archetype.

Let's assume that there's another Archetype, the *ProjectilePhysicsMeshTranformArchetype*, whose layout describes Chunks that look like:

    |--------------------------------------------|
    | Projectile | Projectile | ... | Projectile |
    | Physics    | Physics    | ... | Physics    |
    | Mesh       | Mesh       | ... | Mesh       |
    | Transform  | Transform  | ... | Transform  |
    |--------------------------------------------|

Note that the offsets of `Mesh` and `Transform` are different, and the maximum number of Components per Chunk will be smaller.

Our Query for `{Mesh,Tranform}` would also find this Archetype, and pairs of `Mesh*` and `Transform*` pointers would be found for each Chunk belonging to *ProjectilePhysicsMeshTranformArchetype*.

The System's logic is then invoked for each of these pairs of pointers, along with a count; different Chunks can fit a different maximum number of Components, and some Chunks may not have full residency (e.g., if only a few Entities for a given Archetype exist, there may not be a full Chunk's worth of Components).

Because the System is just given pointers and a count, it doesn't need to know anything at all about Archetypes or Chunks. It just iterates over the Components at the provided spans of memory.

Creating, Destroying, and Modifying Entities
--------------------------------------------

New Entities cannot always be safely inserted into an Archetype's Chunks. Background jobs on other threads may be operating on the Chunks, and due to cacheline sharing, it is unsafe for another thread to mutate those Chunks' memory unless all tasks are coordinating (e.g. using atomic operations). For this reason, new Entities must be queued up and only processed at safe points.

This problem is exasperated by Entities which are destroyed; a destroy cannot be processed immediately because the data may be in active use. Even if we were to know that the specific Entity and its Components aren't in use, we have to "backfill" the memory occuped by the destroyed Components to maintain our nice linear arrays, and this requires moving other Components around. None of the Chunks can be in active use during the destroy operation. For this reason, destroying Entities must also be queued up.

Modifying an Entity's set of Components can be seen as a combination of Create + Destroy. Adding or removing a Component from an Entity will change its Archetype. Since Components are located in Chunks belonging to its Archetype, adding or removing Components will typically result in the Entity needing to be copied into a new Chunk. The copy can be comprised of an addition of the Components to the new Chunk followed by removal from the old Chunk.

Exactly how this queuing works is still TBD. The gist is that the World will need to somehow track the requested additions and deletions and apply them at known safe points between System updates. Deletions are relatively easy (just a list of Entities to be deleted), while creations will also need to buffer up the new Entity's Component data via some mechanism.

Entity Lookup
-------------

While the ECS pattern discourages such, it is sometimes necessary to find a Component based on its Entity. Because Components are associated to an Entity via an Archetype, it is thus necessary to find the Archetype for an Entity; it is then necessary to find the Entity's index in that Archetype.

The basic approach here is to keep tables (as hash maps). One table, in the World, contains Entity identifiers and the Entity's associated Archetype. A slot map data structure is a potential option for Entity identifiers, and an index into the World's list of known Archetypes can be the value stored for the Entity's identifier.

Inside the Archetype, a simple hash map of Entity identifier to index can be stored; there is no need for a slot map here as the index inside an Archetype should be considered transient, and is only really used to find pointers to Components (and the pointers are also transient).

For handling the reverse - where a Component needs to find its own Entity identifier - a few options are available. The simplest is to pass in the Entity identifier as a new array to the System. These could even be allocated separately from the regular Chunk if that were deemed advantageous.

Testing
-------

A killer features of ECS is the ability to easily write tests for game simulation code.

Traditional game engines - even those based on components of some kind - often have a lot of "spaghetti" dependencies between components and external managers. In order to test a component's logic, it is often required to construct elaborate test scenes with many objects and components, carefully balanced to satisfy all dependencies. This can become a maintenance nightmare, and often makes tests so difficult to write that they simply never get written.

With ECS, all testable logic resides in Systems, which mostly just operate on simple arrays of Components. Communication between Systems happens via Component mutations, and Components do not communicate themselves nor have intrinsic dependencies. This makes construction of a test fixture almost trivial: fill some arrays with appropriate Component data, run the System, and then inspect the arrays for the appropriate mutations.

Systems may still have dependencies on external modules like IO, but these kinds of modules are generally easy to mock out for testing purposes. Using arrays and data-oriented techniques also alleviates the overhead of virtual function calls; instead of calling a virtual function for each Entity or Component, for instance, a virtual function may be called with an array containing data for a whole chunk of Entities and Components.
