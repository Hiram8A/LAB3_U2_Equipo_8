# invoke SourceDir generated makefile for release.pem4f
release.pem4f: .libraries,release.pem4f
.libraries,release.pem4f: package/cfg/release_pem4f.xdl
	$(MAKE) -f C:\Users\N5050\workspace_v12_Labs\LAB3_U2_Equipo_8\Aux_files/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\N5050\workspace_v12_Labs\LAB3_U2_Equipo_8\Aux_files/src/makefile.libs clean

