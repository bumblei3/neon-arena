; NeonArena Windows Installer
; Kompilieren mit Inno Setup (https://jrsoftware.org/isdl.php)
; Verwendung: iscc neonarena.iss

[Setup]
AppId={{B9E3C7A2-4F1E-4B8D-9C6A-7D2E5F8A1B3C}
AppName=NeonArena
AppVersion=0.53
AppPublisher=bumblei3
AppPublisherURL=https://github.com/bumblei3/neon-arena
AppSupportURL=https://github.com/bumblei3/neon-arena/issues
AppUpdatesURL=https://github.com/bumblei3/neon-arena/releases
DefaultDirName={autopf}\NeonArena
DefaultGroupName=NeonArena
AllowNoIcons=yes
LicenseFile=LICENSE
OutputDir=installer
OutputBaseFilename=NeonArena-0.53-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
SetupIconFile=assets\neonarena.ico
UninstallDisplayIcon={app}\neonarena.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; Engine (Quake3e)
Source: "engine\quake3e.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "engine\renderer_opengl2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "engine\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion

; Mod-Dateien
Source: "dist\neonarena.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion
Source: "dist\neonarena-qvm.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion
Source: "dist\neon-look.pk3"; DestDir: "{app}\neonarena"; Flags: ignoreversion

; Start-Skript
Source: "scripts\neonarena.bat"; DestDir: "{app}"; Flags: ignoreversion

; Dokumentation
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion; DestName: "README.txt"
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\NeonArena"; Filename: "{app}\neonarena.bat"; WorkingDir: "{app}"; IconFilename: "{app}\quake3e.exe"
Name: "{group}\NeonArena (Engine)"; Filename: "{app}\quake3e.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,NeonArena}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\NeonArena"; Filename: "{app}\neonarena.bat"; WorkingDir: "{app}"; IconFilename: "{app}\quake3e.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\neonarena.bat"; Description: "{cm:LaunchProgram,NeonArena}"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeSetup(): Boolean;
begin
  Result := true;
end;
