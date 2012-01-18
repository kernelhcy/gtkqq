SUBDIRS= src pixmaps
ACLOCAL_AMFLAGS = -I m4

rpm: dist
	mkdir -p rpmbuild
	cp ${PACKAGE}-${VERSION}.tar.gz rpmbuild
	rpmbuild --define "_sourcedir `pwd`/rpmbuild" \
		--define "_srcrpmdir `pwd`/rpmbuild" \
		--define "_builddir `pwd`/rpmbuild" \
		--define "_rpmdir `pwd`/rpmbuild" \
		-ba ${PACKAGE}.spec
