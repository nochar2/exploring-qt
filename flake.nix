{
  inputs = { nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.05"; };
  outputs = { self, nixpkgs, ... }: {
    devShells.x86_64-linux.default =
    with nixpkgs.legacyPackages.x86_64-linux; # this is what 'pkgs' normally is
    mkShell {
      buildInputs = [
        kdePackages.qtbase
        kdePackages.qtdeclarative
      ];
    };
  };
}
