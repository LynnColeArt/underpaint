find_package(${QT_PACKAGE_NAME} REQUIRED COMPONENTS Test Gui)

add_unit_test(desktop aijob
	LIBS ${QT_PACKAGE_NAME}::Test
	SOURCES ../ai/aijob.cpp
)

add_unit_test(desktop aijobrunner
	LIBS ${QT_PACKAGE_NAME}::Test ${QT_PACKAGE_NAME}::Gui
	SOURCES ../ai/aijob.cpp ../ai/aijobrunner.cpp
)

if(TARGET test_desktop_aijobrunner AND TARGET underpaint-ai-worker-stub)
	add_dependencies(test_desktop_aijobrunner underpaint-ai-worker-stub)
endif()
