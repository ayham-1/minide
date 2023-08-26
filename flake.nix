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
                    ];

                    buildInputs = with pkgs; [
                        glfw
                        glew
                    ];

                    shellHook = ''
                        export PS1="minide> "
                        rm -rf .build/
                        mkdir .build/
                        cd .build/
                        echo "cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./minide" > debug.sh
                        chmod +x debug.sh

                        echo "cmake -DCMAKE_BUILD_TYPE=Release .. && make && ./minide" > release.sh
                        chmod +x release.sh
                    '';
                };
            };
        });
}
