find_package(PySide2)

find_path(PySide2_INCLUDE_BASEDIR include/PySide2)
find_path(PySide2_TYPESYSTEMS_PATH_BASEDIR PySide2/typesystems PATHS /usr/share)

set(PySide2_INCLUDE_DIRS
        ${PySide2_INCLUDE_BASEDIR}/include/PySide2
        ${PySide2_INCLUDE_BASEDIR}/include/PySide2/QtCore
)
set(PySide2_TYPESYSTEMS_PATH ${PySide2_TYPESYSTEMS_PATH_BASEDIR}/PySide2/typesystems)
