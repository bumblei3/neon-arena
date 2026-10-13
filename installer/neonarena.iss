; NeonArena Windows Installer
; Kompilieren mit Inno Setup (https://jrsoftware.org/isdl.php)
; Verwendung: iscc /DMyAppVersion=0.91 neonarena.iss
;
; Erwartetes Layout (wie vom CI-Portable-ZIP):
;   engine/quake3e.exe, engine/quake3e_opengl_x86_64.dll,
;   engine/quake3e_vulkan_x86_64.dll, engine/SDL2*.dll,
;   engine/baseoa/  (OpenArena maps/content)
;   neonarena/      (neonarena.pk3, neonarena-qvm.pk3, neon-look.pk3)
;   neonarena.bat

#ifndef MyAppVersion
#define MyAppVersion "0.91"
#endif

[Setup]
AppId={{B9E3C7A2-4F1E-4B8D-9C6A-7D2E5F8A1B3C}
AppName=NeonArena
AppVersion={#MyAppVersion}
AppPublisher=bumblei3
AppPublisherURL=https://github.com/bumblei3/neon-arena
AppSupportURL=https://github.com/bumblei3/neon-arena/issues
AppUpdatesURL=https://github.com/bumblei3/neon-arena/releases
DefaultDirName={autopf}\NeonArena
DefaultGroupName=NeonArena
AllowNoIcons=yes
OutputDir=.
OutputBaseFilename=NeonArena-{#MyAppVersion}-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\quake3e.exe
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Engine (Quake3e, MinGW cross-build from CI)
Source: "engine\quake3e.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "engine\quake3e_opengl_x86_64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "engine\quake3e_vulkan_x86_64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: FileExists(ExpandConstant('{src}\engine\quake3e_vulkan_x86_64.dll'))
Source: "engine\SDL2*.dll"; DestDir: "{app}"; Flags: ignoreversion

; OpenArena base content (maps, textures — engine expects baseoa\)
Source: "engine\baseoa\*"; DestDir: "{app}\baseoa"; Flags: ignoreversion recursesubdirs createallsubdirs

; Mod files (QVM pk3s — portable across engines)
Source: "neonarena\neonarena.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion
Source: "neonarena\neonarena-qvm.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion
Source: "neonarena\neon-look.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion

; Launcher
Source: "neonarena.bat"; DestDir: "{app}"; Flags: ignoreversion

; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion; DestName: "README.txt"

[Icons]
Name: "{group}\NeonArena"; Filename: "{app}\neonarena.bat"; WorkingDir: "{app}"; IconFilename: "{app}\quake3e.exe"
Name: "{group}\NeonArena (Engine)"; Filename: "{app}\quake3e.exe"; Parameters: "+set fs_game neonarena"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,NeonArena}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\NeonArena"; Filename: "{app}\neonarena.bat"; WorkingDir: "{app}"; IconFilename: "{app}\quake3e.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\neonarena.bat"; Description: "{cm:LaunchProgram,NeonArena}"; Flags: nowait postinstall skipifsilent
