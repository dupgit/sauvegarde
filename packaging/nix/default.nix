{ stdenv, fetchurl, pkgconfig, intltool, glib, sqlite, jansson, 
  libmicrohttpd, curl}:

stdenv.mkDerivation rec {
  name = "cdpfgl-${version}";
  version = "0.0.11"; 

  buildInputs = [ pkgconfig intltool glib sqlite jansson libmicrohttpd curl ]; 
  outputs = [ "out" "dev" "man" ];

  src = fetchurl {
    url = "http://cdpfgl.delhomme.org/download/releases/${name}.tar.xz";
    sha256 = "16ablsb2hnni0d28y73w1gk8j8gsmyd5rcjxx4n7zdv5ph8fz5bd";
  };
  
  meta = with stdenv.lib; {
    homepage = https://github.com/dupgit/sauvegarde;
    description = "Continuous data protection for GNU/Linux (cdpfgl).";
    maintainers = "Olivier Delhomme";
    license = licenses.gpl3;
    platforms = platforms.linux;
  };
}
