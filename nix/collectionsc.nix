{ stdenv, fetchFromGitHub, cmake, pkgconfig, cpputest }:

stdenv.mkDerivation {
  name = "Collections-C";
  src = fetchFromGitHub {
    owner = "srdja";
    repo = "Collections-C";
    rev = "775447bfa379417382b2db8454f19039f4d3deb4";
    sha256 = "1yrfflmmd2shi677amxbsbaqb3sksywrsd7mvgaf90jbd1dk69n5";
  };
  buildInputs = [ cmake pkgconfig cpputest ];
  configurePhase = ''
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$out ..
  '';
}
