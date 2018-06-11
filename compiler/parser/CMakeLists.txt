set(SOURCE_FILES
        Parser.h Parser.cpp
        Token.h Token.cpp
        TokenStream.h TokenStream.cpp)

add_library(maxim_parser ${SOURCE_FILES})

add_subdirectory(ast)
