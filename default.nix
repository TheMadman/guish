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
		rev = "e747b605e5c402ed7247f050e5d085e4ab2abd76";
		hash = "sha256-IcF8d1Ey3pSwFH8voOiX1pviwvzuXrEwkPB8F59f4F8=";
	} { inherit libadt; };
in
pkgs.callPackage ./build.nix { inherit libadt scallop-lang descent-xml; }
