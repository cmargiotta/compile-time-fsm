(use-modules (guix packages))

(packages->manifest (map specification->package '("gcc-toolchain@15.1.0"
						  "make"
						  "meson"
						  "ninja"
						  "clang@20.1.5")))
