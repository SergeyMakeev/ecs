## About

Warning: This is not a complete production-ready library for entity-component-system.

This is only my thoughts about how the modern entity-component-system should look.

Key things:

* Data-oriented design for components storage.
  Storages support reordering to reduce 'fragmentation.'

* Compile-time type check  fully compatible with C++ types

* Multithreading friendly. There should be no data dependencies that limit the parallel processing. Systems must know what data they need only for reading, and which ones to modify.

* Systems(Processes) can control the order of updating entities. 
  Should be easy to create a parent/child relationship.

* Support for a large number of entities (100K+)
  The worst complexity of the any algorithm must be O(n) or better.

The main difference from other entity-component-system frameworks is ReMap, Fold and Reorder concept. When the system(process) receives notification about a new entity, the process can determine the key for this entity. After the keys of all entities are defined, the entities are updated in the order that is defined by the key.


## Useful reading (in random order):

Theory and Practice of Game Object Component Architecture by Marcin Chady
http://twvideo01.ubm-us.net/o1/vault/gdccanada09/slides/marcinchadyGDCCanada.ppt

Game Architecture and Components Systems in Lumberyard by Rosen Baklov and Bill Merrill 
http://www.gdcvault.com/play/1023600/

A Data - Driven Game Object System by Scott Bilas
http://scottbilas.com/files/2002/gdc_san_jose/game_objects_slides.pdf

Entity Systems are the future of MMOG development by Adam Martin
http://t-machine.org/index.php/2007/09/03/entity-systems-are-the-future-of-mmog-development-part-1/

"Overwatch" Gameplay Architecture and Netcode (pay wall)
http://www.gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and

Unite Austin 2017 - Writing High Performance C# Scripts by Joachim Ante
https://www.youtube.com/watch?v=tGmnZdY5Y-E

A Dynamic Component Architecture for High Performance Gameplay by Terrance Cohen
http://twvideo01.ubm-us.net/o1/vault/gdccanada10/slides/Terrance_Cohen_DynamicComponentArchitecture.ppt

Pitfalls of Object Oriented Programming by Tony Albrecht 
http://gamedevs.org/uploads/pitfalls-of-object-oriented-programming.pdf

Implementation of a component-based entity system in modern C++ by Vittorio Romeo 
https://github.com/CppCon/CppCon2015/blob/master/Tutorials/Implementation%20of%20a%20component-based%20entity%20system%20in%20modern%20C%2B%2B/Implementation%20of%20a%20component-based%20entity%20system%20in%20modern%20C%2B%2B%20-%20Vittorio%20Romeo%20-%20CppCon%202015.pdf

Evolve Your Hierarchy by Mick West
http://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/

Component-oriented design on consoles by Boris Batkin (in Russian)
http://blog.gamedeff.com/?p=91

