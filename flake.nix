{
  description = "changeme";

  inputs = {
    # see docs at https://flake.parts/
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-25.11";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      flake = { };
      systems = [ "x86_64-linux" ];
      perSystem =
        { pkgs, ... }:
        {
          legacyPackages = pkgs;
          devShells.default = pkgs.mkShell {
            name = "fc660c-hasu";
            buildInputs = with pkgs; [
              qmk
              dos2unix
              via
            ];
          };
        };
    };
}
