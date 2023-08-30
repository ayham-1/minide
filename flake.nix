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
                        gdb
                        valgrind
                        man
                        man-pages
                        man-pages-posix
                    ];

                    buildInputs = with pkgs; [
                        glfw
                        glew
                        freetype
                        cglm
                    ];
                    shellHook = ''
                        #export PS1="minide> "
                        export PS1='[\#][$?] \[\e[2m\]\t \[\e[0;1;3m\]minide\[\e[0m\]> '
                        alias debug="./scripts/debug.sh"
                        alias release="./scripts/release.sh"
                        alias test="./scripts/test.sh"
                        alias ide="nix run github:ayham-1/ide"
                    '';
            };
        };
    });
}
