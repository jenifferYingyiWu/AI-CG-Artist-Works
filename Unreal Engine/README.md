# Unreal Engine 5.4 is now available

<p>
Unreal Engine 5.4 is here, and it’s packed with new features and improvements to performance, visual fidelity, and productivity that will benefit game developers and creators across industries. With this release, we’re delivering the toolsets we’ve been using internally to build and ship Fortnite Chapter 5, Rocket Racing, Fortnite Festival, and LEGO Fortnite. Here are some of the highlights.
</p>

## Animation

### Character rigging and animation authoring

<p>
This release sees substantial updates to Unreal Engine’s built-in animation toolset, enabling you to quickly, easily, and enjoyably rig characters and author animation directly in engine, without the frustrating and time-consuming need to round trip to external applications.
</p>

<p>
With an Experimental new Modular Control Rig feature, you can build animation rigs from understandable modular parts instead of complex granular graphs, while Automatic Retargeting makes it easier to get great results when reusing bipedal character animations. There are also extensions to the Skeletal Editor and a suite of new deformer functions to make the Deformer Graph more accessible.
</p>

<p>
On the animation authoring front, we’ve focused on making our tools both more intuitive and more robust, as well as streamlining workflows. This includes Experimental new Gizmos; reorganized Anim Details; upgrades and improvements to the Constraints system; and a new Layered Control Rigs feature that drastically simplifies adding animation on top of anim clips.
</p>

<p>
Meanwhile, Sequencer—Unreal Engine’s nonlinear animation editor—gets a significant makeover, with better readability and improved usability in several aspects of the Sequencer Tree. Among other new features in this release, we’ve also added Keyframe Scriptability, which opens up further potential for the creation of custom animation tools.
</p>

### Animation gameplay

<p>
Motion Matching, previously introduced as an Experimental feature, is now Production-Ready: in fact, it’s been battle-tested in Fortnite Battle Royale and shipped on all platforms from mobile to console, running on all 100 characters plus NPCs.
</p>

<p>
Motion Matching is an expandable next-gen framework for animation features. Instead of using complex logic to select and transition animation clips at runtime, it relies on searching a relatively large database of captured animation using the current motion information of the character in game as the key.
</p>

<p>
In this release, we’ve focused on making this animator-friendly toolset robust, performant, and memory-scalable, as well as adding a suite of debugging tools that give developers visibility to its inner workings.
</p>

<p>
Also on the gameplay front, we’ve added Choosers, a much-requested tool that enables you to use game context to drive animation selection. The system can both use variables to inform selections and set variables based on those selections to inform back to gameplay logic.
</p>

## Rendering

### Nanite

<p>
Nanite—UE5’s virtualized micropolygon geometry system—continues to receive enhancements, starting with an Experimental new Tessellation feature that enables fine details such as cracks and bumps to be added at render time, without altering the original mesh.
</p>

<p>
Moreover, the addition of software variable rate shading (VRS) via Nanite compute materials brings substantial performance gains. There’s also support for spline mesh workflows—great for creating roads on landscapes, for example. In addition, a new option to disable UV interpolation enables vertex animated textures to be used for World Position Offset animation; effectively, this means that the AnimToTexture plugin now works with Nanite geometry.
</p>

### Temporal Super Resolution

<p>
In this release, Temporal Super Resolution (TSR) has received stability and performance enhancements to ensure a predictable output regardless of the target platform; this includes reduced ghosting thanks to new history resurrection heuristics and the ability to flag materials that use pixel animation.
</p>

<p>
In addition, we’ve added new visualization modes that make it easier to fine-tune and debug TSR’s behavior, together with a number of new options in the Scalability settings to control it with respect to target performance.
</p>

### Rendering performance

<p>
With many developers targeting 60 Hz experiences, we’ve invested significant effort into improving rendering performance in UE 5.4; this includes refactoring the systems to enable a greater degree of parallelization, as well as adding GPU instance culling to hardware ray tracing, which also now benefits from additional primitive types and an optimized Path Tracer. Further optimizations have been made to shader compilation, resulting in a notable improvement in project cook times.
</p>

### Movie Render Graph

<p>
For those creating linear content, Unreal Engine 5.4 introduces a major update to Movie Render Queue as an Experimental feature. Dubbed Movie Render Graph (MRG), the new node-based architecture enables users to set up graphs to render a single shot, or design them to scale out across complex multi-shot workflows for large teams of artists. Graphs are pipeline-friendly, with Python hooks for studios to build tools and automations.
</p>

