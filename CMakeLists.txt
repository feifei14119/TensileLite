CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0)

PROJECT (TensileLite)

SET (TARGET tensile_lite)
SET (ROCM_PATH /opt/rocm)
SET (HIP_PATH ${ROCM_PATH}/hip)
SET (CMAKE_CXX_COMPILER ${HIP_PATH}/bin/hipcc)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${HIP_PATH}/include -I${ROCM_PATH}/include ")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -std=c++11")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --amdgpu-target=gfx908")

file(GLOB SRC_LIST
    src/HgemmMfma.cpp
    src/main.cpp
    )

file(GLOB LIB_LIST
    lib/libff_gpu.a
    lib/libff_utils.a
    )

ADD_EXECUTABLE (${TARGET} ${SRC_LIST})
TARGET_LINK_LIBRARIES (${TARGET} ${LIB_LIST})
#TARGET_LINK_LIBRARIES (${TARGET} ${LIB_LIST} ${ROCM_PATH}/lib/libhip_hcc.so)