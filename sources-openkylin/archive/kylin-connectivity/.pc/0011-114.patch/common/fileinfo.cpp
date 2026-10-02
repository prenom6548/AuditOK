#include "fileinfo.h"

#include <QMimeDatabase>
#include <QMap>
#include <QDebug>

QMap<QString, QString> FileInfo::mimeMapTable = FileInfo::initMimeMap();
QMap<QString, QString> FileInfo::iconNameMapTable = FileInfo::initIconNameMap();
QSet<QString> FileInfo::imageTypeSet = FileInfo::initimageSet();
QSet<QString> FileInfo::textTypeSet = FileInfo::inittextSet();
QSet<QString> FileInfo::audioTypeSet = FileInfo::initaudioSet();
QSet<QString> FileInfo::videoTypeSet = FileInfo::initvideoSet();

QMap<QString, QString> FileInfo::initMimeMap()
{
    QMap<QString, QString> map;
    map.insert("load", "text/html");
    map.insert("123", "application/vnd.lotus-1-2-3");
    map.insert("3ds", "image/x-3ds");
    map.insert("3g2", "video/3gpp");
    map.insert("3ga", "video/3gpp");
    map.insert("3gp", "video/3gpp");
    map.insert("3gpp", "video/3gpp");
    map.insert("602", "application/x-t602");
    map.insert("669", "audio/x-mod");
    map.insert("7z", "application/x-7z-compressed");
    map.insert("a", "application/x-archive");
    map.insert("aac", "audio/mp4");
    map.insert("abw", "application/x-abiword");
    map.insert("abw.crashed", "application/x-abiword");
    map.insert("abw.gz", "application/x-abiword");
    map.insert("ac3", "audio/ac3");
    map.insert("ace", "application/x-ace");
    map.insert("adb", "text/x-adasrc");
    map.insert("ads", "text/x-adasrc");
    map.insert("afm", "application/x-font-afm");
    map.insert("ag", "image/x-applix-graphics");
    map.insert("ai", "application/illustrator");
    map.insert("aif", "audio/x-aiff");
    map.insert("aifc", "audio/x-aiff");
    map.insert("aiff", "audio/x-aiff");
    map.insert("al", "application/x-perl");
    map.insert("alz", "application/x-alz");
    map.insert("amr", "audio/amr");
    map.insert("ani", "application/x-navi-animation");
    map.insert("anim[1-9j]", "video/x-anim");
    map.insert("anx", "application/annodex");
    map.insert("ape", "audio/x-ape");
    map.insert("arj", "application/x-arj");
    map.insert("arw", "image/x-sony-arw");
    map.insert("as", "application/x-applix-spreadsheet");
    map.insert("asc", "text/plain");
    map.insert("asf", "video/x-ms-asf");
    map.insert("asp", "application/x-asp");
    map.insert("ass", "text/x-ssa");
    map.insert("asx", "audio/x-ms-asx");
    map.insert("atom", "application/atom+xml");
    map.insert("au", "audio/basic");
    map.insert("avi", "video/x-msvideo");
    map.insert("aw", "application/x-applix-word");
    map.insert("awb", "audio/amr-wb");
    map.insert("awk", "application/x-awk");
    map.insert("axa", "audio/annodex");
    map.insert("axv", "video/annodex");
    map.insert("bak", "application/x-trash");
    map.insert("bcpio", "application/x-bcpio");
    map.insert("bdf", "application/x-font-bdf");
    map.insert("bib", "text/x-bibtex");
    map.insert("bin", "application/octet-stream");
    map.insert("blend", "application/x-blender");
    map.insert("blender", "application/x-blender");
    map.insert("bmp", "image/bmp");
    map.insert("bz", "application/x-bzip");
    map.insert("bz2", "application/x-bzip");
    map.insert("c", "text/x-csrc");
    map.insert("c++", "text/x-c++src");
    map.insert("cab", "application/vnd.ms-cab-compressed");
    map.insert("cb7", "application/x-cb7");
    map.insert("cbr", "application/x-cbr");
    map.insert("cbt", "application/x-cbt");
    map.insert("cbz", "application/x-cbz");
    map.insert("cc", "text/x-c++src");
    map.insert("cdf", "application/x-netcdf");
    map.insert("cdr", "application/vnd.corel-draw");
    map.insert("cer", "application/x-x509-ca-cert");
    map.insert("cert", "application/x-x509-ca-cert");
    map.insert("cgm", "image/cgm");
    map.insert("chm", "application/x-chm");
    map.insert("chrt", "application/x-kchart");
    map.insert("class", "application/x-java");
    map.insert("cls", "text/x-tex");
    map.insert("cmake", "text/x-cmake");
    map.insert("cpio", "application/x-cpio");
    map.insert("cpio.gz", "application/x-cpio-compressed");
    map.insert("cpp", "text/x-c++src");
    map.insert("cr2", "image/x-canon-cr2");
    map.insert("crt", "application/x-x509-ca-cert");
    map.insert("crw", "image/x-canon-crw");
    map.insert("cs", "text/x-csharp");
    map.insert("csh", "application/x-csh");
    map.insert("css", "text/css");
    map.insert("cssl", "text/css");
    map.insert("csv", "text/csv");
    map.insert("cue", "application/x-cue");
    map.insert("cur", "image/x-win-bitmap");
    map.insert("cxx", "text/x-c++src");
    map.insert("d", "text/x-dsrc");
    map.insert("dar", "application/x-dar");
    map.insert("dbf", "application/x-dbf");
    map.insert("dc", "application/x-dc-rom");
    map.insert("dcl", "text/x-dcl");
    map.insert("dcm", "application/dicom");
    map.insert("dcr", "image/x-kodak-dcr");
    map.insert("dds", "image/x-dds");
    map.insert("deb", "application/x-deb");
    map.insert("der", "application/x-x509-ca-cert");
    map.insert("desktop", "application/x-desktop");
    map.insert("dia", "application/x-dia-diagram");
    map.insert("diff", "text/x-patch");
    map.insert("divx", "video/x-msvideo");
    map.insert("djv", "image/vnd.djvu");
    map.insert("djvu", "image/vnd.djvu");
    map.insert("dng", "image/x-adobe-dng");
    map.insert("doc", "application/msword");
    map.insert("docbook", "application/docbook+xml");
    map.insert("docm", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    map.insert("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    map.insert("dot", "text/vnd.graphviz");
    map.insert("dsl", "text/x-dsl");
    map.insert("dtd", "application/xml-dtd");
    map.insert("dtx", "text/x-tex");
    map.insert("dv", "video/dv");
    map.insert("dvi", "application/x-dvi");
    map.insert("dvi.bz2", "application/x-bzdvi");
    map.insert("dvi.gz", "application/x-gzdvi");
    map.insert("dwg", "image/vnd.dwg");
    map.insert("dxf", "image/vnd.dxf");
    map.insert("e", "text/x-eiffel");
    map.insert("egon", "application/x-egon");
    map.insert("eif", "text/x-eiffel");
    map.insert("el", "text/x-emacs-lisp");
    map.insert("emf", "image/x-emf");
    map.insert("emp", "application/vnd.emusic-emusic_package");
    map.insert("ent", "application/xml-external-parsed-entity");
    map.insert("eps", "image/x-eps");
    map.insert("eps.bz2", "image/x-bzeps");
    map.insert("eps.gz", "image/x-gzeps");
    map.insert("epsf", "image/x-eps");
    map.insert("epsf.bz2", "image/x-bzeps");
    map.insert("epsf.gz", "image/x-gzeps");
    map.insert("epsi", "image/x-eps");
    map.insert("epsi.bz2", "image/x-bzeps");
    map.insert("epsi.gz", "image/x-gzeps");
    map.insert("epub", "application/epub+zip");
    map.insert("erl", "text/x-erlang");
    map.insert("es", "application/ecmascript");
    map.insert("etheme", "application/x-e-theme");
    map.insert("etx", "text/x-setext");
    map.insert("exe", "application/x-ms-dos-executable");
    map.insert("exr", "image/x-exr");
    map.insert("ez", "application/andrew-inset");
    map.insert("f", "text/x-fortran");
    map.insert("f90", "text/x-fortran");
    map.insert("f95", "text/x-fortran");
    map.insert("fb2", "application/x-fictionbook+xml");
    map.insert("fig", "image/x-xfig");
    map.insert("fits", "image/fits");
    map.insert("fl", "application/x-fluid");
    map.insert("flac", "audio/x-flac");
    map.insert("flc", "video/x-flic");
    map.insert("fli", "video/x-flic");
    map.insert("flv", "video/x-flv");
    map.insert("flw", "application/x-kivio");
    map.insert("fo", "text/x-xslfo");
    map.insert("for", "text/x-fortran");
    map.insert("g3", "image/fax-g3");
    map.insert("gb", "application/x-gameboy-rom");
    map.insert("gba", "application/x-gba-rom");
    map.insert("gcrd", "text/directory");
    map.insert("ged", "application/x-gedcom");
    map.insert("gedcom", "application/x-gedcom");
    map.insert("gen", "application/x-genesis-rom");
    map.insert("gf", "application/x-tex-gf");
    map.insert("gg", "application/x-sms-rom");
    map.insert("gif", "image/gif");
    map.insert("glade", "application/x-glade");
    map.insert("gmo", "application/x-gettext-translation");
    map.insert("gnc", "application/x-gnucash");
    map.insert("gnd", "application/gnunet-directory");
    map.insert("gnucash", "application/x-gnucash");
    map.insert("gnumeric", "application/x-gnumeric");
    map.insert("gnuplot", "application/x-gnuplot");
    map.insert("gp", "application/x-gnuplot");
    map.insert("gpg", "application/pgp-encrypted");
    map.insert("gplt", "application/x-gnuplot");
    map.insert("gra", "application/x-graphite");
    map.insert("gsf", "application/x-font-type1");
    map.insert("gsm", "audio/x-gsm");
    map.insert("gtar", "application/x-tar");
    map.insert("gv", "text/vnd.graphviz");
    map.insert("gvp", "text/x-google-video-pointer");
    map.insert("gz", "application/x-gzip");
    map.insert("h", "text/x-chdr");
    map.insert("h++", "text/x-c++hdr");
    map.insert("hdf", "application/x-hdf");
    map.insert("hh", "text/x-c++hdr");
    map.insert("hp", "text/x-c++hdr");
    map.insert("hpgl", "application/vnd.hp-hpgl");
    map.insert("hpp", "text/x-c++hdr");
    map.insert("hs", "text/x-haskell");
    map.insert("htm", "text/html");
    map.insert("html", "text/html");
    map.insert("hwp", "application/x-hwp");
    map.insert("hwt", "application/x-hwt");
    map.insert("hxx", "text/x-c++hdr");
    map.insert("ica", "application/x-ica");
    map.insert("icb", "image/x-tga");
    map.insert("icns", "image/x-icns");
    map.insert("ico", "image/vnd.microsoft.icon");
    map.insert("ics", "text/calendar");
    map.insert("idl", "text/x-idl");
    map.insert("ief", "image/ief");
    map.insert("iff", "image/x-iff");
    map.insert("ilbm", "image/x-ilbm");
    map.insert("ime", "text/x-imelody");
    map.insert("imy", "text/x-imelody");
    map.insert("ins", "text/x-tex");
    map.insert("iptables", "text/x-iptables");
    map.insert("iso", "application/x-cd-image");
    map.insert("iso9660", "application/x-cd-image");
    map.insert("it", "audio/x-it");
    map.insert("j2k", "image/jp2");
    map.insert("jad", "text/vnd.sun.j2me.app-descriptor");
    map.insert("jar", "application/x-java-archive");
    map.insert("java", "text/x-java");
    map.insert("jng", "image/x-jng");
    map.insert("jnlp", "application/x-java-jnlp-file");
    map.insert("jp2", "image/jp2");
    map.insert("jpc", "image/jp2");
    map.insert("jpe", "image/jpeg");
    map.insert("jpeg", "image/jpeg");
    map.insert("jpf", "image/jp2");
    map.insert("jpg", "image/jpeg");
    map.insert("jpr", "application/x-jbuilder-project");
    map.insert("jpx", "image/jp2");
    map.insert("js", "application/javascript");
    map.insert("json", "application/json");
    map.insert("jsonp", "application/jsonp");
    map.insert("k25", "image/x-kodak-k25");
    map.insert("kar", "audio/midi");
    map.insert("karbon", "application/x-karbon");
    map.insert("kdc", "image/x-kodak-kdc");
    map.insert("kdelnk", "application/x-desktop");
    map.insert("kexi", "application/x-kexiproject-sqlite3");
    map.insert("kexic", "application/x-kexi-connectiondata");
    map.insert("kexis", "application/x-kexiproject-shortcut");
    map.insert("kfo", "application/x-kformula");
    map.insert("kil", "application/x-killustrator");
    map.insert("kino", "application/smil");
    map.insert("kml", "application/vnd.google-earth.kml+xml");
    map.insert("kmz", "application/vnd.google-earth.kmz");
    map.insert("kon", "application/x-kontour");
    map.insert("kpm", "application/x-kpovmodeler");
    map.insert("kpr", "application/x-kpresenter");
    map.insert("kpt", "application/x-kpresenter");
    map.insert("kra", "application/x-krita");
    map.insert("ksp", "application/x-kspread");
    map.insert("kud", "application/x-kugar");
    map.insert("kwd", "application/x-kword");
    map.insert("kwt", "application/x-kword");
    map.insert("la", "application/x-shared-library-la");
    map.insert("latex", "text/x-tex");
    map.insert("ldif", "text/x-ldif");
    map.insert("lha", "application/x-lha");
    map.insert("lhs", "text/x-literate-haskell");
    map.insert("lhz", "application/x-lhz");
    map.insert("log", "text/x-log");
    map.insert("ltx", "text/x-tex");
    map.insert("lua", "text/x-lua");
    map.insert("lwo", "image/x-lwo");
    map.insert("lwob", "image/x-lwo");
    map.insert("lws", "image/x-lws");
    map.insert("ly", "text/x-lilypond");
    map.insert("lyx", "application/x-lyx");
    map.insert("lz", "application/x-lzip");
    map.insert("lzh", "application/x-lha");
    map.insert("lzma", "application/x-lzma");
    map.insert("lzo", "application/x-lzop");
    map.insert("m", "text/x-matlab");
    map.insert("m15", "audio/x-mod");
    map.insert("m2t", "video/mpeg");
    map.insert("m3u", "audio/x-mpegurl");
    map.insert("m3u8", "audio/x-mpegurl");
    map.insert("m4", "application/x-m4");
    map.insert("m4a", "audio/mp4");
    map.insert("m4b", "audio/x-m4b");
    map.insert("m4v", "video/mp4");
    map.insert("mab", "application/x-markaby");
    map.insert("man", "application/x-troff-man");
    map.insert("mbox", "application/mbox");
    map.insert("md", "application/x-genesis-rom");
    map.insert("mdb", "application/vnd.ms-access");
    map.insert("mdi", "image/vnd.ms-modi");
    map.insert("me", "text/x-troff-me");
    map.insert("med", "audio/x-mod");
    map.insert("metalink", "application/metalink+xml");
    map.insert("mgp", "application/x-magicpoint");
    map.insert("mid", "audio/midi");
    map.insert("midi", "audio/midi");
    map.insert("mif", "application/x-mif");
    map.insert("minipsf", "audio/x-minipsf");
    map.insert("mka", "audio/x-matroska");
    map.insert("mkv", "video/x-matroska");
    map.insert("ml", "text/x-ocaml");
    map.insert("mli", "text/x-ocaml");
    map.insert("mm", "text/x-troff-mm");
    map.insert("mmf", "application/x-smaf");
    map.insert("mml", "text/mathml");
    map.insert("mng", "video/x-mng");
    map.insert("mo", "application/x-gettext-translation");
    map.insert("mo3", "audio/x-mo3");
    map.insert("moc", "text/x-moc");
    map.insert("mod", "audio/x-mod");
    map.insert("mof", "text/x-mof");
    map.insert("moov", "video/quicktime");
    map.insert("mov", "video/quicktime");
    map.insert("movie", "video/x-sgi-movie");
    map.insert("mp+", "audio/x-musepack");
    map.insert("mp2", "video/mpeg");
    map.insert("mp3", "audio/mpeg");
    map.insert("mp4", "video/mp4");
    map.insert("mpc", "audio/x-musepack");
    map.insert("mpe", "video/mpeg");
    map.insert("mpeg", "video/mpeg");
    map.insert("mpg", "video/mpeg");
    map.insert("mpga", "audio/mpeg");
    map.insert("mpp", "audio/x-musepack");
    map.insert("mrl", "text/x-mrml");
    map.insert("mrml", "text/x-mrml");
    map.insert("mrw", "image/x-minolta-mrw");
    map.insert("ms", "text/x-troff-ms");
    map.insert("msi", "application/x-msi");
    map.insert("msod", "image/x-msod");
    map.insert("msx", "application/x-msx-rom");
    map.insert("mtm", "audio/x-mod");
    map.insert("mup", "text/x-mup");
    map.insert("mxf", "application/mxf");
    map.insert("n64", "application/x-n64-rom");
    map.insert("nb", "application/mathematica");
    map.insert("nc", "application/x-netcdf");
    map.insert("nds", "application/x-nintendo-ds-rom");
    map.insert("nef", "image/x-nikon-nef");
    map.insert("nes", "application/x-nes-rom");
    map.insert("nfo", "text/x-nfo");
    map.insert("not", "text/x-mup");
    map.insert("nsc", "application/x-netshow-channel");
    map.insert("nsv", "video/x-nsv");
    map.insert("o", "application/x-object");
    map.insert("obj", "application/x-tgif");
    map.insert("ocl", "text/x-ocl");
    map.insert("oda", "application/oda");
    map.insert("odb", "application/vnd.oasis.opendocument.database");
    map.insert("odc", "application/vnd.oasis.opendocument.chart");
    map.insert("odf", "application/vnd.oasis.opendocument.formula");
    map.insert("odg", "application/vnd.oasis.opendocument.graphics");
    map.insert("odi", "application/vnd.oasis.opendocument.image");
    map.insert("odm", "application/vnd.oasis.opendocument.text-master");
    map.insert("odp", "application/vnd.oasis.opendocument.presentation");
    map.insert("ods", "application/vnd.oasis.opendocument.spreadsheet");
    map.insert("odt", "application/vnd.oasis.opendocument.text");
    map.insert("oga", "audio/ogg");
    map.insert("ogg", "video/x-theora+ogg");
    map.insert("ogm", "video/x-ogm+ogg");
    map.insert("ogv", "video/ogg");
    map.insert("ogx", "application/ogg");
    map.insert("old", "application/x-trash");
    map.insert("oleo", "application/x-oleo");
    map.insert("opml", "text/x-opml+xml");
    map.insert("ora", "image/openraster");
    map.insert("orf", "image/x-olympus-orf");
    map.insert("otc", "application/vnd.oasis.opendocument.chart-template");
    map.insert("otf", "application/x-font-otf");
    map.insert("otg", "application/vnd.oasis.opendocument.graphics-template");
    map.insert("oth", "application/vnd.oasis.opendocument.text-web");
    map.insert("otp", "application/vnd.oasis.opendocument.presentation-template");
    map.insert("ots", "application/vnd.oasis.opendocument.spreadsheet-template");
    map.insert("ott", "application/vnd.oasis.opendocument.text-template");
    map.insert("owl", "application/rdf+xml");
    map.insert("oxt", "application/vnd.openofficeorg.extension");
    map.insert("p", "text/x-pascal");
    map.insert("p10", "application/pkcs10");
    map.insert("p12", "application/x-pkcs12");
    map.insert("p7b", "application/x-pkcs7-certificates");
    map.insert("p7s", "application/pkcs7-signature");
    map.insert("pack", "application/x-java-pack200");
    map.insert("pak", "application/x-pak");
    map.insert("par2", "application/x-par2");
    map.insert("pas", "text/x-pascal");
    map.insert("patch", "text/x-patch");
    map.insert("pbm", "image/x-portable-bitmap");
    map.insert("pcd", "image/x-photo-cd");
    map.insert("pcf", "application/x-cisco-vpn-settings");
    map.insert("pcf.gz", "application/x-font-pcf");
    map.insert("pcf.z", "application/x-font-pcf");
    map.insert("pcl", "application/vnd.hp-pcl");
    map.insert("pcx", "image/x-pcx");
    map.insert("pdb", "chemical/x-pdb");
    map.insert("pdc", "application/x-aportisdoc");
    map.insert("pdf", "application/pdf");
    map.insert("pdf.bz2", "application/x-bzpdf");
    map.insert("pdf.gz", "application/x-gzpdf");
    map.insert("pef", "image/x-pentax-pef");
    map.insert("pem", "application/x-x509-ca-cert");
    map.insert("perl", "application/x-perl");
    map.insert("pfa", "application/x-font-type1");
    map.insert("pfb", "application/x-font-type1");
    map.insert("pfx", "application/x-pkcs12");
    map.insert("pgm", "image/x-portable-graymap");
    map.insert("pgn", "application/x-chess-pgn");
    map.insert("pgp", "application/pgp-encrypted");
    map.insert("php", "application/x-php");
    map.insert("php3", "application/x-php");
    map.insert("php4", "application/x-php");
    map.insert("pict", "image/x-pict");
    map.insert("pict1", "image/x-pict");
    map.insert("pict2", "image/x-pict");
    map.insert("pickle", "application/python-pickle");
    map.insert("pk", "application/x-tex-pk");
    map.insert("pkipath", "application/pkix-pkipath");
    map.insert("pkr", "application/pgp-keys");
    map.insert("pl", "application/x-perl");
    map.insert("pla", "audio/x-iriver-pla");
    map.insert("pln", "application/x-planperfect");
    map.insert("pls", "audio/x-scpls");
    map.insert("pm", "application/x-perl");
    map.insert("png", "image/png");
    map.insert("pnm", "image/x-portable-anymap");
    map.insert("pntg", "image/x-macpaint");
    map.insert("po", "text/x-gettext-translation");
    map.insert("por", "application/x-spss-por");
    map.insert("pot", "text/x-gettext-translation-template");
    map.insert("ppm", "image/x-portable-pixmap");
    map.insert("pps", "application/vnd.ms-powerpoint");
    map.insert("ppt", "application/vnd.ms-powerpoint");
    map.insert("pptm", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
    map.insert("pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
    map.insert("ppz", "application/vnd.ms-powerpoint");
    map.insert("prc", "application/x-palm-database");
    map.insert("ps", "application/postscript");
    map.insert("ps.bz2", "application/x-bzpostscript");
    map.insert("ps.gz", "application/x-gzpostscript");
    map.insert("psd", "image/vnd.adobe.photoshop");
    map.insert("psf", "audio/x-psf");
    map.insert("psf.gz", "application/x-gz-font-linux-psf");
    map.insert("psflib", "audio/x-psflib");
    map.insert("psid", "audio/prs.sid");
    map.insert("psw", "application/x-pocket-word");
    map.insert("pw", "application/x-pw");
    map.insert("py", "text/x-python");
    map.insert("pyc", "application/x-python-bytecode");
    map.insert("pyo", "application/x-python-bytecode");
    map.insert("qif", "image/x-quicktime");
    map.insert("qt", "video/quicktime");
    map.insert("qtif", "image/x-quicktime");
    map.insert("qtl", "application/x-quicktime-media-link");
    map.insert("qtvr", "video/quicktime");
    map.insert("ra", "audio/vnd.rn-realaudio");
    map.insert("raf", "image/x-fuji-raf");
    map.insert("ram", "application/ram");
    map.insert("rar", "application/x-rar");
    map.insert("ras", "image/x-cmu-raster");
    map.insert("raw", "image/x-panasonic-raw");
    map.insert("rax", "audio/vnd.rn-realaudio");
    map.insert("rb", "application/x-ruby");
    map.insert("rdf", "application/rdf+xml");
    map.insert("rdfs", "application/rdf+xml");
    map.insert("reg", "text/x-ms-regedit");
    map.insert("rej", "application/x-reject");
    map.insert("rgb", "image/x-rgb");
    map.insert("rle", "image/rle");
    map.insert("rm", "application/vnd.rn-realmedia");
    map.insert("rmj", "application/vnd.rn-realmedia");
    map.insert("rmm", "application/vnd.rn-realmedia");
    map.insert("rms", "application/vnd.rn-realmedia");
    map.insert("rmvb", "application/vnd.rn-realmedia");
    map.insert("rmx", "application/vnd.rn-realmedia");
    map.insert("roff", "text/troff");
    map.insert("rp", "image/vnd.rn-realpix");
    map.insert("rpm", "application/x-rpm");
    map.insert("rss", "application/rss+xml");
    map.insert("rt", "text/vnd.rn-realtext");
    map.insert("rtf", "application/rtf");
    map.insert("rtx", "text/richtext");
    map.insert("rv", "video/vnd.rn-realvideo");
    map.insert("rvx", "video/vnd.rn-realvideo");
    map.insert("s3m", "audio/x-s3m");
    map.insert("sam", "application/x-amipro");
    map.insert("sami", "application/x-sami");
    map.insert("sav", "application/x-spss-sav");
    map.insert("scm", "text/x-scheme");
    map.insert("sda", "application/vnd.stardivision.draw");
    map.insert("sdc", "application/vnd.stardivision.calc");
    map.insert("sdd", "application/vnd.stardivision.impress");
    map.insert("sdp", "application/sdp");
    map.insert("sds", "application/vnd.stardivision.chart");
    map.insert("sdw", "application/vnd.stardivision.writer");
    map.insert("sgf", "application/x-go-sgf");
    map.insert("sgi", "image/x-sgi");
    map.insert("sgl", "application/vnd.stardivision.writer");
    map.insert("sgm", "text/sgml");
    map.insert("sgml", "text/sgml");
    map.insert("sh", "application/x-shellscript");
    map.insert("shar", "application/x-shar");
    map.insert("shn", "application/x-shorten");
    map.insert("siag", "application/x-siag");
    map.insert("sid", "audio/prs.sid");
    map.insert("sik", "application/x-trash");
    map.insert("sis", "application/vnd.symbian.install");
    map.insert("sisx", "x-epoc/x-sisx-app");
    map.insert("sit", "application/x-stuffit");
    map.insert("siv", "application/sieve");
    map.insert("sk", "image/x-skencil");
    map.insert("sk1", "image/x-skencil");
    map.insert("skr", "application/pgp-keys");
    map.insert("slk", "text/spreadsheet");
    map.insert("smaf", "application/x-smaf");
    map.insert("smc", "application/x-snes-rom");
    map.insert("smd", "application/vnd.stardivision.mail");
    map.insert("smf", "application/vnd.stardivision.math");
    map.insert("smi", "application/x-sami");
    map.insert("smil", "application/smil");
    map.insert("sml", "application/smil");
    map.insert("sms", "application/x-sms-rom");
    map.insert("snd", "audio/basic");
    map.insert("so", "application/x-sharedlib");
    map.insert("spc", "application/x-pkcs7-certificates");
    map.insert("spd", "application/x-font-speedo");
    map.insert("spec", "text/x-rpm-spec");
    map.insert("spl", "application/x-shockwave-flash");
    map.insert("spx", "audio/x-speex");
    map.insert("sql", "text/x-sql");
    map.insert("sr2", "image/x-sony-sr2");
    map.insert("src", "application/x-wais-source");
    map.insert("srf", "image/x-sony-srf");
    map.insert("srt", "application/x-subrip");
    map.insert("ssa", "text/x-ssa");
    map.insert("stc", "application/vnd.sun.xml.calc.template");
    map.insert("std", "application/vnd.sun.xml.draw.template");
    map.insert("sti", "application/vnd.sun.xml.impress.template");
    map.insert("stm", "audio/x-stm");
    map.insert("stw", "application/vnd.sun.xml.writer.template");
    map.insert("sty", "text/x-tex");
    map.insert("sub", "text/x-subviewer");
    map.insert("sun", "image/x-sun-raster");
    map.insert("sv4cpio", "application/x-sv4cpio");
    map.insert("sv4crc", "application/x-sv4crc");
    map.insert("svg", "image/svg+xml");
    map.insert("svgz", "image/svg+xml-compressed");
    map.insert("swf", "application/x-shockwave-flash");
    map.insert("sxc", "application/vnd.sun.xml.calc");
    map.insert("sxd", "application/vnd.sun.xml.draw");
    map.insert("sxg", "application/vnd.sun.xml.writer.global");
    map.insert("sxi", "application/vnd.sun.xml.impress");
    map.insert("sxm", "application/vnd.sun.xml.math");
    map.insert("sxw", "application/vnd.sun.xml.writer");
    map.insert("sylk", "text/spreadsheet");
    map.insert("t", "text/troff");
    map.insert("t2t", "text/x-txt2tags");
    map.insert("tar", "application/x-tar");
    map.insert("tar.bz", "application/x-bzip-compressed-tar");
    map.insert("tar.bz2", "application/x-bzip-compressed-tar");
    map.insert("tar.gz", "application/x-compressed-tar");
    map.insert("tar.lzma", "application/x-lzma-compressed-tar");
    map.insert("tar.lzo", "application/x-tzo");
    map.insert("tar.xz", "application/x-xz-compressed-tar");
    map.insert("tar.z", "application/x-tarz");
    map.insert("tbz", "application/x-bzip-compressed-tar");
    map.insert("tbz2", "application/x-bzip-compressed-tar");
    map.insert("tcl", "text/x-tcl");
    map.insert("tex", "text/x-tex");
    map.insert("texi", "text/x-texinfo");
    map.insert("texinfo", "text/x-texinfo");
    map.insert("tga", "image/x-tga");
    map.insert("tgz", "application/x-compressed-tar");
    map.insert("theme", "application/x-theme");
    map.insert("themepack", "application/x-windows-themepack");
    map.insert("tif", "image/tiff");
    map.insert("tiff", "image/tiff");
    map.insert("tk", "text/x-tcl");
    map.insert("tlz", "application/x-lzma-compressed-tar");
    map.insert("tnef", "application/vnd.ms-tnef");
    map.insert("tnf", "application/vnd.ms-tnef");
    map.insert("toc", "application/x-cdrdao-toc");
    map.insert("torrent", "application/x-bittorrent");
    map.insert("tpic", "image/x-tga");
    map.insert("tr", "text/troff");
    map.insert("ts", "application/x-linguist");
    map.insert("tsv", "text/tab-separated-values");
    map.insert("tta", "audio/x-tta");
    map.insert("ttc", "application/x-font-ttf");
    map.insert("ttf", "application/x-font-ttf");
    map.insert("ttx", "application/x-font-ttx");
    map.insert("txt", "text/plain");
    map.insert("txz", "application/x-xz-compressed-tar");
    map.insert("tzo", "application/x-tzo");
    map.insert("ufraw", "application/x-ufraw");
    map.insert("ui", "application/x-designer");
    map.insert("uil", "text/x-uil");
    map.insert("ult", "audio/x-mod");
    map.insert("uni", "audio/x-mod");
    map.insert("uri", "text/x-uri");
    map.insert("url", "text/x-uri");
    map.insert("ustar", "application/x-ustar");
    map.insert("vala", "text/x-vala");
    map.insert("vapi", "text/x-vala");
    map.insert("vcf", "text/directory");
    map.insert("vcs", "text/calendar");
    map.insert("vct", "text/directory");
    map.insert("vda", "image/x-tga");
    map.insert("vhd", "text/x-vhdl");
    map.insert("vhdl", "text/x-vhdl");
    map.insert("viv", "video/vivo");
    map.insert("vivo", "video/vivo");
    map.insert("vlc", "audio/x-mpegurl");
    map.insert("vob", "video/mpeg");
    map.insert("voc", "audio/x-voc");
    map.insert("vor", "application/vnd.stardivision.writer");
    map.insert("vst", "image/x-tga");
    map.insert("wav", "audio/x-wav");
    map.insert("wax", "audio/x-ms-asx");
    map.insert("wb1", "application/x-quattropro");
    map.insert("wb2", "application/x-quattropro");
    map.insert("wb3", "application/x-quattropro");
    map.insert("wbmp", "image/vnd.wap.wbmp");
    map.insert("wcm", "application/vnd.ms-works");
    map.insert("wdb", "application/vnd.ms-works");
    map.insert("webm", "video/webm");
    map.insert("wk1", "application/vnd.lotus-1-2-3");
    map.insert("wk3", "application/vnd.lotus-1-2-3");
    map.insert("wk4", "application/vnd.lotus-1-2-3");
    map.insert("wks", "application/vnd.ms-works");
    map.insert("wma", "audio/x-ms-wma");
    map.insert("wmf", "image/x-wmf");
    map.insert("wml", "text/vnd.wap.wml");
    map.insert("wmls", "text/vnd.wap.wmlscript");
    map.insert("wmv", "video/x-ms-wmv");
    map.insert("wmx", "audio/x-ms-asx");
    map.insert("wp", "application/vnd.wordperfect");
    map.insert("wp4", "application/vnd.wordperfect");
    map.insert("wp5", "application/vnd.wordperfect");
    map.insert("wp6", "application/vnd.wordperfect");
    map.insert("wpd", "application/vnd.wordperfect");
    map.insert("wpg", "application/x-wpg");
    map.insert("wpl", "application/vnd.ms-wpl");
    map.insert("wpp", "application/vnd.wordperfect");
    map.insert("wps", "application/vnd.ms-works");
    map.insert("wri", "application/x-mswrite");
    map.insert("wrl", "model/vrml");
    map.insert("wv", "audio/x-wavpack");
    map.insert("wvc", "audio/x-wavpack-correction");
    map.insert("wvp", "audio/x-wavpack");
    map.insert("wvx", "audio/x-ms-asx");
    map.insert("x3f", "image/x-sigma-x3f");
    map.insert("xac", "application/x-gnucash");
    map.insert("xbel", "application/x-xbel");
    map.insert("xbl", "application/xml");
    map.insert("xbm", "image/x-xbitmap");
    map.insert("xcf", "image/x-xcf");
    map.insert("xcf.bz2", "image/x-compressed-xcf");
    map.insert("xcf.gz", "image/x-compressed-xcf");
    map.insert("xhtml", "application/xhtml+xml");
    map.insert("xi", "audio/x-xi");
    map.insert("xla", "application/vnd.ms-excel");
    map.insert("xlc", "application/vnd.ms-excel");
    map.insert("xld", "application/vnd.ms-excel");
    map.insert("xlf", "application/x-xliff");
    map.insert("xliff", "application/x-xliff");
    map.insert("xll", "application/vnd.ms-excel");
    map.insert("xlm", "application/vnd.ms-excel");
    map.insert("xls", "application/vnd.ms-excel");
    map.insert("xlsm", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    map.insert("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    map.insert("xlt", "application/vnd.ms-excel");
    map.insert("xlw", "application/vnd.ms-excel");
    map.insert("xm", "audio/x-xm");
    map.insert("xmf", "audio/x-xmf");
    map.insert("xmi", "text/x-xmi");
    map.insert("xml", "application/xml");
    map.insert("xpm", "image/x-xpixmap");
    map.insert("xps", "application/vnd.ms-xpsdocument");
    map.insert("xsl", "application/xml");
    map.insert("xslfo", "text/x-xslfo");
    map.insert("xslt", "application/xml");
    map.insert("xspf", "application/xspf+xml");
    map.insert("xul", "application/vnd.mozilla.xul+xml");
    map.insert("xwd", "image/x-xwindowdump");
    map.insert("xyz", "chemical/x-pdb");
    map.insert("xz", "application/x-xz");
    map.insert("w2p", "application/w2p");
    map.insert("z", "application/x-compress");
    map.insert("zabw", "application/x-abiword");
    map.insert("zip", "application/zip");
    // unknown
    map.insert("sr", "image/sr");
    map.insert("apng", "image/apng");
    map.insert("jif", "image/jif");
    map.insert("jfi", "image/jfi");
    map.insert("mpa", "image/mpa");
    return map;
}

QMap<QString, QString> FileInfo::initIconNameMap()
{
    QMap<QString, QString> map;
    map.insert("image", "image-x-generic");
    map.insert("video", "video-x-generic");
    map.insert("audio", "audio-x-generic");
    map.insert("text", "text");
    map.insert("application/vnd.rar", "rar");
    map.insert("application/vnd.android.package-archive", "application-apk");
    map.insert("application/zip", "zip");
    map.insert("application/vnd.debian.binary-package", "application-vnd.debian.binary-package");
    map.insert("application/vnd.openxmlformats-officedocument.wordprocessingml.document",
               "application-vnd.openxmlformats-officedocument.wordprocessingml.document");
    map.insert("application/msword", "application-msword");
    map.insert("application/vnd.ms-works", "application-msword");
    map.insert("application/vnd.ms-excel", "application-vnd.ms-excel");
    map.insert("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
               "application-vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    map.insert("application/vnd.ms-powerpoint", "application-vnd.ms-powerpoint");
    map.insert("application/vnd.openxmlformats-officedocument.presentationml.presentation",
               "application-vnd.openxmlformats-officedocument.presentationml.presentation");
    map.insert("application/octet-stream", "application-octet-stream");
    map.insert("application/java-archive", "application-x-java-archive");
    map.insert("application/x-javascript", "application-x-javascript");
    map.insert("application/x-gzip", "application-gzip");
    map.insert("application/rtf", "application-rtf");
    map.insert("application/x-tar", "application-x-tar");
    map.insert("application/x-compressed", "application-x-compress");
    map.insert("application/x-compress", "application-x-compress");
    map.insert("application/x-zip-compressed", "application-x-compress");
    map.insert("application/pdf", "application-x-pdf");
    map.insert("application/x-bittorrent", "application-x-bittorrent");
    map.insert("application/x-smaf", "audio-x-generic");
    map.insert("inode/directory", "folder");
    return map;
}

QSet<QString> FileInfo::initimageSet()
{
    QSet<QString> set;
    set << "bmp"
        << "gif"
        << "jpg"
        << "jpeg"
        << "png"
        << "pbm"
        << "pgm"
        << "ppm"
        << "xbm"
        << "xpm"
        << "svg"
        << "dds"
        << "icns"
        << "jp2"
        << "mng"
        << "tga"
        << "tiff"
        << "tif"
        << "wbmp"
        << "webp"
        << "sr"
        << "exr"
        << "jpe"
        << "pnm"
        << "ras"
        << "dib"
        << "ico"
        << "j2k"
        << "psd"
        << "jfi"
        << "jif"
        << "apng"
        << "jng";
    return set;
}

QSet<QString> FileInfo::inittextSet()
{
    QSet<QString> set;
    set << "txt"
        << "java"
        << "pot"
        << "conf"
        << "cpp"
        << "c"
        << "h"
        << "htm"
        << "html"
        << "log"
        << "prop"
        << "rc"
        << "sh"
        << "rc"
        << "xml";
    return set;
}

QSet<QString> FileInfo::initaudioSet()
{
    QSet<QString> set;
    set << "mp2"
        << "mp3"
        << "ogg"
        << "oga"
        << "wav"
        << "wma"
        << "aac"
        << "ac3"
        << "dts"
        << "ra"
        << "ape"
        << "flac"
        << "thd"
        << "mka"
        << "m4a"
        << "opus"
        << "m3u"
        << "m4b"
        << "m4p"
        << "mpga"
        << "aif"
        << "mmf"
        << "amr"
        << "m4r"
        << "mid"
        << "wv"
        << "mpa";
    return set;
}

QSet<QString> FileInfo::initvideoSet()
{
    QSet<QString> set;
    set << "avi"
        << "vfw"
        << "divx"
        << "mpg"
        << "mpe"
        << "mpeg"
        << "mpg"
        << "mpg4"
        << "m1v"
        << "m2v"
        << "mpv"
        << "dv"
        << "3gp"
        << "3g2"
        << "mov"
        << "mp4"
        << "m4v"
        << "dat"
        << "vcd"
        << "ogm"
        << "ogv"
        << "ogx"
        << "asf"
        << "wmv"
        << "iso"
        << "vob"
        << "mkv"
        << "nsv"
        << "ram"
        << "flv"
        << "rm"
        << "swf"
        << "ts"
        << "rmvb"
        << "dvr-ms"
        << "m2t"
        << "m2ts"
        << "mts"
        << "rec"
        << "wtv"
        << "f4v"
        << "hdmov"
        << "webm"
        << "vp8"
        << "bik"
        << "smk"
        << "wtv"
        << "part"
        << "m4u"
        << "h264";
    return set;
}
FileInfo::FileInfo()
{
    m_id = 0;
    m_name = "";
    m_path = "";
    m_size = 0;
    m_type = 0;
    m_date = "";
    m_dateTime = "";
    m_thumbnailPath = "";
    m_contentType = "";
}
FileInfo::FileInfo(const FileInfo &fileInfo)
{
    m_id = fileInfo.m_id;
    m_name = fileInfo.m_name;
    m_path = fileInfo.m_path;
    m_size = fileInfo.m_size;
    m_type = fileInfo.m_type;
    m_date = fileInfo.m_date;
    m_dateTime = fileInfo.m_dateTime;
    m_thumbnailPath = fileInfo.m_thumbnailPath;
    m_contentType = fileInfo.m_contentType;
}

FileInfo &FileInfo::operator=(const FileInfo &fileInfo)
{
    if (this == &fileInfo) {
        return *this;
    }

    m_id = fileInfo.m_id;
    m_name = fileInfo.m_name;
    m_path = fileInfo.m_path;
    m_size = fileInfo.m_size;
    m_type = fileInfo.m_type;
    m_date = fileInfo.m_date;
    m_dateTime = fileInfo.m_dateTime;
    m_thumbnailPath = fileInfo.m_thumbnailPath;
    m_contentType = fileInfo.m_contentType;
    return *this;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FileInfo &fileInfo)
{
    argument.beginStructure();
    argument << fileInfo.m_id;
    argument << fileInfo.m_name;
    argument << fileInfo.m_path;
    argument << fileInfo.m_size;
    argument << fileInfo.m_type;
    argument << fileInfo.m_date;
    argument << fileInfo.m_dateTime;
    argument << fileInfo.m_thumbnailPath;
    argument << fileInfo.m_contentType;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FileInfo &fileInfo)
{
    argument.beginStructure();
    argument >> fileInfo.m_id;
    argument >> fileInfo.m_name;
    argument >> fileInfo.m_path;
    argument >> fileInfo.m_size;
    argument >> fileInfo.m_type;
    argument >> fileInfo.m_date;
    argument >> fileInfo.m_dateTime;
    argument >> fileInfo.m_thumbnailPath;
    argument >> fileInfo.m_contentType;
    argument.endStructure();
    return argument;
}

void FileInfo::registerMetaType()
{
    qRegisterMetaType<FileInfo>("FileInfo");
    qDBusRegisterMetaType<FileInfo>();
}

QString FileInfo::name() const
{
    return m_name;
}

void FileInfo::setName(const QString &name)
{
    m_name = name;
    m_type = int(judgmentType(name));
    QString suffix = name.mid(name.lastIndexOf(".") + 1, name.size());
    m_contentType = FileInfo::contentType(suffix);
}

qint64 FileInfo::size() const
{
    return m_size;
}

void FileInfo::setSize(const qint64 &size)
{
    m_size = size;
}

FileInfo::FileType FileInfo::type() const
{
    return FileInfo::FileType(m_type);
}

void FileInfo::setType(FileInfo::FileType type)
{
    m_type = int(type);
}

QString FileInfo::dateTime() const
{
    return m_dateTime;
}

void FileInfo::setDateTime(const QString &dateTime)
{
    m_dateTime = dateTime;
}

QString FileInfo::getThumbnailPath() const
{
    return m_thumbnailPath;
}

void FileInfo::setThumbnailPath(const QString &value)
{
    m_thumbnailPath = value;
}

int FileInfo::getId() const
{
    return m_id;
}

void FileInfo::setId(int value)
{
    m_id = value;
}

QString FileInfo::getPath() const
{
    return m_path;
}

void FileInfo::setPath(const QString &path)
{
    m_path = path;
}

QString FileInfo::getDate() const
{
    return m_date;
}

void FileInfo::setDate(const QString &date)
{
    m_date = date;
}
QString FileInfo::getContentType() const
{
    return m_contentType;
}

FileInfo::FileType FileInfo::judgmentType(QString fileName)
{
    // QMimeDatabase db;
    // QMimeType mime = db.mimeTypeForFile(fileName);
    QString fileType = fileName.mid(fileName.lastIndexOf(".") + 1, fileName.size());
    if (textTypeSet.contains(fileType)) {  // mime.name().startsWith("text/") ||
        // 文本文件 text/plain, text/html, text/css, text/javascript
        return FileType::Txt;
    } else if (imageTypeSet.contains(fileType)) {  // mime.name().startsWith("image/") ||
        // 图片 image/gif, image/png, image/jpeg, image/bmp, image/webp
        return FileType::Image;
    } else if (audioTypeSet.contains(fileType)) {  // mime.name().startsWith("audio/") ||
        // 音频 audio/wav, audio/mpeg, audio/midi, audio/webm, audio/ogg
        return FileType::Audio;
    } else if (videoTypeSet.contains(fileType)) {  // mime.name().startsWith("video/") ||
        // 视频 video/mp4, video/x-flv, video/webm, video/ogg
        return FileType::Video;
    } else if (fileType == "doc" || fileType == "docx" || fileType == "wps") {
        // word doc docx
        return FileType::Doc;
    } else if (fileType == "pdf") {
        // pdf
        return FileType::Pdf;
    } else if (fileType == "xlsx" || fileType == "xls" || fileType == "et") {
        // excel
        return FileType::Excel;
    } else if (fileType == "pptx" || fileType == "dps") {
        // ppt
        return FileType::Ppt;
    } else if (fileType == "zip") {
        // zip
        return FileType::Zip;
    } else {
        // 其他
        return FileType::Other;
    }
}

void FileInfo::fileOpen(QString fileName)
{
    qInfo() << "process open" << fileName;
    QString filePath = QByteArray::fromPercentEncoding(fileName.toUtf8());
    FileInfo::FileType fileType = judgmentType(fileName);
    QString command = "";
    switch (fileType) {
        case FileInfo::FileType::Image:
            command = QString("kylin-photo-viewer");
            break;
        case FileInfo::FileType::Video:
            command = QString("kylin-video");
            break;
        case FileInfo::FileType::Txt:
            command = QString("pluma");
            break;
        case FileInfo::FileType::Audio:
            command = QString("kylin-music");
            break;
        case FileInfo::FileType::Doc:
            command = QString("wps");
            break;
        case FileInfo::FileType::Excel:
            command = QString("et");
            break;
        case FileInfo::FileType::Pdf:
            command = QString("wpspdf");
            break;
        case FileInfo::FileType::Ppt:
            command = QString("wpp");
            break;
        case FileInfo::FileType::Zip:
            command = QString("engrampa");
            break;
        case FileInfo::FileType::Other:
            command = QString("pluma");
            break;
        default:
            command = QString("pluma");
            break;
    }
    QProcess::startDetached(command, QStringList() << filePath);
}

QString FileInfo::contentType(QString flag)
{
    if (!mimeMapTable.contains(flag)) {
        return QString(FILEINFO_CONTENT_TYPE_UNKNOWN);
    }
    return mimeMapTable.find(flag).value();
}

QString FileInfo::iconName(QString flag)
{
    if (!iconNameMapTable.contains(flag)) {
        return QString(FILEINFO_CONTENT_TYPE_UNKNOWN);
    }
    return iconNameMapTable.find(flag).value();
}
