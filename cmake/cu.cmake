
#disable linking to stdc++ (Only works on linux)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

if(DEBUG)
add_definitions(-DDEBUG)
endif()

if(UNIX)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")



if(DEBUG)

if(BUILD_STEP)

  set(CMAKE_C_COMPILER /usr/bin/clang)
  set(CMAKE_CXX_COMPILER /usr/bin/clang++)

endif()

  #set(CLANG_DEBUG "-fsanitize=address,memory,undefined,safe-stack,thread")
  set(CLANG_DEBUG "") #break here __asan::ReportGenericError

set(STRICT_FLAGS "-Werror -Wall -Wextra -pedantic -Wcast-align  -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self  -Wmissing-include-dirs   -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo  -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -fdiagnostics-show-option -Wno-missing-field-initializers -Wno-missing-braces")

else()

set(OPT_FLAGS "-O3")
set(STRICT_FLAGS)

endif()



   set(PLATFORM_INCLUDES
    "${CMAKE_BINARY_DIR}/../src/cu_std/linux"
    )

  set(ASSIMP_LIB "${CMAKE_BINARY_DIR}/../extlib/libassimp.so.4")
  set(DL_LIB "dl")
  set(PTHREAD_LIB "pthread")
  
  set(PLATFORM_LIBS ${DL_LIB} ${PTHREAD_LIB} m)


  set(META_CMD -DCPP_PASS -E)
  set(META_OUT -o )

  set(PDLL SHARED)
  set(METACALL_SRC "")

else(UNIX)


#enable masm
enable_language(ASM_MASM)

if (NOT CMAKE_CL_64)
            message(FATAL_ERROR "Error: Masm64 not found")
endif()


if(DEBUG)

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/Od")

else()

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/O2")

endif()


    set(PLATFORM_INCLUDES
    "${CMAKE_BINARY_DIR}/../src/cu_std/win32"
    )

    set(ASSIMP_LIB "../extlib/assimp")
    set(PLATFORM_LIBS "kernel32.lib" "User32.lib" "Ole32.lib")


    #allow debug info here cos windows doesn't embed it into the dll/exe
    set(FLAGS "/EHsc /Zi ${OPT_FLAGS} ${STRICT_FLAGS}")


  set(META_CMD /DCPP_PASS /P)
  set(META_OUT /Fi)

  set(PDLL MODULE)
  set(METACALL_SRC src/asm_win32/cparser_win32.asm)

endif(UNIX)











function(Gen_IncludeList RET_LIST CMAKE_CUR_SRC_DIR)

set(C_INCLUDES "")

get_property(RAW_C_INCLUDES DIRECTORY ${CMAKE_CUR_SRC_DIR} PROPERTY INCLUDE_DIRECTORIES)

foreach(inc ${RAW_C_INCLUDES})

list(APPEND C_INCLUDES "-I${inc}")

endforeach()

set(${RET_LIST} ${C_INCLUDES} PARENT_SCOPE)

endfunction(Gen_IncludeList)

function(Add_Metapass TARGET SRC INC OUT DEP)

add_custom_command( TARGET ${TARGET} PRE_BUILD

#output pp files
COMMAND ${CMAKE_CXX_COMPILER} ${META_CMD} ${SRC} ${INC} ${META_OUT}${OUT}.i

#run generator on pp files
COMMAND cparser ${OUT}.i -meta ../include/generated/${OUT}_meta.h

)

add_dependencies(${DEP} ${TARGET})

endfunction()



function(Add_MetaCompass TARGET SRC INC OUT DEP)

add_custom_command( TARGET ${TARGET} PRE_BUILD

#output pp files
COMMAND ${CMAKE_CXX_COMPILER} ${META_CMD} ${SRC} ${INC} ${META_OUT}${OUT}.i

#run generator on pp files
COMMAND cparser ${OUT}.i -component ../include/generated/${OUT}comp_meta.h -meta ../include/generated/${OUT}_meta.h

)

add_dependencies(${DEP} ${TARGET})

endfunction()



function(CompileAllShaders TARGET)

file(GLOB SHADERS src/shaders/*)

foreach(shader ${SHADERS})

get_filename_component(SHADERNAME ${shader} NAME)

add_custom_command(TARGET ${TARGET} PRE_BUILD

COMMAND echo Shader Compiling ${SHADERNAME} to ${SHADERNAME}.spv

#output preprocessed
COMMAND glslc -DPREPROCESS -E -I ../include/shader_include -std=450 --target-env=vulkan ../src/shaders/${SHADERNAME} -o ../rsrc/shaders/pp_${SHADERNAME}

#compile
COMMAND glslc -I ../include/shader_include -std=450 --target-env=vulkan ../src/shaders/${SHADERNAME} -o ../rsrc/shaders/${SHADERNAME}.spv

#reflect
COMMAND glslparser ../rsrc/shaders/pp_${SHADERNAME} ../rsrc/shaders/${SHADERNAME}.spv

)

endforeach()

endfunction()

function(CompileShaderCase TARGET DEF IN OUT)

add_custom_command(TARGET ${TARGET} PRE_BUILD

COMMAND echo Shader Compiling ${IN} to ${OUT}

#output preprocessed
COMMAND glslc -D${DEF} -DPREPROCESS -E -I ../include/shader_include -std=450 --target-env=vulkan ../src/shaders/${IN} -o ../rsrc/shaders/pp_${OUT}

COMMAND glslc -I ../include/shader_include -std=450 --target-env=vulkan -D${DEF} ../src/shaders/${IN} -o ../rsrc/shaders/${OUT}.spv

COMMAND glslparser -D${DEF} ../rsrc/shaders/pp_${OUT} ../rsrc/shaders/${OUT}.spv

)

endfunction()

function(GenAndInstallAdditionalDep)

find_file(FOUND_ADDITIONAL assimp.dll PATHS ${CMAKE_BINARY_DIR}/../extlib/ NO_DEFAULT_PATH)



if(NOT FOUND_ADDITIONAL)

execute_process(
COMMAND echo DOWNLOADING...
COMMAND curl -L https://edrickhong.github.io/Pages/cu_additional.zip -o cu_additional.zip
)

execute_process(
COMMAND unzip -x cu_additional.zip -d ../
)

execute_process(
COMMAND rm cu_additional.zip
)

endif()



#generate wayland extension files if they exist
if(UNIX)

find_file(FOUND_WAYLAND_EXT_XML xdg-shell.xml PATHS /usr/share/wayland-protocols/stable/xdg-shell/ NO_DEFAULT_PATH)

find_program(FOUND_WAYLAND_SCANNER wayland-scanner)

if(FOUND_WAYLAND_EXT_XML AND FOUND_WAYLAND_SCANNER)



execute_process(

COMMAND echo GENERATING WAYLAND EXTENSIONS

#generate wayland files
COMMAND bash -c "wayland-scanner code </usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml >${CMAKE_BINARY_DIR}/../include/generated/xdg-shell.c"

COMMAND bash -c "wayland-scanner client-header </usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml >${CMAKE_BINARY_DIR}/../include/generated/xdg-shell.h"

)



else()

if(NOT FOUND_WAYLAND_SCANNER)

message(STATUS "WARNING: wayland-scanner not found!\n")

endif()

if(NOT FOUND_WAYLAND_EXT_XML)
message(STATUS "WARNING: wayland extension xml not found!\n")
endif()

message(STATUS "WARNING: Building without wayland extensions\n")

add_definitions(-DNO_WAYLAND_EXTENSIONS)


endif()


endif(UNIX)

endfunction()