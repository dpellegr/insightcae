
macro(setOFlibvar prefix)
  SET(${prefix}_LIBRARIES "")
   FOREACH(f ${ARGN})
    IF (EXISTS "${${prefix}_FOAM_LIBBIN}/lib${f}.so")
      LIST(APPEND ${prefix}_LIBRARIES "${${prefix}_FOAM_LIBBIN}/lib${f}.so")
    endif()
   ENDFOREACH(f)
#   set (${prefix}_LIBRARIES ${${prefix}_LIBRARIES} PARENT_SCOPE)
endmacro()

macro(detectEnvVar prefix varname outvarname)
 execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/getOFCfgVar ${${prefix}_BASHRC} print-${varname} OUTPUT_VARIABLE ${prefix}_${outvarname})
 #message(STATUS "Detected value of env var " ${varname} "=" ${${prefix}_${outvarname}})
endmacro()

macro(detectEnvVars prefix)
 FOREACH(f ${ARGN})
  detectEnvVar(${prefix} ${f} ${f})
 ENDFOREACH(f)
endmacro()

macro(detectDepLib prefix fromlib pattern)
 execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/findInDepLibs ${${prefix}_BASHRC} ${fromlib} "${pattern}" OUTPUT_VARIABLE addlibs)
 #message(STATUS "detected for ${pattern} in dependencies of ${fromlib}: " ${addlibs})
 LIST(APPEND ${prefix}_LIBRARIES "${addlibs}")
endmacro()

macro(detectIncPaths prefix)
 execute_process(COMMAND ${CMAKE_SOURCE_DIR}/CMake/printOFincPath ${${prefix}_BASHRC} ${ARGN} OUTPUT_VARIABLE ${prefix}_INCLUDE_PATHS)
 list(APPEND ${prefix}_INCLUDE_PATHS ${${prefix}_LIBSRC_DIR})
endmacro()



macro(addOFConfig prefix shortcut versionnumber)

    set(INSIGHT_OF_ALIASES "${INSIGHT_OF_ALIASES}
alias ${shortcut}=\"source insight.bashrc.${shortcut}\"
")

    set(${prefix}_ISCFG_BASHRC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/insight.bashrc.${shortcut})
  
    create_script("insight.bashrc.${shortcut}"
#"source ${${prefix}_BASHRC}
"source openfoam.bashrc.${shortcut}

export CURRENT_OFE=${prefix}
export CURRENT_OFE_FILE=$(basename $CURRENT_OFE)
foamClean=$WM_PROJECT_DIR/bin/foamCleanPath
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSIGHT_INSTDIR/${${prefix}_INSIGHT_INSTALL_LIB}
#- Clean LD_LIBRARY_PATH
cleaned=`$foamClean \"$LD_LIBRARY_PATH\"` && LD_LIBRARY_PATH=\"$cleaned\"
export PATH=$PATH:$INSIGHT_INSTDIR/${${prefix}_INSIGHT_INSTALL_BIN}
#- Clean PATH
cleaned=`$foamClean \"$PATH\"` && PATH=\"$cleaned\"
")

    # do not install the following, just keep in bin dir. Installed variant is generated in superbuild
    file(WRITE "${CMAKE_BINARY_DIR}/share/insight/ofes.d/${shortcut}.ofe"
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<root>
<ofe label=\"${prefix}\" bashrc=\"insight.bashrc.${shortcut}\" version=\"${versionnumber}\"/>
</root>
")

endmacro()
