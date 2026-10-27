<#
.SYNOPSIS
    Build and run dosdumb.exe inside DOSBox-X on Windows, with a native
    DOS DJGPP and Allegro 4.2.

.DESCRIPTION
    Sets up a DOS C: drive in a work directory and starts DOSBox-X on it:

      C:\DJGPP     DJGPP 2.05 (djdev, gcc, binutils, make) from the DJGPP
                   mirror, plus CWSDPMI
      C:\ALLEGRO   Allegro 4.2 (branch 4.2 from GitHub)
      C:\DUMB      a copy of this DUMB source tree
      C:\RUN       DOSDUMB.EXE, CWSDPMI.EXE, GAME.WAD (the DUMB game),
                   DOOM4DUM.WAD, HTIC4DUM.WAD and, with -Iwad or
                   -Freedoom, DOOM.WAD
      C:\LOGS      build and run logs

    Inside DOSBox-X, C:\BUILD.BAT builds Allegro ("make lib") and DUMB
    ("make -f dos/makefile.dj"), and C:\RUN.BAT starts DOSDUMB.EXE.
    By default it plays the DUMB game (cadwal/dumb-game): its
    prebuilt/game.wad is downloaded, because building it needs netpbm,
    which DOS doesn't have.
    Downloads and the Allegro build are kept, so later runs are quicker.
    Building with GCC under emulated DOS is slow: the first build of
    Allegro can take a long time.

    Downloads go to WORKDIR\dl.  If a download is blocked, put the file
    there yourself (same name) and run the script again.

.PARAMETER WorkDir
    Where to set up the DOS drive.  Default: build-dosbox in the DUMB tree.

.PARAMETER DosBoxX
    Path to dosbox-x.exe.  Default: found in PATH or the usual places.

.PARAMETER Iwad
    Play this Doom IWAD instead of the DUMB game (copied as
    C:\RUN\DOOM.WAD and used with DOOM4DUM.WAD).

.PARAMETER Freedoom
    Like -Iwad, with Freedoom's freedoom1.wad, downloaded from GitHub.

