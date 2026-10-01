find_package(Qt6 REQUIRED COMPONENTS Test)

function(tde_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE Tde::Tde Qt6::Test)
    target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

tde_test(tst_config)
tde_test(tst_iconrepair)
