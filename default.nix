let
	pkgs = import <nixpkgs> {};
	callFromGitHub = gitHubParams:
		let
			path = pkgs.fetchFromGitHub gitHubParams;
			pkg = pkgs.callPackage "${path}/build.nix";
		in
		pkg;
	libadt = callFromGitHub {
		owner = "TheMadman";
		repo = "libadt";
		rev = "6ab226d4f0b5fb91d00d81810b60e82a88f1fd99";
		hash = "sha256-87YhgCmCBvJaZ2w+I1BKILUOSuF8/Isip3FWAPDYZIQ=";
	} {};
	scallop-lang = callFromGitHub {
		owner = "TheMadman";
		repo = "scallop-lang";
		rev = "21835cfa98cba49be6374253498635d53e0ca41d";
		hash = "sha256-zDu136x9Lc2cOrncsVJhj0W69j5bMPIDm0nTPldlqD0=";
	} { inherit libadt; };
	descent-xml = callFromGitHub {
		owner = "TheMadman";
		repo = "descent-xml";
		rev = "12ebb570d9839b3ba3a4e9b47e710f3b9d4e6da0";
		hash = "sha256-zTlU1iZJOHqY37kEjMlgNGarei+8yr9FIGz55jy3Q5Q=";
	} { inherit libadt; };
in
pkgs.callPackage ./build.nix { inherit libadt scallop-lang descent-xml; }
