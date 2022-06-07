# UnrealCAVE

This project was created as part of my bachelor thesis. 
This is a implementation of a CAVE for the Unreal Engine using the nDisplay plugin.
nDisplay creates a cluster that can synchronize multiple Unreal Engine instances.
This project also uses the [UE4-JoyConDriver](https://github.com/jpdante/UE4-JoyConDriver) by jpdante and the [Neo Kinect](https://www.unrealengine.com/marketplace/en-US/product/neo-kinect) plugin by Rodrigo Villani.
This implementation of a CAVE is optimized for the CAVE at the University of Applied Sciences Berlin.

## Prerequisites
- *Unreal Engine 4.27* installed in a location whose filepath does not contain any spaces
- *Kinect v2 SDK* installed
- *.Net Framework 4.6+*, *Unreal Engine installer* and *Nuget targets and build tasks* installed via Visual Studio installer
- for input usign Joy-Cons a pair of Joy-Cons should be connected via Bluetooth
- clone the repository to a location whose filepath does not contain any spaces
- copy *Kinect.Face.h*, *Kinect.h* and *Kinect.INPC.h* from *C:\Program Files\Microsoft SDKs\Kinect\v2.xxx\inc* to *Project Folder\Plugins\NeoKinectUnreal\Source\ThirdParty\Kinect\Inc*
- copy *NuiDatabase* folder and *Kinect20.Face.dll* from *C:\Program Files\Microsoft SDKs\Kinect\v2.xxx\redist\Face\x64* to *Project Folder\Binaries\Win64*

## Starting the Project
- start the *uproject* file
- start the *Switchboard Listender* and *Switchboard Application*
- create a new configuration for the *uproject* file
- click *Add Device* -> *nDisplay* and choose the *Project Folder\Content\Configuration\UnrealCave.uasset* file
- click *Connect All Devices*
- click *Start All Connected Devices* 
