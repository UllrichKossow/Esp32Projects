idf_component_register(SRCS "StackTest.cc"
                    INCLUDE_DIRS ".")

idf_build_set_property(COMPILE_OPTIONS "-fstack-usage" APPEND)
#idf_build_set_property(COMPILE_OPTIONS "-fdump-ipa-cgraph" APPEND)
