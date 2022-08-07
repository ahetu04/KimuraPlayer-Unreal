# Kimura Player - Unreal plugin

**August 6th, 2022**

This is a plugin for the playback of alembic content using the Kimura Player library. It is the most complete implementation of the Kimura Player so far. The plugin is compatible with both 4.27 and 5.1, but not 5.0 (preview). 

# Documentation
Check out [Documentation/KimuraPlugin.md](/Documentation/KimuraPlugin.md).

# Kimura Converter and Visual Studio
The Kimura Converter module requires /Source/KimuraConverter/Libraries/x64/Release/KimuraConverter.lib to be present. KimuraConverter.lib was compiled using the Visual Studio 2022 toolchain and might not properly link against versions of Unreal Engine that were compiled using the Visual Studio 2019 toolchains. If you need to work with Visual Studio 2019, the standalone projects (see below) are available and can be used to rebuild the library using the toolchain of your choice.

# Standalone libraries
Standalone libraries and executables (converter and player) can be found in a separate repository: [Kimura Player](https://github.com/ahetu04/KimuraPlayer)

# Notices
All content and source code for this package are subject to the terms of the [MIT License](LICENSE.md).

The KimuraConverter library and executable (AbcToKimura) contain dependencies to
-  [Alembic 1.7.16 LICENSE](https://github.com/ahetu04/KimuraPlayer/tree/master/ThirdParty/Alembic/alembic-1.7.16/LICENSE.txt)
-  [OpenEXR 2.5.2 LICENSE](https://github.com/ahetu04/KimuraPlayer/tree/master/ThirdParty/Alembic/openexr-2.5.2/LICENSE.md)
-  [zlib 1.2.11 README](https://github.com/ahetu04/KimuraPlayer/tree/master/ThirdParty/Alembic/zlib-1.2.11/README)
-  [DirectXTex LICENSE](https://github.com/ahetu04/KimuraPlayer/tree/master/ThirdParty/DirectXTex/LICENSE)

# Credits
The Kimura Player libraries and Kimura Player plugin for Unreal were created by Alexandre Hetu.