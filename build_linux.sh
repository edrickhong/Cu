WORK_DIR="$(dirname "$0")"
cd $WORK_DIR

cd src/shaders/
echo "BUILDING METADATA"
#build meta data

./../../bin/cparser ../game/gamecomp.h -c ../game/gamecomp_meta.h -m testing.txt

echo "BUILDING SHADERS"

for file in *

do
    glslc -I ../shadercommon -std=450 --target-env=vulkan $file -o ../../rsrc/shaders/$file.spv
    ./../../bin/glslparser $file ../../rsrc/shaders/$file.spv
done

#specific vert files to process
glslc -I ../shadercommon -std=450 --target-env=vulkan -DUSE_SKEL model.vert -o ../../rsrc/shaders/model_skel.vert.spv
./../../bin/glslparser -DUSE_SKEL model.vert ../../rsrc/shaders/model_skel.vert.spv

glslc -I ../shadercommon -std=450 --target-env=vulkan -DUSE_SKEL tfetch_region.vert -o ../../rsrc/shaders/tfetch_region_skel.vert.spv
./../../bin/glslparser -DUSE_SKEL tfetch_region.vert ../../rsrc/shaders/tfetch_region_skel.vert.spv

glslc -I ../shadercommon -std=450 --target-env=vulkan -DUSE_TEXTURE m_gui.frag -o ../../rsrc/shaders/m_gui_tex.frag.spv
./../../bin/glslparser -DUSE_TEXTURE m_gui.frag ../../rsrc/shaders/m_gui_tex.frag.spv

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

#normal system build
#time -v make -j12
make -j12

#build in a chroot environment
#schroot -c xenial -- time -v make -j12

#manual compiling
#cd ../

# echo "COMPILING..."

# C_COMPILER="clang"
# CXX_COMPILER="clang++"

# C_COMPILER="/home/pluto/storage/Linux/svn/clang-compiler/build/bin/clang"
# CXX_COMPILER="/home/pluto/storage/Linux/svn/clang-compiler/build/bin/clang++"

# OUT_DIR="bin/"

# INCLUDE_DIR="-Isrc/game/ -Isrc/engine/ -Iinclude/ -Isrc/cu_std/cu -Isrc/cu_std/linux -Isrc/cu_std/amd64 -I/home/pluto/storage/Linux/sdk/VulkanSDK/custom/x86_64/include -I/home/pluto/storage/Linux/sdk/VulkanSDK/custom/x86_64/include/vulkan"

# OPT_FLAGS="-O0 -march=x86-64 -fno-omit-frame-pointer -fno-fast-math -fno-exceptions -fno-rtti "

# STRICT_FLAGS="-Werror -Wall -Wextra -pedantic -Wcast-align  -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self  -Wmissing-include-dirs   -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo  -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -fdiagnostics-show-option -Wno-missing-field-initializers -Wno-missing-braces"

# #"-fsanitize=address,memory,undefined,safe-stack,thread"  break here __asan::ReportGenericError
# CLANG_SANITIZERS=""
# FLAGS="-g -std=c++14 $OPT_FLAGS$STRICT_FLAGS$CLANG_SANITIZERS"

# #clean the build dir
# # rm $OUT_DIR/*.o
# # rm $OUT_DIR/*.a
# # rm $OUT_DIR/*.so

# #actual compilation

# #game dll
# GAME_OBJ="game.o"
# $CXX_COMPILER -fPIC -o $OUT_DIR$GAME_OBJ $INCLUDE_DIR $FLAGS -c src/game/game.cpp &

# #core std lib
# CU_CORE_OBJ="cu_core.o"
# $CXX_COMPILER -o $OUT_DIR$CU_CORE_OBJ $INCLUDE_DIR $FLAGS -c src/cu_std/cu/main.cpp &

# #platform std lib
# CU_PLAT_OBJ="cu_plat.o"
# $CXX_COMPILER -o $OUT_DIR$CU_PLAT_OBJ $INCLUDE_DIR $FLAGS -c src/cu_std/linux/main.cpp &

# #engine
# CU_ENGINE_OBJ="cu.o"
# $CXX_COMPILER -o $OUT_DIR$CU_ENGINE_OBJ $INCLUDE_DIR $FLAGS -c src/engine/main.cpp &

# wait

# #link cu_std
# CU_STDLIB="libcu_std.a"
# ar rcs $OUT_DIR$CU_STDLIB $OUT_DIR$CU_CORE_OBJ $OUT_DIR$CU_PLAT_OBJ

# #link game
# GAME="libgame.so"
# $C_COMPILER -rdynamic -shared $OUT_DIR$GAME_OBJ -o $OUT_DIR$GAME &

# # #link engine
# CU_ENGINE="cu_engine"
# $C_COMPILER -rdynamic $OUT_DIR$CU_ENGINE_OBJ $OUT_DIR$CU_STDLIB -lpthread -ldl -lm -o $OUT_DIR$CU_ENGINE &

# #importer
# IMPORTER="importer"
# $CXX_COMPILER -o $OUT_DIR$IMPORTER $INCLUDE_DIR $FLAGS -lpthread -ldl -lm -l:libassimp.so src/importer/main.cpp $OUT_DIR$CU_STDLIB &

# #cparser
# CPARSER="cparser"
# $CXX_COMPILER -o $OUT_DIR$CPARSER $INCLUDE_DIR $FLAGS -lpthread -ldl -lm src/cparser/main.cpp $OUT_DIR$CU_STDLIB &

# #glslparser
# GLSLPARSER="glslparser"
# $CXX_COMPILER -o $OUT_DIR$GLSLPARSER $INCLUDE_DIR $FLAGS -lpthread -ldl -lm src/glslparser/main.cpp $OUT_DIR$CU_STDLIB &
