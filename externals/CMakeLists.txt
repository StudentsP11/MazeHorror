function(initialize_submodule DIRECTORY)
  if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/.git)
    find_package(Git QUIET REQUIRED)
    message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/.git does not exist. Initializing ${DIRECTORY} submodule ...")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --remote --recursive ${DIRECTORY}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_EXIT_CODE)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --recursive ${DIRECTORY}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_EXIT_CODE)
    if(NOT GIT_EXIT_CODE EQUAL "0")
      message(FATAL_ERROR "${GIT_EXECUTABLE} submodule update --init ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY} failed with exit code ${GIT_EXIT_CODE}, please checkout submodules")
    endif()
  endif()
endfunction(initialize_submodule)

## SDL2
initialize_submodule(SDL)
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# SDL_mixer
initialize_submodule(SDL_mixer)
set(SDL2MIXER_VENDORED ON)
set(SDL2MIXER_WAVE ON)

set(SDL2MIXER_OPUS ON)
set(SDL2MIXER_MIDI ON)
set(SDL2MIXER_MP3 ON)
set(SDL2MIXER_VORBIS OFF)
set(SDL2MIXER_FLAC_LIBFLAC OFF)
set(SDL2MIXER_FLAC_DRFLAC OFF)
set(SDL2MIXER_MOD OFF)

add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)

# GLAD
add_subdirectory(GLAD)

