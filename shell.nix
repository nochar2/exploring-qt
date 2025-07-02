with import <nixpkgs> {};
mkShell {
  buildInputs = [
    kdePackages.qtbase
    kdePackages.qtdeclarative
  ];
}
