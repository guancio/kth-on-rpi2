# A CMake toolchain file so we can cross-compile for the Rapsberry-Pi bare-metal

# usage
# cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake ../

# The Generic system name is used for embedded targets (targets without OS) in
# CMake
set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     BCM2386 )

# Set a toolchain path. You only need to set this if the toolchain isn't in
# your system path. Don't forget a trailing path separator!
set( TC_PATH "" )

# The toolchain prefix for all toolchain executables
set( CROSS_COMPILE arm-none-eabi- )

# specify the cross compiler. CMake automatically adds the CMAKE_C_COMPILER
# value into the CACHE
set( CMAKE_C_COMPILER   ${TC_PATH}${CROSS_COMPILE}gcc )

# We must set the OBJCOPY setting into cache so that it's available to the
# whole project. Otherwise, this does not get set into the CACHE and therefore
# the build doesn't know what the OBJCOPY filepath is
set( CMAKE_OBJCOPY      ${TC_PATH}${CROSS_COMPILE}objcopy
    CACHE FILEPATH "The toolchain objcopy command " FORCE )

