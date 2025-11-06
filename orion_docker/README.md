# Docker and container options for using ORION Project

Here you can find a compilation on how to use the ORION robot with Docker and other tools in case you do not have a native ROS 2 Jazzy installation. You can find the next options:

- [Using Docker and DevContainers](#using-docker-and-devcontainers)
- [Using Flatboat](#using-flatboat)

## Using Docker and DevContainers

This option is based on a modified ROS 2 Jazzy image with additional set up for using ORION, while also being managed by a devcontainer which allows the configuration of graphics, ports and workspace. Consider the elements present in the **.devcontainer** directory.

### Usage

1. Make sure you have installed **Docker**, consider the [official guidelines](https://docs.docker.com/engine/install/ubuntu/)

2. Check that you have **VS Code** installed, and install the [Docker](https://marketplace.visualstudio.com/items/?itemName=ms-azuretools.vscode-docker) and [Dev Containers](https://marketplace.visualstudio.com/items/?itemName=ms-vscode-remote.remote-containers) extensions.

3. Clone the ORION Common project.

    ~~~bash
    git clone https://github.com/Tesis-ORION/orion_common.git
    ~~~

4. Separate the **orion_docker** directory from any ROS 2 Workspace, for example, place it in your HOME or DOCUMENTS path.

    ~~~bash
    mv .orion_common/orion_docker ~
    ~~~

5. Change the ownership of the directory to the *1010** UID (or the User ID you will set in your container).

    ~~~bash
    sudo chown -R dan1620:dan1620 orion_docker
    ~~~

6. Open with Visual Studio Code

    ~~~bash
    code ~/orion_docker
    ~~~

7. Launch the command (Ctrl + Shift + P) to rebuild and open in container: **Dev Containers: Rebuild and reopen in Container**.

8. This will create a new VS Code Window, you can check the logs. Once it is done, clic enter on the **PostCreateCommand** tab that appears, this will close it and open a new terminal inside container.

9. You are now ready to use ORION in a container, in the **src** dir you should be able to see the ORION packages.

## Using Flatboat

For more information check the [Flatboat Project by JuanCSUCoder](https://github.com/JuanCSUCoder/FlatBoatProject)
Usage with ORION robot will come soon...