.PARAMETER GameWad
    Use this game.wad (e.g. built with dos/cross-build.sh or by the DUMB
    game's own make) instead of downloading the prebuilt one.

.PARAMETER BuildOnly
    Build, but don't start DOSDUMB.EXE.

.PARAMETER NoDosBox
    Only prepare the work directory; don't start DOSBox-X.

.PARAMETER Rebuild
    Recopy the DUMB sources and rebuild DUMB (and Allegro) from scratch.

.PARAMETER DjgppMirror
    DJGPP mirror; must have the "current" layout (v2/, v2gnu/, v2misc/).

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File dos\dosbox-x-build.ps1

.EXAMPLE
    dos\dosbox-x-build.cmd -Iwad C:\Games\DOOM\DOOM.WAD

.EXAMPLE
    dos\dosbox-x-build.cmd -Freedoom
#>
[CmdletBinding()]
param(
    [string]$WorkDir,
    [string]$DosBoxX,
    [string]$Iwad,
    [switch]$Freedoom,
    [string]$GameWad,
    [switch]$BuildOnly,
    [switch]$NoDosBox,
    [switch]$Rebuild,
    [string]$DjgppMirror = 'https://www.delorie.com/pub/djgpp/current'
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'   # Invoke-WebRequest is slow otherwise
if ($PSVersionTable.PSVersion.Major -lt 6) {
    [Net.ServicePointManager]::SecurityProtocol =
        [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
}

$AllegroZipUrl = 'https://github.com/liballeg/allegro5/archive/refs/heads/4.2.zip'
$GameWadUrl = 'https://raw.githubusercontent.com/cadwal/dumb-game/modernize/prebuilt/game.wad'
$FreedoomUrl = 'https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip'
$CwsdpmiUrls = @(
    "$DjgppMirror/v2misc/csdpmi7b.zip",
    'http://sandmann.dotster.com/cwsdpmi/csdpmi7b.zip',
    'https://ftp.fu-berlin.de/pc/languages/djgpp/current/v2misc/csdpmi7b.zip'
)

# DJGPP packages: directory, file name pattern, and how to turn the
# digits in the name into a sortable version (newest is used).
$DjgppPackages = @(
    @{ Name = 'djdev'; Dir = 'v2';    Pattern = '^djdev(\d+)\.zip$'; Kind = 'plain' },
    @{ Name = 'gcc';   Dir = 'v2gnu'; Pattern = '^gcc(\d+)b\.zip$';  Kind = 'gcc' },
    @{ Name = 'bnu';   Dir = 'v2gnu'; Pattern = '^bnu(\d+)b\.zip$';  Kind = 'binutils' },
    @{ Name = 'make';  Dir = 'v2gnu'; Pattern = '^mak(\d+)b\.zip$';  Kind = 'digits' }
)

$SrcDir = Split-Path -Parent $PSScriptRoot
if (-not $WorkDir) { $WorkDir = Join-Path $SrcDir 'build-dosbox' }
$WorkDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($WorkDir)
$Dl = Join-Path $WorkDir 'dl'
$C = Join-Path $WorkDir 'c'
$DjDir = Join-Path $C 'DJGPP'
$AllegDir = Join-Path $C 'ALLEGRO'
$DumbDir = Join-Path $C 'DUMB'
$RunDir = Join-Path $C 'RUN'
$LogDir = Join-Path $C 'LOGS'

function Say([string]$msg) { Write-Host "dosbox-x-build: $msg" }

function Get-Download([string[]]$Urls, [string]$Name) {
    $dest = Join-Path $Dl $Name
    if (Test-Path $dest) { return $dest }
    foreach ($url in $Urls) {
        Say "downloading $url"
        try {
            Invoke-WebRequest -Uri $url -OutFile "$dest.part" -UseBasicParsing
            Move-Item -Force "$dest.part" $dest
            return $dest
        } catch {
            Say "  failed: $($_.Exception.Message)"
            Remove-Item -Force -ErrorAction SilentlyContinue "$dest.part"
        }
    }
    throw "Could not download $Name. Put it in $Dl yourself and run this again."
}

function Get-VersionKey([string]$Digits, [string]$Kind) {
    # e.g. gcc1220 -> 12.2.0, gcc930 -> 9.3.0, bnu2351 -> 2.35.1,
    # bnu244 -> 2.44, mak44 -> 4.4, mak421 -> 4.2.1
    switch ($Kind) {
        'gcc'      { $parts = @($Digits.Substring(0, $Digits.Length - 2),
                                $Digits.Substring($Digits.Length - 2, 1),
                                $Digits.Substring($Digits.Length - 1)) }
        'binutils' { $parts = @($Digits.Substring(0, 1), $Digits.Substring(1, 2),
                                $Digits.Substring(3)) }
        'digits'   { $parts = $Digits.ToCharArray() | ForEach-Object { [string]$_ } }
        default    { $parts = @($Digits) }
    }
    ($parts | ForEach-Object { if ($_ -eq '') { '000' } else { ([int]$_).ToString('000') } }) -join '.'
}

function Find-DjgppPackage($Pkg) {
    # A file already in dl/ wins, so you can pin versions or work offline.
    $names = @(Get-ChildItem -Path $Dl -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match $Pkg.Pattern } | ForEach-Object { $_.Name })
    if ($names.Count -eq 0) {
        $url = "$DjgppMirror/$($Pkg.Dir)/"
        Say "looking for the newest $($Pkg.Name) in $url"
        $page = Invoke-WebRequest -Uri $url -UseBasicParsing
        $names = @($page.Links | ForEach-Object { Split-Path -Leaf ([string]$_.href) } |
            Where-Object { $_ -match $Pkg.Pattern } | Sort-Object -Unique)
        if ($names.Count -eq 0) { throw "No $($Pkg.Name) package found at $url" }
    }
    $best = $names | Sort-Object -Descending {
        [void]($_ -match $Pkg.Pattern); Get-VersionKey $Matches[1] $Pkg.Kind
    } | Select-Object -First 1
    Get-Download @("$DjgppMirror/$($Pkg.Dir)/$best") $best
}

function Expand-Zip([string]$Zip, [string]$Dest) {
    New-Item -ItemType Directory -Force -Path $Dest | Out-Null
    Expand-Archive -Path $Zip -DestinationPath $Dest -Force
}

function Write-DosFile([string]$Path, [string]$Text) {
    # DOS wants CRLF and plain ASCII.
    $Text = ($Text -replace "`r?`n", "`r`n")
    [IO.File]::WriteAllText($Path, $Text, [Text.Encoding]::ASCII)
}

function Find-DosBoxX {
    if ($DosBoxX) {
        if (-not (Test-Path $DosBoxX)) { throw "No such file: $DosBoxX" }
        return $DosBoxX
    }
    $cmd = Get-Command dosbox-x.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $candidates = @(
        "$env:ProgramFiles\DOSBox-X\dosbox-x.exe",
        "${env:ProgramFiles(x86)}\DOSBox-X\dosbox-x.exe",
        "$env:LOCALAPPDATA\Programs\DOSBox-X\dosbox-x.exe",
        'C:\DOSBox-X\dosbox-x.exe'
    )
    foreach ($p in $candidates) { if ($p -and (Test-Path $p)) { return $p } }
    throw ("DOSBox-X not found. Install it from https://dosbox-x.com/ " +
           "(or: winget install joncampbell123.DOSBox-X) or pass -DosBoxX.")
}

foreach ($d in @($Dl, $C, $RunDir, $LogDir)) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
}

# ---- DJGPP + CWSDPMI --------------------------------------------------------

if (-not (Test-Path (Join-Path $DjDir 'bin\gcc.exe'))) {
    foreach ($pkg in $DjgppPackages) {
        $zip = Find-DjgppPackage $pkg
        Say "unpacking $(Split-Path -Leaf $zip)"
        Expand-Zip $zip $DjDir
    }
}
foreach ($f in @('djgpp.env', 'bin\gcc.exe', 'bin\ld.exe', 'bin\make.exe', 'bin\redir.exe')) {
    if (-not (Test-Path (Join-Path $DjDir $f))) { throw "DJGPP is incomplete: no $f in $DjDir" }
}

$cwsExe = Join-Path $DjDir 'bin\CWSDPMI.EXE'
if (-not (Test-Path $cwsExe)) {
    $zip = Get-Download $CwsdpmiUrls 'csdpmi7b.zip'
    $tmp = Join-Path $Dl 'cwsdpmi'
    Expand-Zip $zip $tmp
    $exe = Get-ChildItem -Path $tmp -Recurse -File -Filter 'CWSDPMI.EXE' | Select-Object -First 1
    if (-not $exe) { throw "csdpmi7b.zip has no CWSDPMI.EXE" }
    Copy-Item $exe.FullName $cwsExe
}
Copy-Item -Force $cwsExe (Join-Path $RunDir 'CWSDPMI.EXE')

# ---- Allegro 4.2 ------------------------------------------------------------

if ($Rebuild -and (Test-Path $AllegDir)) { Remove-Item -Recurse -Force $AllegDir }
if (-not (Test-Path (Join-Path $AllegDir 'makefile.dj'))) {
    $zip = Get-Download @($AllegroZipUrl) 'allegro-4.2.zip'
    $tmp = Join-Path $Dl 'allegro-src'
    if (Test-Path $tmp) { Remove-Item -Recurse -Force $tmp }
    Expand-Zip $zip $tmp
    $top = Get-ChildItem -Path $tmp -Directory | Select-Object -First 1
    Move-Item $top.FullName $AllegDir
    # What "fix.bat djgpp" would do.
    Write-DosFile (Join-Path $AllegDir 'makefile') "# generated by dosbox-x-build.ps1`nMAKEFILE_INC = makefile.dj`ninclude makefile.all`n"
    Write-DosFile (Join-Path $AllegDir 'include\allegro\platform\alplatf.h') "/* generated by dosbox-x-build.ps1 */`n#define ALLEGRO_DJGPP`n"
}

# ---- DUMB sources -----------------------------------------------------------

if ($Rebuild -and (Test-Path $DumbDir)) { Remove-Item -Recurse -Force $DumbDir }
Say "copying the DUMB sources to $DumbDir"
$exclude = @('.git', 'autom4te.cache', 'build-dos', 'build-dosbox', 'dosobj', $WorkDir)
& robocopy $SrcDir $DumbDir /E /XD @exclude /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE)" }

