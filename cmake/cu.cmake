if(DEBUG)
  add_definitions(-DDEBUG)
endif()

if(UNIX)

  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

  if(DEBUG)

    if(BUILD_STEP)

      set(CMAKE_C_COMPILER /usr/bin/clang)
      set(CMAKE_CXX_COMPILER /usr/bin/clang++)

    endif()

    # set(CLANG_DEBUG "-fsanitize=address,memory,undefined,safe-stack,thread")
    set(CLANG_DEBUG "") # break here __asan::ReportGenericError

    set(
      STRICT_FLAGS
      "-Werror -Wall -Wextra -pedantic -Wcast-align  -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self  -Wmissing-include-dirs   -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo  -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -fdiagnostics-show-option -Wno-missing-field-initializers -Wno-missing-braces -Wimplicit-fallthrough -Wdouble-promotion -Wnull-dereference -Wswitch -Wuninitialized -Wunknown-pragmas -Warray-bounds -Wtautological-compare -Wfloat-equal -Wabsolute-value -Wdangling-else -Waddress -Wpacked -Wvla -Wstack-protector"
      )

  else()

    set(OPT_FLAGS "-O3")
    set(STRICT_FLAGS)

  endif()

  set(ASSIMP_LIB "${CMAKE_SOURCE_DIR}/extlib/libassimp.so.4")
  set(DL_LIB "dl")
  set(PTHREAD_LIB "pthread")

  set(PLATFORM_LIBS ${DL_LIB} ${PTHREAD_LIB} m)

  set(META_CMD -DCPP_PASS -E)
  set(META_OUT -o)

  set(PDLL SHARED)
  set(METACALL_SRC "")

else(UNIX)

  # enable masm
  enable_language(ASM_MASM)

  if(NOT CMAKE_CL_64)
    message(FATAL_ERROR "Error: Masm64 not found")
  endif()

  if(DEBUG)

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/Od")

  else()

    set(STRICT_FLAGS "/W3 /WX")
    set(OPT_FLAGS "/O2")

  endif()

  set(ASSIMP_LIB "../extlib/assimp")
  set(PLATFORM_LIBS "kernel32.lib" "User32.lib" "Ole32.lib" "Avrt.lib")

  # allow debug info here cos windows doesn't embed it into the dll/exe
  set(FLAGS "/FC /EHsc /Zi ${OPT_FLAGS} ${STRICT_FLAGS}")

  set(META_CMD /DCPP_PASS /P)
  set(META_OUT /Fi)

  set(PDLL MODULE)
  set(METACALL_SRC src/asm_win32/cparser_win32.asm)

endif(UNIX)

function(Gen_IncludeList RET_LIST CMAKE_CUR_SRC_DIR)

  set(C_INCLUDES "")

  get_property(RAW_C_INCLUDES
               DIRECTORY ${CMAKE_CUR_SRC_DIR}
               PROPERTY INCLUDE_DIRECTORIES)

  foreach(inc ${RAW_C_INCLUDES})

    list(APPEND C_INCLUDES "-I${inc}")

  endforeach()

  set(${RET_LIST} ${C_INCLUDES} PARENT_SCOPE)

endfunction(Gen_IncludeList)

function(Add_Metapass TARGET SRC INC OUT DEP OUT_VAR)

  add_custom_command(
                     OUTPUT ${OUT}.i
                     DEPENDS ${SRC}
                     # output pp files
                     COMMAND ${CMAKE_CXX_COMPILER} ${META_CMD} ${SRC} ${INC}
                             ${META_OUT}${OUT}.i
                             # run generator on pp files
                     COMMAND cparser ${OUT}.i -meta-source
                             ../include/generated/${OUT}_meta.cpp -meta-header
                             ../include/generated/${OUT}_meta.h
                     )

  set(${OUT_VAR} ${OUT}.i PARENT_SCOPE)

endfunction()

function(Add_MetaCompass TARGET SRC INC OUT DEP OUT_VAR)

  add_custom_command(
                     OUTPUT ${OUT}.i
                     DEPENDS ${SRC}
                     # output pp files
                     COMMAND ${CMAKE_CXX_COMPILER} ${META_CMD} ${SRC} ${INC}
                             ${META_OUT}${OUT}.i
                             # run generator on pp files
                     COMMAND cparser ${OUT}.i -component-source
                             ../include/generated/${OUT}comp_meta.cpp
                             -meta-source ../include/generated/${OUT}_meta.cpp
                             -component-header
                             ../include/generated/${OUT}comp_meta.h -meta-header
                             ../include/generated/${OUT}_meta.h
                     )

  set(${OUT_VAR} ${OUT}.i PARENT_SCOPE)

