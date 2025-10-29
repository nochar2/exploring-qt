{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib";
  # LD_LIBRARY_PATH=/run/current-system/opengl/lib;
  buildInputs = with pkgs; [
    # cmake
    # bear
    # kdePackages.qtbase
    # kdePackages.qtdeclarative
    # kdePackages.qtcharts    
    # kdePackages.qtgraphs
    # kdePackages.full
    qt6.full
    
    # kdePackages.qtquick3d
    # clang-tools
    # qt6.full
    gammaray

    # otherwise, CMake doesn't build (it needs "wrapopengl")
    libglvnd

    libGL
    libGLU
    glfw
    glm
    lua
    mesa-gl-headers

    mesa-demos
  ];
  # nativeBuildInputs = [wrapQtAppsHook];
}
