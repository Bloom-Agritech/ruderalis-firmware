# Project Ruderalis

Project Ruderalis: The Smart Marijuana Greenhouse

## Welcome to the Project Ruderalis Firmware Repository!

This is where the firmware running Rudy the Greenhouse is located. Over the months Rudy has gone through a Raspberry Pi, Feather Huzzah 32, Particle Photon, and as of March 31, 2020 is running on a Particle Argon.


#### ```/src``` folder:  
This is the source folder that contains the firmware files for the project. It should *not* be renamed. 
Anything that is in this folder when we compile code will be sent to the Particle compile service and compiled into a firmware binary for the Argon.

If the application contains multiple files, they should all be included in the `src` folder. If the firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on the Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for the Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that the project depends on. Dependencies are added automatically to the `project.properties` file when someone adds a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to the project

#### Using multiple sources
If you would like add additional files to the application, they should be added to the `/src` folder. All files in the `/src` folder will be sent to the Particle Cloud to produce a compiled binary.

#### Projects with external libraries
If you want to use a library that has not been registered in the Particle libraries system, create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h`, `.cpp` & `library.properties` files for the library there. Read the [Firmware Libraries guide](https://docs.particle.io/guide/tools-and-features/libraries/) for more details on how to develop libraries. Note that all contents of the `/lib` folder and subfolders will also be sent to the Cloud for compilation.

## Compiling your project

When you're ready to compile, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for the project
- Any libraries stored under `lib/<libraryname>/src`
