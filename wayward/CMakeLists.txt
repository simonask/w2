message(INFO "Lol")

set(WAYWARD_SOURCES
    wayward.cpp
    app.cpp
    server.cpp
)

add_library(wayward SHARED ${WAYWARD_SOURCES})
target_include_directories(wayward PRIVATE ${HTTPPARSER_INCLUDE_DIR})
target_link_libraries(wayward ${HTTPPARSER_LIBS})
