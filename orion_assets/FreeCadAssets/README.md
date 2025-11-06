# FreeCAD Assets

In this directory you can explore, modify and export all the components used while designing the ORION robot.

It was made using **FreeCAD 1.0.0**, so if you work with previous versions please consider to update it.

## Known issues

### Long loading of half and complete platform

The CAD files of the bases/levels of the robot have high complexity, so they make take a while to load. You can improve this by simplifying the properties of your FreeCAD visual and ensuring you are using GPU (if available) when running FreeCAD.

### Assemblies

There aren't ORION assemblies in FreeCAD due to high payload when building the robot assembly with Assembly2, Assembly3 and Assembly4. In order to test your modifications, it is recommended to use partial assemblies.
