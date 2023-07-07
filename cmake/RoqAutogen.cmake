set(AUTOGEN_PY "${CMAKE_SOURCE_DIR}/scripts/autogen.py")

function(roq_autogen)
  set(options DUMMY_OPTION)
  set(one_value OUTPUT NAMESPACE OUTPUT_TYPE FILE_TYPE)
  set(multi_value SOURCES DEPENDS)
  cmake_parse_arguments(
    SELF
    "${options}"
    "${one_value}"
    "${multi_value}"
    ${ARGN}
  )
  unset(result)
  foreach(source ${SELF_SOURCES})
    get_filename_component(dir ${source} DIRECTORY)
    get_filename_component(name ${source} NAME_WE)
    set(target "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}.${SELF_FILE_TYPE}")
    set(depends "${PYTHON_EXECUTABLE}" "${AUTOGEN_PY}" "${source}")
    if(SELF_DEPENDS)
      list(APPEND depends ${SELF_DEPENDS})
    endif(SELF_DEPENDS)
    add_custom_command(
      OUTPUT "${target}"
      COMMAND ${PYTHON_EXECUTABLE} "${AUTOGEN_PY}" --namespace "${SELF_NAMESPACE}" --output-type "${SELF_OUTPUT_TYPE}" --file-type "${SELF_FILE_TYPE}" "${CMAKE_CURRENT_SOURCE_DIR}/${source}" > "${target}"
      COMMAND ${CLANG_FORMAT} -i "${target}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
      VERBATIM
      DEPENDS "${depends}")
    list(APPEND result "${target}")
  endforeach()
  set(${SELF_OUTPUT} ${result} PARENT_SCOPE)
endfunction()

function(roq_gitignore)
  set(options DUMMY_OPTION)
  set(one_value OUTPUT)
  set(multi_value SOURCES)
  cmake_parse_arguments(
    SELF
    "${options}"
    "${one_value}"
    "${multi_value}"
    ${ARGN}
  )
  set(gitignore "${CMAKE_CURRENT_SOURCE_DIR}/${SELF_OUTPUT}")
  message(STATUS "gitignore=${gitignore}")
  file(WRITE "${gitignore}" "")
  file(APPEND "${gitignore}" "!!! THIS FILE HAS BEEN AUTO-GENERATED !!!\n")
  foreach(source ${SELF_SOURCES})
    get_filename_component(name ${source} NAME)
    file(APPEND "${gitignore}" "${name}\n")
  endforeach()
endfunction()
