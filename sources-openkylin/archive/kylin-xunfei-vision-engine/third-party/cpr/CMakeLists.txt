set(MODULE_CPR cpr)
add_compile_options(-fPIC)

find_package(PkgConfig REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CPR_SOURCE
    cpr/accept_encoding.cpp
    cpr/auth.cpp
    cpr/async.cpp
    cpr/bearer.cpp
    cpr/cert_info.cpp
    cpr/cookies.cpp
    cpr/cprtypes.cpp
    cpr/curl_container.cpp
    cpr/curlholder.cpp
    cpr/error.cpp
    cpr/interceptor.cpp
    cpr/multipart.cpp
    cpr/parameters.cpp
    cpr/payload.cpp
    cpr/proxies.cpp
    cpr/proxyauth.cpp
    cpr/redirect.cpp
    cpr/response.cpp
    cpr/session.cpp
    cpr/ssl_ctx.cpp
    cpr/threadpool.cpp
    cpr/timeout.cpp
    cpr/unix_socket.cpp
    cpr/util.cpp)

add_library(${MODULE_CPR} STATIC ${CPR_SOURCE})

target_link_libraries(
    cpr PRIVATE
    pthread
)
