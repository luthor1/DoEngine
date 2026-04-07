# DoEngine Dependency Setup Script
# This script clones the required thirdparty libraries into the thirdparty/ folder.

$ThirdPartyDir = Join-Path $PSScriptRoot "thirdparty"

if (!(Test-Path $ThirdPartyDir)) {
    New-Item -ItemType Directory -Path $ThirdPartyDir
}

function Clone-Repo($Name, $Url, $Tag) {
    $TargetDir = Join-Path $ThirdPartyDir $Name
    if (!(Test-Path $TargetDir)) {
        Write-Host "[INFO] Cloning $Name (including submodules)..." -ForegroundColor Cyan
        git clone --depth 1 --branch $Tag --recursive --shallow-submodules $Url $TargetDir
    } else {
        Write-Host "[INFO] $Name already exists. To force update, delete the folder first." -ForegroundColor Yellow
    }
}

# 1. GLFW
Clone-Repo "glfw" "https://github.com/glfw/glfw.git" "3.3.10"

# 2. EnTT
Clone-Repo "entt" "https://github.com/skypjack/entt.git" "v3.13.2"

# 3. enkiTS
Clone-Repo "enkits" "https://github.com/dougbinks/enkiTS.git" "v1.11"

# 4. Jolt Physics
Clone-Repo "jolt" "https://github.com/jrouwe/JoltPhysics.git" "v5.2.0"

# 5. NVRHI
Clone-Repo "nvrhi" "https://github.com/NVIDIA-RTX/NVRHI.git" "main"

# 6. miniaudio (Single Header)
$MiniaudioDir = Join-Path $ThirdPartyDir "miniaudio"
if (!(Test-Path $MiniaudioDir)) { New-Item -ItemType Directory -Path $MiniaudioDir }
$MiniaudioHeader = Join-Path $MiniaudioDir "miniaudio.h"
if (!(Test-Path $MiniaudioHeader)) {
    Write-Host "[INFO] Downloading miniaudio.h..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri "https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h" -OutFile $MiniaudioHeader
}

Write-Host "[SUCCESS] Dependency setup complete!" -ForegroundColor Green
