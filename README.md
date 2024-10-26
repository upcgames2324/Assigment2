
# Helheim-Engine
HelheimEngine is a custom-built game engine developed in C++ and OpenGL for Chrysalis. Built specifically for the needs of Chrysalis, HelheimEngine showcases a balance between technical innovation and efficiency, delivering a powerful solution to meet the project's demanding requirements.


- [Game Engine Github Repository](https://github.com/RollingBarrel/Helheim-Engine)

## Installation

Download Zip file in the latest [release](https://github.com/RollingBarrel/Helheim-Engine/releases), unzip it and execute the Helheim_Engine.exe file.

## Rolling Barrel Team
**Programmers**
- Jordi Joan Alonso Llovet [LinkedIn](https://www.linkedin.com/in/jordi-alonso-llovet-99bb01b5/)
- Jordi Bach Balcells [LinkedIn](https://www.linkedin.com/in/jordi-bach-822510214/)
- Marc Belmonte Juliachs [LinkedIn](https://www.linkedin.com/in/marc-belmonte-juliachs-741470240/)
- Jaime Coronado [LinkedIn](https://www.linkedin.com/in/jaime-andres-coronado-arteta-23700728a/)
- Ariadna Cortés i Puig [LinkedIn](https://www.linkedin.com/in/ariadna-cortes-puig)
- Marc Gabernet Rodríguez [LinkedIn](https://www.linkedin.com/in/marc-gabernet-rodríguez-56213b24a/)
- Oriol Gallego Vazquez [LinkedIn](https://www.linkedin.com/in/oriol-gallego-vazquez-778266227/)
- Moises Garcia Jimenez [LinkedIn](https://www.linkedin.com/in/mois%C3%A9s-garcia-jimenez-797586147/)
- Carlos García Segura [LinkedIn](https://www.linkedin.com/in/carlosgs1999/)
- Xinyu Jiang [LinkedIn](https://www.linkedin.com/in/jiang-xinyu-b39a06200/)
- David Longarón Lorente [LinkedIn](https://www.linkedin.com/in/davidlongaron/)
- Giovanni Matera [LinkedIn]()
- Oscar Mellado Arbelo [LinkedIn]()
- Adrián Mirabel Martínez [LinkedIn](https://www.linkedin.com/in/a-mirabel/)
- Jordi Nieto Maldonado [LinkedIn](https://www.linkedin.com/in/jordi-nieto-maldonado-b84327144/)
- Oscar Pellicé Mas [LinkedIn](https://www.linkedin.com/in/oscar-pellice-mas/)
- Josep Adrià Vilaplana Miret [LinkedIn](https://www.linkedin.com/in/josepvilaplana/)

**Artists**
- Robin Aitken [LinkedIn](https://www.linkedin.com/in/robin-aitken-aa20949a/)
- Natalia Cabrera [LinkedIn](https://www.linkedin.com/in/natalia-cabrera/)
- Phillip Cherukara [LinkedIn](https://www.linkedin.com/in/philip-c-philip/)
- Roc Feijoo Massot [LinkedIn](https://www.linkedin.com/in/roc-feijoo-massot/)
- Kristina Lyudmilova Stoyanova [LinkedIn](https://www.linkedin.com/in/kristina-stoyanova-898363a7/)
- Joan March Martos [LinkedIn](https://www.linkedin.com/in/joan-march-martos-383b3a287/)
- Carlos Núñez Galindo [LinkedIn](https://www.linkedin.com/in/carlosnunezgalindo/)
- Laia Ruiz Royo [LinkedIn](https://www.linkedin.com/in/laia-ruiz-royo-291632278/?locale=en_US)
- Alex Alonso Torrell Corrales [LinkedIn](https://www.linkedin.com/in/alex-torrell-corrales-6631a4334)

## Features
- 3D scene viewer with multiple debug options
- Loading and Saving of scenes
- Import of GLTF models and DirectXTex textures
- Resource management of imported files
- Model animation with state machines
- VFX system
- Fmod audio support
- Rendering
  - Geometry draw with batching
  - Deffered rendering
  - GPU Culling
  - G-buffer materials
  - Image Based Lightning for ambient lights
  - Post-processing (Bloom, Volumetric lights, Height fog)
  - Screen Space Ambient Occlusion
- C++ Scripting system for gameplay
- AI pathfinding and navigation support.
- UI Canvas creation
   
## Engine Guide

- Camera
  - Hold Right-Click and use “WASD” for a fps-like movement and free look around must be enabled.
  - Use “Q” and Use “E” for moving the camera Up and Down.
  - While Right clicking, move the mouse in any direction for free look.
  - Use the mouse Wheel for zooming in and out.
  - Left ALT + Left Click to Rotate de center of the scene.
  - Holding SHIFT duplicates de movement speed.
- Project Panel
  - Dragging a .gltf file inside the engine should import the file into project panel. (Click Refresh if the new asset is not appearing). 
  **All model files (.gltf, .bin, textures...) should always be in the same folder**.
  - Dragging an asset inside the scene should create a GameObject in the Hierarchy panel with an meshRenderer component.
- Hierarchy Panel  
  - Right click in hierarchy panel will open a popup where you can create, duplicate and delete a gameobject.
  - Dragging a GameObject into another GameObjet will set it as the child of the second object.
- Inspector Panel
  - Click Button "Add Component" in the inspector panel when a gameObject is selected to add a new component.
  - Every gameObject has a transform tha can be modified using the transform section in the inspector panel.
  - The name of a gameObject can be modified in the inspector panel.
- Guizmo
  - W to move object.
  - E to Rotate object.
  - R to scale object.  

- Main Menu bar
  - File
    - Save Scene
    - Load Scene
    - Quit
  - View
    - QuadTree
      - CheckBox to enable or disable Frustum Culling (Disabled by default).
      - CheckBox to show QuadTree in the Scene.
      - Shows QuadTree structure (can be updated using Refresh button).
    - Debug
      - Activate or Deactivate Grid and world Guizmo.
  - Tools
    - View frame data and timer values.
    - Navemesh controller where you can edit parameters.
    - Scene debug options (Draw colliders and bounding boxes, change render type).
    - View Fmod audio memory usage and active events.
  - Window
    - Panels
      - From here every panel of the engine can be opened or closed.
    - Help
      - About
        - Show License and some information about the Engine.
  - Scripting
    - .h files have to be CREATED from VS in the path Engine\Game\Assets\Scripts.
    - Use the TestScript.h and .cpp files as an example to use and create scripts. It is mandatori to create the extern function CreateTestScript.
    - When the .h file is created u have to drag&drop it into the engine to add it as a resource.
    - If it does not compile because linking problems try to rebuild the Scripting project.
