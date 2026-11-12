cmake_minimum_required(VERSION 3.25)
project(PigCalculator VERSION 1.0.0 LANGUAGES CXX)


# ---- 1. ГЛОБАЛЬНЫЕ НАСТРОЙКИ КОМПИЛЯТОРА ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Управление режимами Debug / Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # В режиме отладки: полностью отключаем оптимизации, включаем максимум символов
    add_compile_options(
    	-Og 			# no optimization
    	-g3 			# 
    	-march=native 	# use native CPU instructions
    	-fmax-errors=3  				# 
        #-fcontext-limits=1              # Максимально сжимает контекст ошибок в шаблона
        )
    #add_compile_options("-fdiagnostics-absolute-prefix=${CMAKE_CURRENT_SOURCE_DIR}")
    message(STATUS "=== Режим отладки (Debug): Оптимизации выключены (-O0), символы включены ===")
else()
    # В режиме релиза: выжимаем максимум, включая все векторные инструкции вашего CPU (-march=native)
    add_compile_options(-O3 -march=native)
    message(STATUS "=== Режим релиза (Release): Максимальная скорость (-O3), инструкции хоста (-march=native) ===")
endif()

# ---- 1а. ПОДКЛЮЧЕНИЕ CLI11 (Парсер аргументов) ----
include(FetchContent)
FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(cli11)



# ---- 2. СБОРКА ЯДРА (СТАТИЧЕСКАЯ БИБЛИОТЕКА) ----
add_library(pgcalc_core STATIC 
    src/calculator.cpp
    src/main.cpp
)
add_library(pgcalc::core ALIAS pgcalc_core)

# Настраиваем пути к инклудам (чтобы #include "lz77.h" работал везде)
target_include_directories(pgcalc_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Ограничиваем количество выводимых ошибок в терминал до 3 для удобства
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(pgcalc_core PRIVATE -fmax-errors=3)
endif()

# ---- 3. СБОРКА ПРИЛОЖЕНИЯ ----
add_executable(${PROJECT_NAME} src/main.cpp)

# Линкуем наше ядро к приложению
target_link_libraries(${PROJECT_NAME} PRIVATE pgcalc::core CLI11::CLI11)

# ---- 4. БЛОК САНИТАЙЗЕРА (ДЛЯ ПОИСКА ОШИБОК ПАМЯТИ) ----
if(ENABLE_SANITIZER)
    message(STATUS "=== AddressSanitizer включен для всех целей ===")
    target_compile_options(pgcalc_core PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(pgcalc_core PRIVATE -fsanitize=address)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()


# ---- 5. ПОДКЛЮЧЕНИЕ ТЕСТОВ (Google Test) ----
option(BUILD_TESTING "Собрать тесты" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()


# Определяем путь к папке, куда нужно копировать (можно указать абсолютный или относительный путь)
set(TARGET_COPY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/workarea")

# Создаем папку, если её еще нет, во время конфигурации
file(MAKE_DIRECTORY "${TARGET_COPY_DIR}")

# Вешаем триггер на сборку приложения
add_custom_command(
    TARGET ${PROJECT_NAME}         # Имя вашей цели (add_executable(${PROJECT_NAME} ...))
    POST_BUILD                     # Запустить СРАЗУ ПОСЛЕ успешной сборки
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${PROJECT_NAME}> # Что копируем (авто-путь к собранному бинарнику)
            "${TARGET_COPY_DIR}/"          # Куда копируем
    COMMENT "Копирование обновленного бинарника в ${TARGET_COPY_DIR}..."
)
