aux_source_directory(. EDDY_SDK_SOURCE)
add_library(eddy_sdk STATIC ${EDDY_SDK_SOURCE})


if (UNIX)
	message("eddy sdk linking ..." ${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})
	target_link_libraries(eddy_sdk ${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})
endif(UNIX)

if (UNIX)
	target_link_libraries(eddy_sdk pthread)
	target_link_libraries(eddy_sdk tcmalloc)
endif (UNIX)

message("------------------------------------------")
