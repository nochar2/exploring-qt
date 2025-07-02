with import <nixpkgs> {};
mkShell {
  buildInputs = [kdePackages.qtbase];
}