endfunction()

function(compileallshaders OUT_LIST)

  file(GLOB SHADERS src/shaders/*)

  foreach(shader ${SHADERS})

    get_filename_component(SHADERNAME ${shader} NAME)
    list(APPEND TLIST ${CMAKE_BINARY_DIR}/${SHADERNAME}.stamp)

    file(GLOB ${SHADERNAME}_FILE ${CMAKE_SOURCE_DIR}/src/shaders/${SHADERNAME})

    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/${SHADERNAME}.stamp
      DEPENDS ${${SHADERNAME}_FILE}
      COMMAND cmake -E touch ${CMAKE_BINARY_DIR}/${SHADERNAME}.stamp
      COMMAND echo Shader Compiling ${SHADERNAME} to ${SHADERNAME}.spv
              # output preprocessed
      COMMAND glslc -DPREPROCESS -E -I ../include/shader_include -std=450
              --target-env=vulkan ../src/shaders/${SHADERNAME} -o
                                  ../rsrc/shaders/pp_${SHADERNAME}
                                  # compile
      COMMAND glslc -I ../include/shader_include -std=450
              --target-env=vulkan ../src/shaders/${SHADERNAME} -o
                                  ../rsrc/shaders/${SHADERNAME}.spv
                                  # reflect
      COMMAND glslparser ../rsrc/shaders/pp_${SHADERNAME}
              ../rsrc/shaders/${SHADERNAME}.spv
      VERBATIM
      )

  endforeach()

  set(${OUT_LIST} ${TLIST} PARENT_SCOPE)

endfunction()

function(compileshadercase DEF IN OUT OUT_VAR)

  get_filename_component(IN_SHADERNAME ${IN} NAME)
  get_filename_component(OUT_SHADERNAME ${OUT} NAME)

  file(GLOB ${IN_SHADERNAME}_FILE ${CMAKE_SOURCE_DIR}/src/shaders/${IN_SHADERNAME})

  add_custom_command(
                     OUTPUT ${CMAKE_BINARY_DIR}/${OUT_SHADERNAME}.stamp
                     DEPENDS ${${IN_SHADERNAME}_FILE}
                     COMMAND cmake -E touch ${CMAKE_BINARY_DIR}/${OUT_SHADERNAME}.stamp
                     COMMAND echo Shader Compiling ${IN} to ${OUT}
                             # output preprocessed
                     COMMAND glslc -D${DEF} -DPREPROCESS -E -I
                             ../include/shader_include -std=450
                             --target-env=vulkan ../src/shaders/${IN} -o
                                                 ../rsrc/shaders/pp_${OUT}
                     COMMAND glslc -I ../include/shader_include -std=450
                             --target-env=vulkan -D${DEF} ../src/shaders/${IN}
                                                 -o ../rsrc/shaders/${OUT}.spv
                     COMMAND glslparser -D${DEF} ../rsrc/shaders/pp_${OUT}
                             ../rsrc/shaders/${OUT}.spv)

  set(${OUT_VAR} ${CMAKE_BINARY_DIR}/${OUT_SHADERNAME}.stamp PARENT_SCOPE)

endfunction()

function(GenAndInstallAdditionalDep)

  find_file(FOUND_ADDITIONAL assimp.dll
            PATHS ${CMAKE_SOURCE_DIR}/extlib/
            NO_DEFAULT_PATH)

  if(NOT FOUND_ADDITIONAL)

    execute_process(COMMAND echo DOWNLOADING...
                    COMMAND curl -L
                            https://edrickhong.github.io/Pages/cu_additional.zip
                            -o cu_additional.zip)

    execute_process(COMMAND unzip -x cu_additional.zip -d ../)

    execute_process(COMMAND rm cu_additional.zip)

  else()

  execute_process(COMMAND echo SKIP DOWNLOAD)

  endif()

endfunction()

function(InitSubmodules)

  find_file(FOUND_SUBMODULES RefCMakeLists.txt
            PATHS ${CMAKE_SOURCE_DIR}/Cu_std/
            NO_DEFAULT_PATH)

  if(NOT FOUND_SUBMODULES)

    execute_process(COMMAND git submodule init)
    execute_process(COMMAND git submodule update)

  endif()

endfunction()
