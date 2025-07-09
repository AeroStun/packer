cmake_minimum_required (VERSION 3.16)
project (loader)

set (CMAKE_C_STANDARD 23)

add_executable (loader loader.c)

add_executable (embedder embedder.c)

add_custom_command(TARGET loader
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                            -DOBJDUMP_COMMAND=${CMAKE_OBJDUMP}
                            -DEMBEDDER_BINARY=$<TARGET_FILE:embedder>
                            -DTARGET_BINARY=$<TARGET_FILE:loader>
                            -P ${CMAKE_CURRENT_LIST_DIR}/CMake/Scripts/EmbedLoaderIdentity.cmake
                   COMMENT "Embedding loader identity"
)
set_target_properties (loader PROPERTIES LINK_FLAGS_RELEASE -s)

add_executable (packer packer.c)
set_target_properties (packer PROPERTIES LINK_FLAGS_RELEASE -s)
add_dependencies (packer loader)
