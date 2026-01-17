{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "enigma-engine";

  packages = with pkgs; [
    cmake
    pkg-config
    qt6.wrapQtAppsHook
    gcc
    gdb
    zig
    gnumake
    protobuf
    wasmtime
    sqlite-interactive
  ];

  buildInputs = with pkgs; [
    zlib
    sqlite
    freetype
    glfw
    mesa
    vulkan-loader
    vulkan-headers
    curl
    libGL
    qt6.qtbase
    qt6.qttools
    qt6.qtdeclarative
    qt6.qtwebengine
    qt6.qtmultimedia
  ];

  shellHook = ''
    echo "Entering development shell..."
    echo "zig\\\\$(zig version)"
    echo "gcc\\\\$(gcc --version | head -n 1 | awk '{print $3}')"
    echo "wasmtime\\\\$(wasmtime --version | awk '{print $2}')"
    echo "sqlite3\\\\$(sqlite3 --version | awk '{print $1}')"
    export VULKAN_SDK="${pkgs.vulkan-loader}"
    '';
}
