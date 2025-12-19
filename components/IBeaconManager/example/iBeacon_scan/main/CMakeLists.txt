   # 递归查找所有 C/C++ 源文件
   file(GLOB_RECURSE SOURCES "*.c" "*.cpp")

   # 注册组件
   idf_component_register(
           SRCS ${SOURCES}
           INCLUDE_DIRS "."
           PRIV_REQUIRES bt IBeaconManager
   )