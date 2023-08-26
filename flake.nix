{
    description = "minide";

    inputs = {
        nixpkgs.url = "nixpkgs/nixos-23.05";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, flake-utils }: 
        flake-utils.lib.eachDefaultSystem (system : let 
            pkgs = import nixpkgs { inherit system; };
        in {
        packages = {
            default = pkgs.stdenv.mkDerivation {
                    name = "minide";
                    src = ./.;

                    nativeBuildInputs = with pkgs; [
                        clang
                        cmake
                        ninja
                    ];

                    buildInputs = with pkgs; [
                        glfw
                        glew
                    ];
                    shellHook = ''
                        export PS1="minide> "
                        alias debug="rm -rf .build/ && cmake -B.build -DCMAKE_BUILD_TYPE=Debug -GNinja && ninja -C .build && ./.build/minide"
                        alias release="rm -rf .build/ && cmake -B.build -DCMAKE_BUILD_TYPE=Release -GNinja && ninja -C .build && ./.build/minide"
                        alias ide="nix run github:ayham-1/ide"
                    '';
            };
        };
    });
}