# ---- game data --------------------------------------------------------------

# The DUMB game is always installed; DOOM.WAD only for -Iwad/-Freedoom.
$gameWadDest = Join-Path $RunDir 'GAME.WAD'
if ($GameWad) {
    if (-not (Test-Path $GameWad)) { throw "No such file: $GameWad" }
    Copy-Item -Force $GameWad $gameWadDest
} else {
    Copy-Item -Force (Get-Download @($GameWadUrl) 'game.wad') $gameWadDest
}

$doomWad = Join-Path $RunDir 'DOOM.WAD'
if ($Iwad) {
    if (-not (Test-Path $Iwad)) { throw "No such file: $Iwad" }
    Copy-Item -Force $Iwad $doomWad
} elseif ($Freedoom) {
    $zip = Get-Download @($FreedoomUrl) 'freedoom-0.13.0.zip'
    $tmp = Join-Path $Dl 'freedoom'
    Expand-Zip $zip $tmp
    $wad = Get-ChildItem -Path $tmp -Recurse -File -Filter 'freedoom1.wad' | Select-Object -First 1
    if (-not $wad) { throw "The Freedoom zip has no freedoom1.wad" }
    Copy-Item -Force $wad.FullName $doomWad
} elseif (Test-Path $doomWad) {
    Remove-Item -Force $doomWad
}
# Without -w, DUMB loads DOOM.WAD and DOOM4DUM.WAD.
if (Test-Path $doomWad) { $dumbArgs = '' } else { $dumbArgs = '-w GAME.WAD ' }

