# web/CMakeLists.txt

# 安装 web/usr/local/www 目录下的所有文件到 DESTDIR/usr/local/www
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/usr/local/www/
        DESTINATION ${DESTDIR}/usr/local/www
        FILES_MATCHING PATTERN "*"
        PATTERN ".git*" EXCLUDE
        PATTERN "CMakeLists.txt" EXCLUDE
)
