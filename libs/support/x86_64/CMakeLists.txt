
add_library (x86_64support OBJECT rightshifts.cpp div.cpp sext.cpp zext.cpp)
target_include_directories (x86_64support PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(x86_64support PUBLIC "-fPIC")

	
