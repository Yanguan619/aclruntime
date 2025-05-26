set(PKG_NAME pybind11)
set(DOWNLOAD_PATH "$ENV{PROJECT_ROOT_PATH}/third_party")
set(DIR_NAME "${DOWNLOAD_PATH}/pybind11")

if (NOT ${PKG_NAME}_FOUND)

download_opensource_pkg(${PKG_NAME}
    DOWNLOAD_PATH ${DOWNLOAD_PATH}
)

endif()
