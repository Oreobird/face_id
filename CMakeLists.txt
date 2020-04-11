cmake_minimum_required(VERSION 2.8.4)

project(face_id)

# Build options
option(BUILD_FACE_ID "Set to ON to build examples"  ON)
option(BUILD_ARCFACE "Build ArcFace, default is FaceNet" ON)
option(BUILD_DEBUG "Build Debug version" ON)

# Use C++14
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")
message(STATUS "C++14 support has been enabled by default.")

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/base/)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml/)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/rapidjson/include/)
include_directories(src
					src/test
					src/ncnn
					src/ncnn/layer
					src/ncnn/layer/x86
					src/ncnn/layer/arm)
		
aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/base base_src_files)
add_library(base ${base_src_files})

aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/tinyxml tinyxml_src_files)
add_library(tinyxml ${tinyxml_src_files})

#aux_source_directory(${CMAKE_SOURCE_DIR}/3rdparty/openblas openblas_src_files)
#add_library(openblas ${openblas_src_files})

aux_source_directory(src evergrande_src_files)
aux_source_directory(src/ncnn ncnn_src)
aux_source_directory(src/ncnn/layer ncnn_layer_src)
aux_source_directory(src/ncnn/layer/x86 ncnn_layer_x86_src)
aux_source_directory(src/ncnn/layer/arm ncnn_layer_arm_src)

set(ncnn_files
	${ncnn_src}
	${ncnn_layer_src}
	${ncnn_layer_x86_src}
	${ncnn_layer_arm_src}
	)
set(cli_files
	src/mysql_conn.cpp
	src/mysql_mgt.cpp
	src/conf_mgt.cpp
	src/codecs.cpp
	src/protocol.cpp
	src/test/cli.cpp
	)
set(face_crop_tool_files
	src/face_aligner.cpp
	src/mtcnn.cpp
	src/face_detector.cpp
	src/tools/face_crop_tool.cpp
	)
set(get_face_feature_files
	src/mysql_conn.cpp
	src/mysql_mgt.cpp
	src/conf_mgt.cpp
	src/user_mgt.cpp
	src/face_aligner.cpp
	src/facenet_recognizer.cpp
	src/insightface_recognizer.cpp
	src/tools/face_get_feature.cpp
	)

if (BUILD_DEBUG)
	add_definitions(-DDEBUG)
endif (BUILD_DEBUG)

if (BUILD_FACE_ID)
    message(STATUS "Build with face id.")
    find_package(OpenCV)
    if (NOT OpenCV_FOUND)
        message(WARNING "OpenCV not found. Test will not be built.")
    else()
		include_directories(${Boost_INCLUDE_DIRS} build)
		include_directories(${OpenCV_INCLUDE_DIRS} build)
		include_directories(/usr/include/mysql/)
		include_directories(/usr/local/include/tf)
		include_directories(/usr/local/include/eigen3)
		include_directories(/usr/local/include/openblas)
		link_directories(/usr/local/lib/)
		set(required_libs ${OpenCV_LIBS})

		
		add_executable(fid_srv ${evergrande_src_files} ${ncnn_files})
		target_link_libraries(fid_srv ${required_libs} base -lrt -pthread -lprotobuf -lopenblas -ltensorflow_cc -ltensorflow_framework -lmysqlclient tinyxml)
		if (BUILD_ARCFACE)
			set(ARCFACE "1")
			target_compile_definitions(fid_srv 
										PRIVATE 
										ARCFACE=${ARCFACE})
		endif (BUILD_ARCFACE)
		
		add_executable(cli ${cli_files})
		target_link_libraries(cli ${required_libs} base -lrt -lpthread tinyxml -lmysqlclient)
		
		add_executable(crop_face ${face_crop_tool_files} ${ncnn_files})
		target_link_libraries(crop_face ${required_libs} base -lrt -lpthread)
		
		add_executable(get_face_feature ${get_face_feature_files})
		target_link_libraries(get_face_feature ${required_libs} base -lrt -lpthread -lprotobuf -lopenblas -ltensorflow_cc -ltensorflow_framework tinyxml -lmysqlclient)
    endif()
endif()
