

add_executable(ipdupdetect
    main.cpp
    Interfaces.cpp
    IPInfo.cpp
    Monitor.cpp
    MonitorManager.cpp
)

add_dependencies(ipdupdetect liblogger)
add_dependencies(ipdupdetect libclientserver)

target_include_directories(ipdupdetect PRIVATE
    .
    ${CMAKE_BINARY_DIR}/include
    ${UUID_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

target_link_libraries(ipdupdetect
    -Wl,-z,defs
    -lpthread
    -lpcap
    -Wl,-Bstatic -L${CMAKE_BINARY_DIR}/lib/ -llogger -lclientserver -Wl,-Bdynamic
    ${UUID_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

install(TARGETS ipdupdetect DESTINATION bin)

