set(CMAKE_C_STANDARD 99)

find_program(GZIP_PATH
        NAMES gzip
        PATHS /bin
        /usr/bin
        /usr/local/bin
        /usr/local/sbin)
add_executable(statsh main.c)
target_link_libraries(statsh
        config
        cpu_stats
        disk_stats
        mem_stats
        net_stats
        signals
        rt
        m)

add_library(config config.c)
add_library(cpu_stats cpu_stats.c)
add_library(disk_stats disk_stats.c)
add_library(mem_stats mem_stats.c)
add_library(net_stats net_stats.c)
add_library(signals signals.c)

add_custom_target(manpage ALL
        ${GZIP_PATH} -c ${CMAKE_CURRENT_SOURCE_DIR}/statsh.1 > ${CMAKE_BINARY_DIR}/statsh.1.gz)

install(FILES ${CMAKE_BINARY_DIR}/statsh.1.gz
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(TARGETS statsh
        DESTINATION ${CMAKE_INSTALL_BINDIR})
