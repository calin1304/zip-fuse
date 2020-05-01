{ pkgs ? import <nixpkgs> {} }:

let
  collectionsc = pkgs.callPackage ./nix/collectionsc.nix {};
in
  pkgs.mkShell {
    buildInputs = with pkgs; [ pkgconfig fuse3 libzip.dev collectionsc ];
  }
