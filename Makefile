.PHONY: build app clean archive
app_name=AnycubicSlicer
build_dir=build
curdate=$(shell date +%Y%m%d)
number=$(shell system_profiler SPHardwareDataType|grep "Total Number of Cores"|awk '{print $$5}')
# "Minimum OS X deployment version"
CMAKE_OSX_DEPLOYMENT_TARGET=10.15
SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX$(CMAKE_OSX_DEPLOYMENT_TARGET).sdk
CMAKE_OSX_ARCHITECTURES=x86_64

archive: app
	type create-dmg >/dev/null 2>&1 ||  brew install create-dmg || exit 1
	ls *.dmg|xargs rm -rf
	create-dmg  --volname "Anycubic Slicer Installer"  --volicon "resources/$(app_name).icns"  \
		--window-pos 0 0 \
		--window-size 1290 750 \
		--background bg.png \
		--icon-size 210 \
		--icon "$(app_name)" 401 351  \
		--hide-extension "$(app_name)" \
		--app-drop-link 908 351 \
		"$(app_name)_$(curdate)_$(shell file $(build_dir)/src/AnycubicSlicer|awk '{print $$NF}').dmg" \
		"$(app_name).app"
	rm -rf $(app_name).app rw.$(app_name)*.dmg
	exit 0

build:
	ls deps/$(build_dir)/finished || { SDKROOT=$(SDKROOT) cmake -S deps -B deps/$(build_dir)   -DCMAKE_OSX_SYSROOT:PATH=$(SDKROOT) -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES} -DCMAKE_BUILD_TYPE:STRING=Release -DDEFAULT_SYSROOT:PATH=$(SDKROOT) -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=$(CMAKE_OSX_DEPLOYMENT_TARGET)&&cd  deps/$(build_dir) &&  cmake --build  . -j$(number) --config Release && cd - && touch deps/$(build_dir)/finished ; }
	SDKROOT=$(SDKROOT) cmake  -S . -B $(build_dir) -DCMAKE_OSX_SYSROOT:PATH=$(SDKROOT) -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=$(CMAKE_OSX_DEPLOYMENT_TARGET)   && cmake --build  $(build_dir) -j$(number) --config Release -t AnycubicSlicer

app: build
	rm -rf $(app_name) $(app_name).app || echo "rm"
	mkdir -p $(app_name)/Contents/Frameworks || echo "mk"
	mkdir -p $(app_name)/Contents/MacOS || echo "mk"
	cp Info.plist $(app_name)/Contents
	cp -r resources $(app_name)/Contents
	cp -r PlugIns $(app_name)/Contents
	cp $(build_dir)/src/AnycubicSlicer $(app_name)/Contents/MacOS
	mv $(app_name) "$(app_name).app"
clean:
	rm -rf $(build_dir)/CMake*
	ls deps/$(build_dir)/finished || rm -rf deps/$(build_dir)/finished deps/$(build_dir)/CMake* deps/$(build_dir)/dep_*


