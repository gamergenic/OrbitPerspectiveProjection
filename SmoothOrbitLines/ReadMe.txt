SmoothOrbitLines

PURPOSE
This example project demonstrates rendering orbits as conic sections perspectively projected onto a view-aligned plane.

The goal is to tesselate only relevant sections of each conic, in a space relevant to the viewer's eye (such as clip space).  This avoids results in a curved line that avoids artifacts such as regions where the curve appears to be represented by individual line segments.

The example is implemented in Unreal Engine 5 (Preview).  It is not optimized for any particular purpose (readability, performance, memory, maintainability, the author or author's organization's marketability, climate change, etc).

ALGORITHM
Project an elliptical orbit only a view-aligned plane.  (Hyperbolic orbits are not handled.)
Clip the projection against the lateral frustum boundaries.  (Up to 4 conic subsections result.)
Tesselate each conic subsection as a line against the view aligned plane.

IMPLEMENTATION
The code of interest is located in Source/OrbitRendering/Private/Conics.

ProjectEllipseToPlane.h
Implementation of an algoritm that perspectively projects an Ellipse (such as an orbit) onto a plane (such as the frustum near/far etc plane).  The resulting projection is another conic section (circular, elliptical, parabolic, hyperbolic).  Only Elliptical and Hyperbolic projections are explicitly handled, as the circular and parabolic projections only occur in contrived cases.
This algorithm is an implementation of an algorithm originally developed by David Eberly and published by Geometric Tools.  The published algorithm has since been significantly rewritten, and I'd urge anyone attempting this algorithm in their own code to reference it for their own implementations.  It can be viewed at:
https://www.geometrictools.com/Documentation/PerspectiveProjectionEllipse.pdf

ClipEllipseToFrustum.h
This clips an elliptical conic section to a rectangular view frustum area.  No assumption is made that it in clip space or located on an actual frustum plane - only that the rectangle is axis aligned and centered at the origin.  The result is up to 4 conic subsections defined by the angle between the +x axis and section start/end.  The implentation is based on simple algebra to find the points of intersection between the ellipse and clip lines.   If there is no intersection with a given line, the ellipse is tested to determine whether it is on the visible or non-visible side of the line.  The conic section is returned as not-visible if it is determined to be on the non-visible side of any clip line.   If that is not the case, yet the ellipse does not intersect any clip lines it is assumed to be fully visible.   The intercept points are then sorted and points where the ellipse first begins to be visible and non-visible are isolated to define segment start/stop times.

ClipHyperbolaToFrustum.h
Equivalent to above, but for Hyperbolic conic sections.
Likewise, the implementation is based on simple algebra to determine intersection points against the clip lines.  If there is no intersection, the hyperbola's first asmptote is compared to the clip line to determine if the hyperbola is initially visible or non-visible.  If not visible, the hyperbola is returned as not-visble in entirety.   All intersection points are sorted in order.  The initial visiblility state of each clip line (visible/non-visible) is used to determine segment visibility following each intersection.

Modules
OrbitalPhysics
This is just a very simple & basic solar system (Sun, Mercury, Venus, Earth, Mars. Pallas - an asteroid - is included to add some variety in the form of a higher inclination orbit.)   Each body is defined by simple Kepler Orbit.  All orbits are oscillatory (meaning elliptical orbits, hyperbolic escape orbits are not supported.  Hopefully none of us live to see the day Earth is on an escape orbit anyways, right?)  Sub-orbits (moons, etc) are not supported.  Most types of interest are defined in types Unreal Engine is capable of serializing and exposing in blue prints.  "OrbitingBody" component can be added to an object to make it a planet.   "OrbitSystemState" component represents the state of the universe - an et (ephemeris time) epoch - in seconds past J2000.

OrbitRendering
Contains the algorithm described above.   An "OrbitProjector" component is added to the pawn.   The orbit projector is responsible for projecting orbits into clipped screen-aligned projections.  A "ConicRenderer" component renders the conic subsections as orbit lines.  The "OrbitViewerController" component's is responsible for mapping Unreal Engine's single precision scenegraph space to the double-precision universe's space.  The universe position is shifted to keep the object of interest centered in the UE scenegraph.  As scenegraph positions are thus view dependent, OrbitViewerController's role is to translate data types from one coordinate system to the other.
The ConicRendererComponent is an implementation based on UMeshComponent.  It is not intended to be the best manner to implement line rendering in UE nor best practices.  Its role here is to simply get lines on the screen without crashing.

SmoothOrbitLines
The main module the defines the example's pawn (OrbitViewerPawn), Player Controller (OrbitViewerController), Game State (OrbitGameState) and planet actors (OrbitingBody).  There's nothing special about these implementations other than the invoke the other two modules.  E.g. other implementations can be substituded here and should be able to use the Orbital Physics and Orbit Rendering modules similarly.

NOTES
The algorithm as currently implemented contains a glaring inefficiency.  It deduces the conic section's description - which could be used to clip and tesselate in 2D clip space.  These vertices could enter the vertex shader pre-transformed.   However, the current implementation places the vertices on a screen-aligned plane - in the 3D scenegraph space.   This requires the vertex shader to re-transform the vertices.
The current implementation projects and clips the conic sections on the main thread.   This could easily be offloaded to the render thread.
The current implementation tessellates the line on the render thread.  This could easily be offloaded to the GPU via a compute shader.
Portions of the line tesselation could be done in the vertex shader as well.
The current material uses multiple 1-d textures, which could obviously be optimize to reduce the # of samplers.
All line primitives are submitted as a single batch, per-line shader constants (colors, textures, etc) are not possible without a workaround or breaking the batch into multiple lines.


I'm happy to accept feedback, answer questions, etc at chuck@gamergenic.com.   Or, if you'd just like to tell someone you had a really good day today, that's fine, too.
-Chuck Noble, Director of Technology and more, Gamergenic

LICENSE
Copyright 2021 Gamergenic. All Rights Reserved.
Author: chuck@gamergenic.com

Distributed under the Boost Software License, Version 1.0.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.