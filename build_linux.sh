WORK_DIR="$(dirname "$0")"
cd $WORK_DIR

cd src/shaders/

#specific vert files to process


BUILD_GUI_BIN_DATA=false

if ! [ -z "$1" ];then

    if [ $1 = "GUIDATA" ]; then

	echo "BUILDING GUI BIN DATA"
	BUILD_GUI_BIN_DATA=true
    fi
fi

if "$BUILD_GUI_BIN_DATA" == true; then

    #generates gui_bin.h
    > ../../src/cu_std/cu/gui_bin.h

fi

cd ../../rsrc/shaders/



if "$BUILD_GUI_BIN_DATA" == true; then

    xxd -i m_gui.vert.spv >> ../../src/cu_std/cu/gui_bin.h
    xxd -i m_gui.frag.spv >> ../../src/cu_std/cu/gui_bin.h
    xxd -i m_gui_tex.frag.spv >> ../../src/cu_std/cu/gui_bin.h

    echo "BUILT GUIBIN_HEADER SHADERS"

fi

cd ../../bin/

if "$BUILD_GUI_BIN_DATA" == true; then

    xxd -i Ubuntu-B.fbmp >> ../src/cu_std/cu/gui_bin.h

    echo "BUILT GUIBIN_HEADER FONT"

fi

cd ../Make

make -j12

