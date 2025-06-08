{
	stdenv,
	cmake,
	libadt,
	scallop-lang,
	descent-xml,
	wayland,
}:

stdenv.mkDerivation {
	pname = "guish";
	version = "0.0.1";
	src = ./.;

	nativeBuildInputs = [cmake];
	buildInputs = [
		libadt
		scallop-lang
		descent-xml
		wayland
	];

	cmakeFlags = [
		"-DBUILD_TESTING=True"
	];

	doCheck = true;
	checkTarget = "test";
}
