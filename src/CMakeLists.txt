# This is FruityMesh CMakeLists file.

add_subdirectory(interfaces/log)
add_subdirectory(fruitymesh_platform/log)
add_all_files_to_targets("${NATIVE_TARGETS}")