cmake_minimum_required(VERSION 3.15)
project(cppLox DESCRIPTION "A tree walk interpreter for the lox language as described in the book crafting interpreters")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wnull-dereference -Wshadow -Wformat=2 -pedantic)
# Debug mode sanitizers
add_compile_options("$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
add_link_options("$<$<CONFIG:Debug>:-fsanitize=address,undefined>")

include_directories("${CMAKE_SOURCE_DIR}/include")
add_executable(
	lox
	"src/lox.cxx"
	"src/error.cxx"
	"src/scanner.cxx"
	"src/parser.cxx"
	"src/resolver.cxx"
	"src/object.cxx"
	"src/native.cxx"
	"src/lox_function.cxx"
	"src/lox_class.cxx"
	"src/interpreter.cxx"
)
