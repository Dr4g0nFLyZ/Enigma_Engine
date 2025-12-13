{ pkgs ? import <nixpkgs> {} }:

pkgs.clangStdenv.mkDerivation {
  name = "clang-dev-shell";

  buildInputs = with pkgs; [
    cmake
    gdb
    zlib
    glm
    sqlite
    freetype
    glfw
    mesa
    vulkan-loader
    vulkan-headers
  ];

  shellHook = ''
    echo "Entering Enigma_Engine development shell."
    export VULKAN_SDK="${pkgs.vulkan-loader}"
    '';
}
