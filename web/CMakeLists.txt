# web/CMakeLists.txt

# 安装 web/var/www 目录下的所有文件到 DESTDIR/var/www
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/install/
	DESTINATION ${DESTDIR}/
)
