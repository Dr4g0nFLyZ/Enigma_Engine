# shell.nix
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "clang-dev";

   nativeBuildInputs = [
      pkgs.pkg-config
   ];

   buildInputs = with pkgs; [
      gnumake
      mesa
      clang
      glfw
      glm
      sqlite
      freetype
      curl.dev
      vulkan-tools
      vulkan-loader
      vulkan-headers
      vulkan-validation-layers
   ];

   shellHook = ''
      echo "Entering Clang development shell ($(clang --version))"
   '';
}
