param(
    [string]$ObjectDirectory = "generated/obj/NoEngine/Debug",
    [string]$BinaryDirectory = "generated/outputs/Debug"
)
$ErrorActionPreference = "Stop"
$originalPath = $env:PATH
$repoRoot = Split-Path $PSScriptRoot -Parent
Push-Location $repoRoot
try {
    # ビルド済みの本体を使い、描画基盤を初期化せず実際の読み込み経路を検証する。
    $sceneObjectRoot = [IO.Path]::GetFullPath([IO.Path]::Combine($repoRoot, $ObjectDirectory))
    $sceneBinaryRoot = [IO.Path]::GetFullPath([IO.Path]::Combine($repoRoot, $BinaryDirectory))
    $testRoot = [IO.Path]::Combine($repoRoot, "generated/tests/scene-manager")
    [IO.Directory]::CreateDirectory($testRoot) | Out-Null
    $testObject = [IO.Path]::Combine($testRoot, "SceneManagerTests.obj")
    $testExe = [IO.Path]::Combine($testRoot, "checks.exe")
    & cl.exe /nologo /std:c++latest /EHsc /MTd /Z7 /utf-8 /D_DEBUG /DUSE_IMGUI /DDEBUG_BUILD /Iproject /Iproject/externals/assimp/include /FIstdafx.h /c tests/SceneManagerTests.cpp "/Fo$testObject"
    if ($LASTEXITCODE -ne 0) { throw "Test compilation failed" }
    $commandFile = [IO.Path]::Combine($sceneObjectRoot, "NoEngine.tlog/link.command.1.tlog")
    $command = (Get-Content -LiteralPath $commandFile -Encoding Unicode | Where-Object { $_ -notmatch '^\^' }) -join " "
    if (!$command) { throw "Build the Debug application before running these tests" }
    $command = $command -replace '(?i)/OUT:"[^"]*"', ('/OUT:"' + $testExe + '"')
    $command = $command -replace '(?i)(?<!\S)(?:"[^"]*[\\/]MAIN\.OBJ"|[^\s"]*[\\/]MAIN\.OBJ)(?=\s|$)', ''
    $command = $command -replace '(?i)/SUBSYSTEM:WINDOWS', '/SUBSYSTEM:CONSOLE'
    $command += ' /DEBUG:NONE /INCREMENTAL:NO "' + $testObject + '"'
    $response = [IO.Path]::Combine($testRoot, "link.rsp")
    [IO.File]::WriteAllText($response, $command, [Text.Encoding]::Unicode)
    Push-Location project
    try { & link.exe "@$response" } finally { Pop-Location }
    if ($LASTEXITCODE -ne 0) { throw "Test linking failed" }
    $env:PATH = "$sceneBinaryRoot;$env:PATH"
    $fixtureRoot = [IO.Path]::Combine($testRoot, [Guid]::NewGuid().ToString("N"))
    & $testExe $fixtureRoot ([IO.Path]::Combine($repoRoot, "project"))
    if ($LASTEXITCODE -ne 0) { throw "Scene regression tests failed" }
} finally {
    $env:PATH = $originalPath
    Pop-Location
}
