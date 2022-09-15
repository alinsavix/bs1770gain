# according to the automake manual:
#   Automake generates Makefiles that work depth-first in postfix order
SUBDIRS=\
libpbutil\
lib1770-2\
libff\
libbg
#libffsox-2
#bs1770gain

if FF_DYNLOAD
#dist-hook:
#	mkdir -p $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVUTIL_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(SWRESAMPLE_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(SWSCALE_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVCODEC_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVFORMAT_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(POSTPROC_SO) $(distdir)/bs1770gain-tools
#	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVFILTER_SO) $(distdir)/bs1770gain-tools
install-exec-hook:
	mkdir -p $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVUTIL_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(SWRESAMPLE_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(SWSCALE_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVCODEC_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVFORMAT_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(POSTPROC_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
	cp -p $(FFMPEG_PREFIX)/$(SODIR)/$(AVFILTER_SO) $(DESTDIR)$(bindir)/bs1770gain-tools
uninstall-hook:
	rm -rf $(DESTDIR)$(bindir)/bs1770gain-tools
endif