# ---- DOS batch files and DOSBox-X configuration -----------------------------

Write-DosFile (Join-Path $C 'BUILD.BAT') @'
@echo off
rem Written by dos\dosbox-x-build.ps1: builds Allegro 4.2 and DUMB.
set DJGPP=C:\DJGPP\DJGPP.ENV
set PATH=C:\DJGPP\BIN;%PATH%
set LFN=y
if exist C:\LOGS\BUILD.OK del C:\LOGS\BUILD.OK

cd \ALLEGRO
if exist lib\djgpp\liballeg.a goto dumb
echo === Building Allegro 4.2 (logs: C:\LOGS\ALLEG*.LOG; this is slow) ===
rem These generated files are not listed as dependencies, so make
rem them first.
redir -o C:\LOGS\ALLEG1.LOG -eo make obj/djgpp/asmcapa.h
redir -o C:\LOGS\ALLEG2.LOG -eo make obj/djgpp/asmdef.inc
if errorlevel 1 goto fail
redir -o C:\LOGS\ALLEG3.LOG -eo make lib
if errorlevel 1 goto fail

:dumb
echo === Building DUMB (log: C:\LOGS\DUMB.LOG) ===
cd \DUMB
redir -o C:\LOGS\DUMB.LOG -eo make -f dos/makefile.dj ALLEGRO=C:/ALLEGRO
if errorlevel 1 goto fail
copy dosobj\dosdumb.exe C:\RUN\DOSDUMB.EXE
copy dosobj\doom4dum.wad C:\RUN\DOOM4DUM.WAD
copy dosobj\htic4dum.wad C:\RUN\HTIC4DUM.WAD
echo ok> C:\LOGS\BUILD.OK
echo === Build finished ===
goto end

:fail
echo *** Build failed; see the logs in C:\LOGS ***
:end
cd \
'@

Write-DosFile (Join-Path $C 'RUN.BAT') @"
@echo off
rem Written by dos\dosbox-x-build.ps1: runs DOSDUMB.EXE.
cd \RUN
echo DUMB writes its log to C:\LOGS\DOSDUMB.LOG
DOSDUMB.EXE $dumbArgs%1 %2 %3 %4 %5 %6 %7 %8 %9 > C:\LOGS\DOSDUMB.LOG
cd \
"@

if ($BuildOnly) {
    $runLines = "echo Built C:\RUN\DOSDUMB.EXE; start it with C:\RUN.BAT.`ngoto end"
} else {
    $runLines = "call C:\RUN.BAT`nexit"
}
Write-DosFile (Join-Path $C 'MAIN.BAT') @"
@echo off
call C:\BUILD.BAT
if not exist C:\LOGS\BUILD.OK goto failed
$runLines
:failed
echo The logs are in $LogDir
:end
"@

$conf = Join-Path $WorkDir 'dosbox-x.conf'
Set-Content -Encoding ASCII -Path $conf -Value @"
# Written by dos\dosbox-x-build.ps1
[dosbox]
memsize=256
machine=svga_s3

[cpu]
core=dynamic
cputype=pentium_mmx
cycles=max

[dos]
ver=7.1
lfn=true

[autoexec]
mount c "$C"
c:
call MAIN.BAT
"@

Say "work directory: $WorkDir"
if ($NoDosBox) {
    Say "start it with: dosbox-x -conf `"$conf`""
    return
}
$exe = Find-DosBoxX
Say "starting $exe"
# Start-Process -Wait, because PowerShell doesn't wait for GUI programs.
Start-Process -FilePath $exe -ArgumentList @('-conf', "`"$conf`"") -Wait
