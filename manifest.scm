(use-modules (guix packages)
	     (guix download)
             (gnu packages embedded)
	     (gnu packages compression)
             (gnu packages version-control)
	     (gnu packages gcc)
	     (gnu packages ncurses)
	     (guix inferior)
	     (guix channels)
	     (guix licenses)
	     (srfi srfi-1)
	     (nonguix build-system binary))

(packages->manifest (map specification->package '("gcc-toolchain@15.1.0"
						  "make"
						  "meson"
						  "ninja"
						  "clang@20.1.5")))
