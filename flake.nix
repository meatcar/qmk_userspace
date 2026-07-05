{
  description = "QMK userspace: Leopold FC660C (hasu) and Cipulot EC660C keymaps";

  inputs = {
    # see docs at https://flake.parts/
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-26.05";
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
            ];
          };
        };
    };
}
