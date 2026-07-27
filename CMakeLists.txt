cmake_minimum_required(VERSION 3.10)
project(lte-network-discovery VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# srsRAN install paths
set(SRSRAN_INCLUDE_DIR "/usr/local/include" CACHE PATH "srsRAN headers")
set(SRSRAN_LIB_DIR "/usr/local/lib" CACHE PATH "srsRAN libraries")

# Find srsRAN
find_library(SRSRAN_PHY srsran_phy PATHS ${SRSRAN_LIB_DIR})
find_library(SRSRAN_RF srsran_rf PATHS ${SRSRAN_LIB_DIR})
find_library(SRSRAN_COMMON srsran_common PATHS ${SRSRAN_LIB_DIR})
find_library(SRSRAN_RADIO srsran_radio PATHS ${SRSRAN_LIB_DIR})
find_library(RRC_ASN1 rrc_asn1 PATHS ${SRSRAN_LIB_DIR})
find_library(ASN1_UTILS asn1_utils PATHS ${SRSRAN_LIB_DIR})
find_library(SRSLOG srslog PATHS ${SRSRAN_LIB_DIR})

find_package(SoapySDR REQUIRED)
find_library(FFTW3F_LIB fftw3f PATHS /usr/lib /usr/lib/aarch64-linux-gnu)

# Library
add_library(lte_scan STATIC src/lte_scan.cc)
target_include_directories(lte_scan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SRSRAN_INCLUDE_DIR}
)
target_link_libraries(lte_scan PUBLIC
    ${SRSRAN_PHY} ${SRSRAN_RF} ${SRSRAN_RADIO} ${SRSRAN_COMMON}
    ${RRC_ASN1} ${ASN1_UTILS} ${SRSLOG}
    SoapySDR ${FFTW3F_LIB} pthread m dl
)

# Example CLI
add_executable(lte_scan_example examples/lte_scan_example.c)
target_include_directories(lte_scan_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lte_scan_example lte_scan)

# Unit tests (offline — no SDR needed)
enable_testing()
add_executable(test_lte_scan tests/test_lte_scan.c)
target_include_directories(test_lte_scan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_lte_scan lte_scan)
add_test(NAME lte_scan_unit COMMAND test_lte_scan)

install(TARGETS lte_scan lte_scan_example
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES src/lte_scan.h DESTINATION include)
