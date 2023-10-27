{
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    mach-nix.url = "mach-nix/3.5.0";
  };
  outputs = { self, nixpkgs, flake-utils, mach-nix }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ ];
          config.allowUnfree = true;
        };
      in
      rec {
        nixConfig.sandbox = "relaxed";
        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [ pkg-config ];

          buildInputs =
            with pkgs;
            [
              meson
              ninja
              gcc13
              clang-tools_16

              (
                mach-nix.lib."${system}".mkPython {
                  requirements = ''
                    quom
                  '';
                }
              )
            ];
        };
      });
}
