# FreeCAD Assets

![orion_in_freecad](/docs/readmes/orion_freecad_assembly.png)

In this directory you can explore, modify and export all the components used while designing the ORION robot.

It was made originally using **FreeCAD 1.0.0**, but later migrated to **FreeCAD 1.1.1**. So if you work with later versions please consider to update the pieces first.

## Known issues

### Long loading of half and complete platform

The CAD files of the bases/levels of the robot have high complexity, so they make take a while to load. You can improve this by simplifying the properties of your FreeCAD visual and ensuring you are using GPU (if available) when running FreeCAD.

### Assemblies

The assemblies were made using the A2Plus Workbench, which may break in case of updating any reference of the pieces.

Currently there are two assemblies:

1. [orion_assembly.FCStd](/orion_assets/FreeCadAssets/assembly_orion.FCStd): A base assembly for minimal visualization of the robot appearance.

2. [orion_assembly_full.FCStd](/orion_assets/FreeCadAssets/assembly_orion_full.FCStd): A full robot assembly showcasing the components position.
