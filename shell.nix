{ pkgs ? import <nixpkgs> {} }:
# {mkShell,qt6,libglvnd}:
pkgs.mkShell {
# pkgs.stdenv.mkDerivation {
  buildInputs = with pkgs; [
    cmake
    bear
    kdePackages.qtbase
    kdePackages.qtdeclarative
    kdePackages.qtquick3d
    clang-tools
    # qt6.full
    # libglvnd
  ];
}
