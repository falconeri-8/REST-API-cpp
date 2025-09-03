cmake_minimum_required(VERSION 3.31)
project(RESTful_API_cpp)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)

include(${wxWidgets_USE_FILE})

add_executable(Run main.cpp)

target_link_libraries(Run Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(Run ${wxWidgets_LIBRARIES} )
target_compile_definitions(Run PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(Run PRIVATE ${wxWidgets_INCLUDE_DIRS})