<p>
MRG includes Render Layers, a long-requested feature that offers the ability to easily generate high-quality elements for post compositing—such as separating foreground and background elements—with support for both the Path Tracer and the Deferred Renderer.
</p>

## AI and machine learning

### Neural Network Engine

<p>
In Unreal Engine 5.4, the Neural Network Engine (NNE) moves from Experimental to Beta status. With support for both in-editor and runtime applications, NNE enables developers to load and efficiently run their pre-trained neural network models.
</p>

<p>
Example use cases include tooling, animation, rendering, and physics, each with different needs in terms of platform and model support. NNE addresses these disparate needs by providing a common API, enabling easy swapping of backends as required. We’ve also provided extensibility hooks to enable third-party developers to implement the NNE interface in a plugin.
</p>

## Developer iteration

### Cloud and local Derived Data Cache

<p>
New in this release, Unreal Cloud DDC is a self-hosted cloud storage system for Unreal Engine Derived Data Cache (DDC). Designed for distributed users and teams, it enables them to efficiently share Unreal Engine cached data across public network connections.
</p>

<p>
This low-maintenance, secure, and accessible solution automatically replicates data between multiple regional Unreal Cloud DDC hosted endpoints, enabling users to always connect to the closest one available. Protected by OIDC login and authentication, the system has been production-tested on AWS at Epic, and also comes with deployment instructions for Azure.
</p>

<p>
This release also sees enhancements for our local DDC, which now uses a new Unreal Zen Storage server architecture, offering improved data conditioning performance; faster editor load times and Play In Editor (PIE) workflows; and greater control over cache writes, eviction, and data deduplication.
</p>

### Multi-process cook

<p>
Introduced as Beta in Unreal Engine 5.3, Multi-Process Cook is now Production-Ready. The feature enables developers converting content from the internal UE format to a platform-specific format to leverage additional CPU and memory resources, significantly reducing the time it takes to get a cooked output from a build farm server or on a local workstation.
</p>

### Unreal Build Accelerator

<p>
This release introduces the new Unreal Build Accelerator (UBA), a scalable distributed compilation solution for C++. UBA is used in conjunction with Unreal Build Tool and/or Unreal Horde’s Remote Execution (compute task) system to accelerate build compilation time.
</p>

<p>
Currently a Beta feature, in this release UBA supports Windows OS for C++ compilation jobs; native macOS and Linux support are included as Experimental, along with process idle  detection and shader compilation.
</p>

## Media and entertainment

### Motion graphics

<p>
Unreal Engine 5.4 introduces an Experimental new Motion Design mode, equipped with specialized tools for authoring complex 2D motion graphics.
</p>

<p>
Developed in conjunction with production testing and feedback from leading broadcasters, this dedicated mode is engineered to provide an elevated user experience and sustained productivity for motion designers. It offers a comprehensive suite of tools, including 3D cloners, effectors, modifiers, animators, and more.
</p>

### Virtual production

<p>
Filmmakers embracing virtual production will benefit from updates to Unreal Engine’s Virtual Camera tool, which is now Production-Ready, and adds Android to its existing iOS platform support. Virtual Camera workflows are also now fully supported in Unreal Engine on macOS. The mobile application is now renamed Unreal VCam, and can be found on the Apple Store and Google Play.
</p>

<p>
On the VR Scouting front, we’re introducing an Experimental new fully customizable toolkit that utilizes XR Creative Framework to support OpenXR HMDs—such as Oculus and Valve Index—offering a dramatically improved user experience over the existing Virtual Scouting toolkit.
</p>

<p>
For ICVFX, a new Depth of Field Compensation feature means you can accurately control the amount of DOF falloff of the digital content rendered by nDisplay as seen by the movie camera, yielding better results for close-up beauty shots.
</p>

<p>
We’ve also added Multi-Process Inner Frustum, enabling the render of what’s seen by the movie camera to be split across more GPUs and hardware resources, alongside numerous stability improvements and other enhancements to our SMPTE 2110 support—which is moving closer to Production-Ready status.
</p>

### Linux support

<p>
Studios using Linux will also benefit from editor stability improvements on the platform, as well the introduction of Experimental ray tracing support for Vulkan.
</p>

## Cloth simulation

### USD importer

<p>
A new USD Importer in the Panel Cloth Editor means it’s now possible to import a garment—including simulation parameters—from Marvelous Designer or CLO and set it up to simulate in real time within minutes.
</p>

<p>
With automatic simulation graph setup, skinning, and level of detail (LOD) creation, this new workflow enables even users with little or no experience to create convincing garments for their Unreal Engine characters.
</p>
