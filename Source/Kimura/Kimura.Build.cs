
using System.IO;

using UnrealBuildTool;

public class Kimura : ModuleRules
{
	public Kimura(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		//this.bUseUnity = false;
		//this.CppStandard = CppStandardVersion.Cpp17;
		//this.OptimizeCode = CodeOptimization.Never;

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
				"RHI",
				"Engine",

				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RenderCore",
				"Projects", 
				"Renderer"
				// ... add private dependencies that you statically link with here ...	
			}
			);


		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"MovieRenderPipelineCore",
					"MovieRenderPipelineEditor"
				});
		}


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// access to KimuraAlembic.h and its libs
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Libraries", "Include"));
		}

		PrivateDefinitions.Add("KIMURA_UNREAL");

	}
}
