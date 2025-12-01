cmake_minimum_required(VERSION 3.14)
project(RayTracing_OpenMP CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(RayTracing_OpenMP main.cpp)

# --- Configuración de OpenMP más explícita ---

find_package(OpenMP REQUIRED)

if (OpenMP_FOUND)
    # 1. Añade las banderas de compilación (ej: -fopenmp) al compilar main.cpp
    target_compile_options(RayTracing_OpenMP PRIVATE ${OpenMP_CXX_FLAGS})

    # 2. Añade las banderas de enlace (ej: -fopenmp) al enlazar el ejecutable
    target_link_libraries(RayTracing_OpenMP PRIVATE ${OpenMP_CXX_FLAGS})
endif()
