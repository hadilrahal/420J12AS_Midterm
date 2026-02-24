cmake_minimum_required(VERSION 3.21)
project(420J12_Midterm)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
			   src/main.cpp
			   src/entity.cpp
			   # Si vous ajoutez d'autres fichiers .cpp, il faut les lister ici:
			   )

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3 SDL3_image SDL3_ttf)

set(GAME_RES_SRC_DIR ${CMAKE_SOURCE_DIR}/res)
set(GAME_RES_BINARY_DIR ${CMAKE_BINARY_DIR}/res)
add_custom_target(
		resources ALL
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GAME_RES_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${GAME_RES_SRC_DIR} ${GAME_RES_BINARY_DIR}
		COMMENT "Clearing and copying game resources into the binary directory."
		)
add_dependencies(${PROJECT_NAME} resources)
