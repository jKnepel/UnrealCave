// Copyright 2016-2018 Rodrigo Villani Pereira

using System.IO;
using UnrealBuildTool;

public class NeoKinectUnreal : ModuleRules
{
	public NeoKinectUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        /*
		 * PublicIncludePaths.AddRange(
			new string[] {
				"NeoKinectUnreal/Public"
				// ... add public include paths required here ...
			}
			);
		*/
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"NeoKinectUnreal/Private"
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
                "RHI",
                "RenderCore",
                "D3D11RHI"
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		AddEngineThirdPartyPrivateStaticDependencies(Target,
			new string[]
            {
				// ... add any third party modules that your module depends on here ...
            }
            );

        string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/"));

        string NeoKinectPath = Path.Combine(ThirdPartyPath, "NeoKinect");
		string KinectPath = Path.Combine(ThirdPartyPath, "Kinect");

		PublicSystemLibraryPaths.Add(Path.Combine(NeoKinectPath, "Lib"));
		PublicIncludePaths.Add(Path.Combine(NeoKinectPath, "Inc"));
		PublicSystemLibraryPaths.Add(Path.Combine(KinectPath, "Lib"));
		PublicIncludePaths.Add(Path.Combine(KinectPath, "Inc"));

		// Add static libraries
		PublicSystemLibraries.AddRange(new string[] {
            "NeoKinect.lib",
            "Kinect20.lib",
            "Kinect20.Face.lib"
        });

        // Add dynamic libraries
        PublicDelayLoadDLLs.AddRange(new string[] {
            "Kinect20.Face.dll"
        });
    }
}
