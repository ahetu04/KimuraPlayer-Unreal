
using System.IO;

using UnrealBuildTool;

public class KimuraConverter : ModuleRules
{
	public KimuraConverter(ReadOnlyTargetRules Target) : base(Target)
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
				"RHI",
				"RenderCore",
				"InputCore",
				"EditorStyle",
				"UnrealEd",
				"DesktopWidgets",
				"MainFrame",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// access to IKimuraConverter.h and its libs
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Libraries", "Include"));
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				switch (Target.WindowsPlatform.Compiler)
                {
					case WindowsCompiler.VisualStudio2019:
                    {
						PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Libraries", "x64", "Release", "KimuraConverter2019.lib"));
						break;
                    }

					case WindowsCompiler.VisualStudio2022:
					{
						PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Libraries", "x64", "Release", "KimuraConverter2022.lib"));
						break;
					}

					default:
					{
						break;
					}
				}
			}
		}
	}
}
