find_package(Shiboken2)

find_path(Shiboken2_INCLUDE_BASEDIR include/shiboken2)
set(Shiboken2_INCLUDE_DIRS
        ${Shiboken2_INCLUDE_BASEDIR}/include/shiboken2
        ${Qt5Core_INCLUDE_DIRS}
)
