# Create python bindings for base module
find_package(pybind11 CONFIG REQUIRED)

set(HEADER_FILES
    include/modules/base/pythonbindings/io/volumewriting.h
    include/modules/base/pythonbindings/algorithm/volumeoperations.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/modules/base/pythonbindings ${HEADER_FILES})

set(SOURCE_FILES
    src/basebindings.cpp
    src/io/volumewriting.cpp
    src/algorithm/volumeoperations.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwbase)
target_sources(ivwbase
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
target_link_libraries(ivwbase PUBLIC inviwo::module::base inviwo::module::python3)

ivw_install_helper(TARGET ivwbase
    NAMESPACE inviwo
    DESTINATION inviwo
)
