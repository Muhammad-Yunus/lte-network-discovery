cmake_minimum_required(VERSION 3.10)
project(lte-network-discovery VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Find srsRAN
find_library(SRSRAN_PHY srsran_phy PATHS ${SRSRAN_LIB_DIR})
find_library(SRSRAN_RF srsran_rf PATHS ${SRSRAN_LIB_DIR})
find_library(SRSRAN_COMMON srsran_common PATHS ${SRSRAN_LIB_DIR})
find_library(RRC_ASN1 rrc_asn1 PATHS ${SRSRAN_LIB_DIR})

# Find SoapySDR
find_package(SoapySDR REQUIRED)

# Library
add_library(lte_scan STATIC src/lte_scan.cc)
target_include_directories(lte_scan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SRSRAN_INCLUDE_DIR}
)
target_link_libraries(lte_scan PUBLIC
    srsran_phy srsran_rf srsran_common rrc_asn1
    SoapySDR pthread m
)

# Example CLI
add_executable(lte_scan_example examples/lte_scan_example.c)
target_include_directories(lte_scan_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lte_scan_example lte_scan)

install(TARGETS lte_scan lte_scan_example
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES src/lte_scan.h DESTINATION include)
