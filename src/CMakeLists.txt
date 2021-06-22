set(CMAKE_C_STANDARD 99)

add_executable(statsh)
target_sources(statsh PRIVATE
    main.c
    config.c
    cpu_stats.c
    disk_stats.c
    mem_stats.c
    net_stats.c
    signals.c
)

target_link_libraries(statsh rt m)

add_library(signals signals.c)

# Zipping the man page
find_program(GZIP_PATH
    NAMES gzip
    PATHS /bin
    /usr/bin
    /usr/local/bin
    /usr/local/sbin
)

add_custom_target(manpage ALL
        ${GZIP_PATH} -c ${CMAKE_CURRENT_SOURCE_DIR}/statsh.1 > ${CMAKE_BINARY_DIR}/statsh.1.gz)

install(FILES ${CMAKE_BINARY_DIR}/statsh.1.gz
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

install(TARGETS statsh
        DESTINATION ${CMAKE_INSTALL_BINDIR})
