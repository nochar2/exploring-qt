{ pkgs ? import <nixpkgs> {} }:
# {mkShell,qt6,libglvnd}:
pkgs.mkShell {
# pkgs.stdenv.mkDerivation {
  buildInputs = with pkgs; [
    cmake
    kdePackages.qtbase
    kdePackages.qtdeclarative
    kdePackages.qtquick3d
    # qt6.full
    # libglvnd
  ];
}
