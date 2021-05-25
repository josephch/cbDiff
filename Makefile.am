AM_CPPFLAGS = $(WX_CXXFLAGS) \
		-I$(top_srcdir)/src/include \
		-I$(top_srcdir)/src/sdk/wxscintilla/include \
		-I./dtl


pluginlibdir = $(pkglibdir)/plugins

pluginlib_LTLIBRARIES = libcbDiff.la

libcbDiff_la_LDFLAGS = @MODULE_SHARED_LDFLAGS@ -version-info 0:1:0 -no-undefined -avoid-version

libcbDiff_la_LIBADD = ../../../sdk/libcodeblocks.la $(WX_LIBS)

libcbDiff_la_SOURCES = src/cbDiff.cpp \
		src/cbDiffConfigPanel.cpp \
		src/cbDiffCtrl.cpp \
		src/cbDiffEditor.cpp \
		src/cbDiffMenu.cpp \
		src/cbDiffSelectFiles.cpp \
		src/cbDiffUtils.cpp \
		src/cbSideBySideCtrl.cpp \
		src/cbTableCtrl.cpp \
		src/cbUnifiedCtrl.cpp \
		src/wxDiff.cpp

noinst_HEADERS = src/cbDiff.h \
		src/cbDiffConfigPanel.h \
		src/cbDiffCtrl.h \
		src/cbDiffEditor.h \
		src/cbDiffMenu.h \
		src/cbDiffSelectFiles.h \
		src/cbDiffUtils.h \
		src/cbSideBySideCtrl.h \
		src/cbTableCtrl.h \
		src/cbUnifiedCtrl.h \
		src/wxDiff.h \
		dtl/dtl/Diff.hpp \
		dtl/dtl/Diff3.hpp \
		dtl/dtl/dtl.hpp \
		dtl/dtl/functors.hpp \
		dtl/dtl/Lcs.hpp \
		dtl/dtl/Sequence.hpp \
		dtl/dtl/Ses.hpp \
		dtl/dtl/variables.hpp

resources_forZIP = manifest.xml \
		images/16x16/copyLeft.png \
		images/16x16/copyLeft_next.png \
		images/16x16/copyRight.png \
		images/16x16/copyRight_next.png \
		images/16x16/firstDifference.png \
		images/16x16/lastDifference.png \
		images/16x16/nextDifference.png \
		images/16x16/previousDifference.png \
		images/16x16/reload.png \
		images/16x16/sidebyside.png \
		images/16x16/swap.png \
		images/16x16/table.png \
		images/16x16/unified.png \
		images/20x20/copyLeft.png \
		images/20x20/copyLeft_next.png \
		images/20x20/copyRight.png \
		images/20x20/copyRight_next.png \
		images/20x20/firstDifference.png \
		images/20x20/lastDifference.png \
		images/20x20/nextDifference.png \
		images/20x20/previousDifference.png \
		images/20x20/reload.png \
		images/20x20/sidebyside.png \
		images/20x20/swap.png \
		images/20x20/table.png \
		images/20x20/unified.png \
		images/24x24/copyLeft.png \
		images/24x24/copyLeft_next.png \
		images/24x24/copyRight.png \
		images/24x24/copyRight_next.png \
		images/24x24/firstDifference.png \
		images/24x24/lastDifference.png \
		images/24x24/nextDifference.png \
		images/24x24/previousDifference.png \
		images/24x24/reload.png \
		images/24x24/sidebyside.png \
		images/24x24/swap.png \
		images/24x24/table.png \
		images/24x24/unified.png \
		images/28x28/copyLeft.png \
		images/28x28/copyLeft_next.png \
		images/28x28/copyRight.png \
		images/28x28/copyRight_next.png \
		images/28x28/firstDifference.png \
		images/28x28/lastDifference.png \
		images/28x28/nextDifference.png \
		images/28x28/previousDifference.png \
		images/28x28/reload.png \
		images/28x28/sidebyside.png \
		images/28x28/swap.png \
		images/28x28/table.png \
		images/28x28/unified.png \
		images/32x32/copyLeft.png \
		images/32x32/copyLeft_next.png \
		images/32x32/copyRight.png \
		images/32x32/copyRight_next.png \
		images/32x32/firstDifference.png \
		images/32x32/lastDifference.png \
		images/32x32/nextDifference.png \
		images/32x32/previousDifference.png \
		images/32x32/reload.png \
		images/32x32/sidebyside.png \
		images/32x32/swap.png \
		images/32x32/table.png \
		images/32x32/unified.png \
		images/40x40/copyLeft.png \
		images/40x40/copyLeft_next.png \
		images/40x40/copyRight.png \
		images/40x40/copyRight_next.png \
		images/40x40/firstDifference.png \
		images/40x40/lastDifference.png \
		images/40x40/nextDifference.png \
		images/40x40/previousDifference.png \
		images/40x40/reload.png \
		images/40x40/sidebyside.png \
		images/40x40/swap.png \
		images/40x40/table.png \
		images/40x40/unified.png \
		images/48x48/copyLeft.png \
		images/48x48/copyLeft_next.png \
		images/48x48/copyRight.png \
		images/48x48/copyRight_next.png \
		images/48x48/firstDifference.png \
		images/48x48/lastDifference.png \
		images/48x48/nextDifference.png \
		images/48x48/previousDifference.png \
		images/48x48/reload.png \
		images/48x48/sidebyside.png \
		images/48x48/swap.png \
		images/48x48/table.png \
		images/48x48/unified.png \
		images/56x56/copyLeft.png \
		images/56x56/copyLeft_next.png \
		images/56x56/copyRight.png \
		images/56x56/copyRight_next.png \
		images/56x56/firstDifference.png \
		images/56x56/lastDifference.png \
		images/56x56/nextDifference.png \
		images/56x56/previousDifference.png \
		images/56x56/reload.png \
		images/56x56/sidebyside.png \
		images/56x56/swap.png \
		images/56x56/table.png \
		images/56x56/unified.png \
		images/64x64/copyLeft.png \
		images/64x64/copyLeft_next.png \
		images/64x64/copyRight.png \
		images/64x64/copyRight_next.png \
		images/64x64/firstDifference.png \
		images/64x64/lastDifference.png \
		images/64x64/nextDifference.png \
		images/64x64/previousDifference.png \
		images/64x64/reload.png \
		images/64x64/sidebyside.png \
		images/64x64/swap.png \
		images/64x64/table.png \
		images/64x64/unified.png \
		cb_diff_toolbar.xrc

EXTRA_DIST = $(srcdir)/*.cbp \
		$(srcdir)/wxsmith/cbDiffConfigPanel.wxs \
		$(srcdir)/wxsmith/cbDiffSelectFiles.wxs \
		$(resources_forZIP)

pkgdata_DATA = cbDiff.zip

CLEANFILES = $(pkgdata_DATA)

cbDiff.zip: $(resources_forZIP)
	PWD=`pwd` cd $(srcdir) && zip $(PWD)/cbDiff.zip $(resources_forZIP) > /dev/null
