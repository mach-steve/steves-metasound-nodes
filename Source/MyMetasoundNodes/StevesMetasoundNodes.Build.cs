// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StevesMetasoundNodes : ModuleRules
{
	public StevesMetasoundNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
				"SignalProcessing",
				"AudioExtensions",
				"MetasoundEngine",
				"MetasoundFrontend",
				"MetasoundEditor",
				"MetasoundGraphCore",
				"Harmonix",
				"HarmonixDsp",
				

			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        //PrivateDefinitions.AddRange(
        //        new string[]
        //        {
        //            "METASOUND_PLUGIN=Metasound",
        //            "METASOUND_MODULE=MetasoundStandardNodes"
        //        });
    }
}
