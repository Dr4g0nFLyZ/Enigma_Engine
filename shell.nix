{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
   nativeBuildInputs = [
      pkgs.pkg-config
   ];

   buildInputs = with pkgs; [
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
}
