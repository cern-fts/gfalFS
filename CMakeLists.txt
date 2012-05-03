project (gfalFS)
cmake_minimum_required (VERSION 2.6)
message("cmake source dir : ${CMAKE_SOURCE_DIR}")
# build type
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
# load module
include(DefineInstallationPaths REQUIRED)
include(MacroCopyFile REQUIRED)
include(ReleaseDebugAutoFlags REQUIRED)
include(CMakeGeneratePkgConfig REQUIRED)

find_package(PkgConfig REQUIRED)

#enable testing
enable_testing()

#define PROJECT vars
set(PROJECT_NAME_MAIN "gfalFS")
set(OUTPUT_NAME_MAIN "gfalFS")


set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})




# libs checks
pkg_check_modules(GLIB2_PKG REQUIRED glib-2.0)
pkg_check_modules(GTHREAD2_PKG REQUIRED gthread-2.0)

pkg_check_modules(GFAL2_PKG REQUIRED gfal2)

pkg_check_modules(FUSE_PKG REQUIRED fuse)

# main files
FILE(GLOB src_main "src/*.c")

include_directories("src/" ${GLIB2_PKG_INCLUDE_DIRS} 
						${GTHREAD2_PKG_INCLUDE_DIRS} ${GFAL2_PKG_INCLUDE_DIRS}
						 ${FUSE_PKG_INCLUDE_DIRS})
add_definitions( -D_GFALFS_VERSION=\"${VERSION_STRING}\" ${GLIB2_PKG_CFLAGS}
				${GTHREAD2_PKG_CFLAGS} ${GFAL2_PKG_CFLAGS} ${FUSE_PKG_CFLAGS} 
				-D_LARGEFILE64_SOURCE )

add_executable(gfalFS ${src_main})
target_link_libraries(gfalFS ${GLIB2_PKG_LIBRARIES} 
						${GTHREAD2_PKG_LIBRARIES} ${GFAL2_PKG_LIBRARIES}
						 ${FUSE_PKG_LIBRARIES})



install(TARGETS gfalFS
		RUNTIME       DESTINATION ${BIN_INSTALL_DIR}
		LIBRARY       DESTINATION ${LIB_INSTALL_DIR} )
		
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gfalFS_umount 
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			DESTINATION ${BIN_INSTALL_DIR})
			
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/gfalFS.man 
			RENAME gfalFS.1
			DESTINATION ${MAN_INSTALL_DIR}/man1/)



#install doc license
install(FILES "LICENSE" "README" "VERSION" "DESCRIPTION"
			DESTINATION ${DOC_INSTALL_DIR})
