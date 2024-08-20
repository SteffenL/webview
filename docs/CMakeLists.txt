add_subdirectory(api)

add_custom_target(docs DEPENDS api_docs)
