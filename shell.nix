{ pkgs ? import <nixpkgs> {} }:
# {mkShell,qt6,libglvnd}:
pkgs.mkShell {
# pkgs.stdenv.mkDerivation {
  buildInputs = [
    # kdePackages.qtbase
    # kdePackages.qtdeclarative
    # kdePackages.qtquick3d
    pkgs.qt6.full
    pkgs.libglvnd
  ];
}
