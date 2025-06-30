$ErrorActionPreference = "Stop"

# CONFIG
$msysRoot = "C:\msys64"
$msysInstaller = "$env:TEMP\msys2-x86_64.exe"
$msysURL = "https://github.com/msys2/msys2-installer/releases/download/2025-06-22/msys2-x86_64-20250622.exe"
$bashScriptName = "update_and_install.sh"
$repoURL = "https://github.com/1emvr/rvm64.git"
$repoTarget = "rvm64"  # folder inside MSYS2 ~/

# Download MSYS2
if (!(Test-Path $msysInstaller)) {
    Write-Host "Downloading MSYS2 installer..."
    Invoke-WebRequest -Uri $msysURL -OutFile $msysInstaller
}

# Install MSYS2
if (!(Test-Path $msysRoot)) {
    Write-Host "Installing MSYS2 to $msysRoot..."
    Start-Process -FilePath $msysInstaller -ArgumentList "--root $msysRoot --script" -Wait
} else {
    Write-Host "MSYS2 already installed at $msysRoot."
}

# Write Bash install script
$bashScriptContent = @"
#!/usr/bin/bash
set -e

echo "Updating system..."
pacman -Syuu --noconfirm || true
pacman -Syuu --noconfirm || true

echo "Installing base tools and compilers..."
pacman -S --noconfirm base-devel git mingw-w64 clang++

# Install musl-cross-make
echo "Installing musl-cross toolchain from source..."
cd ~
git clone https://github.com/kraj/musl.git
cd musl

# Create a config.mak for static RISC-V or x86_64 targets
cat <<EOF > config.mak
TARGET = x86_64-linux-musl
OUTPUT = /opt/musl
COMMON_CONFIG += --disable-nls
EOF

make -j$(nproc)
make install

echo 'export PATH=/opt/musl/bin:\$PATH' >> ~/.bashrc
echo "musl toolchain installed to /opt/musl"
"@
$bashScriptPath = Join-Path $msysRoot $bashScriptName
Set-Content -Path $bashScriptPath -Value $bashScriptContent -Encoding ASCII
Write-Host "Created bash installer script at: $bashScriptPath"

# Run the script inside MSYS2
Write-Host "Running toolchain installation..."
& "$msysRoot\usr\bin\bash.exe" "--login" "-c" "bash /$bashScriptName"

# Clone the repo
$repoCloneCommand = @"
cd \$HOME
rm -rf $repoTarget
git clone $repoURL $repoTarget
"@
Write-Host "Cloning your repository into MSYS2 home..."
& "$msysRoot\usr\bin\bash.exe" "--login" "-c" "$repoCloneCommand"

Write-Host "`nDone! MSYS2 is installed, toolchains are ready, and your repo is cloned."
