let
	pkgs = import <nixpkgs> {};
	descent-xml = import ./.;
in
pkgs.mkShell {
	inputsFrom = [descent-xml];
	nativeBuildInputs = [pkgs.gdb pkgs.graphviz pkgs.doxygen];
	shellHook = ''
		export CFLAGS='-Wall -Wextra -Wshadow -fsanitize=address -fsanitize=leak -fsanitize=undefined'
	'';
}
