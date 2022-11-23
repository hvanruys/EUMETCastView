cmake_minimum_required(VERSION 3.18)

project(xRITDecompress)

add_executable(${PROJECT_NAME} xRITDecompress.cpp)

target_include_directories(xRITDecompress PUBLIC 
	"${CMAKE_SOURCE_DIR}/DISE"
	"${CMAKE_SOURCE_DIR}/COMP/Inc"
	"${CMAKE_SOURCE_DIR}/COMP/JPEG/Inc"
	"${CMAKE_SOURCE_DIR}/COMP/T4/Inc"
	"${CMAKE_SOURCE_DIR}/COMP/WT/Inc"
)

# sequence of libraries is important !
target_link_libraries(${PROJECT_NAME}
	my_compiler_flags JPEG T4 WT DISE COMP)