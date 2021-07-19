set(CMAKE_C_STANDARD 99)

add_executable(statsh)

file(GLOB SOURCE_FILES "*.c")

target_sources(statsh
  PRIVATE ${SOURCE_FILES})

target_link_libraries(statsh rt m)

# gzip is needed to compress the manpage
find_program(GZIP_PATH
  NAMES gzip
  PATHS /bin /usr/bin /usr/local/bin /usr/local/sbin)

add_custom_target(manpage ALL
  ${GZIP_PATH} -c ${CMAKE_CURRENT_SOURCE_DIR}/statsh.1 > ${CMAKE_BINARY_DIR}/statsh.1.gz)

install(FILES ${CMAKE_BINARY_DIR}/statsh.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

install(TARGETS statsh
  DESTINATION ${CMAKE_INSTALL_BINDIR})
