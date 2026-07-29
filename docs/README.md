# Documentation

Here you can find a recopilation of resources for the documentation of the ORION Project.

You can find more info on the Wiki of the orion_common repository.

## Information about the directories

- [cam](/docs/cam/): Guides and scripts for camera setup. Currently covers the OV5647 Pi Camera (libcamera RPi fork, camera_ros, ROS 2 Jazzy integration).

- [memory](/docs/memory/): Photos and media from the original thesis project (prior to the fork). Kept as a historical reference.

- [readmes](/docs/readmes/): Images and GIFs used across the package READMEs.

- [schematics](/docs/schematics/): Images for electronic connections and cable management of the project.

- [utility_scripts](/docs/utility_scripts/): Bash scripts for documentation tasks such as generating Markdown image links and batch-renaming photos.

- [V1.3.2_com](/docs/V1.3.2_com/): Photos of the ORION project at hardware version V1.3.2.

- [V1.5.1_com](/docs/V1.5.1_com/): Photos of the hardware modifications introduced from version V1.3.2 to V1.5.1.

## Tips and tricks

The [utility_scripts](/docs/utility_scripts/) directory contains the following bash helpers:

- `reduce.sh` — compresses image files for upload to GitHub.
- `links.sh` — generates Markdown-formatted links to images for use in the Wiki.
- `html_format.sh` — generates HTML image tags for alignment and size control.
