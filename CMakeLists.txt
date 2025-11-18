cmake_minimum_required(VERSION 3.19)
project(VrindaAI LANGUAGES CXX)

# ✅ Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ✅ Enable automatic Qt meta-object code, resource and UI generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ✅ Find all needed Qt components in one go (no duplication)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql PrintSupport LinguistTools Gui Network)

# ✅ Optional: Sets common Qt compiler settings
qt_standard_project_setup()

# ✅ Define sources
qt_add_executable(VrindaAI
    WIN32 MACOSX_BUNDLE
    # --- Main Application Files ---
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    # --- Services ---
    Services/databasemanager.cpp
    Services/databasemanager.h
    Services/modelmanager.cpp
    Services/modelmanager.h
    Services/reportgenerator.cpp
    Services/reportgenerator.h
    Services/projectmanager.cpp
    Services/projectmanager.h
    Services/vectordatabasemanager.cpp
    Services/vectordatabasemanager.h

    # --- Controllers ---
    Controllers/projectworkflow.cpp
    Controllers/projectworkflow.h
    Controllers/blendercontroller.cpp
    Controllers/blendercontroller.h
    Controllers/unrealcontroller.cpp
    Controllers/unrealcontroller.h
    Controllers/compilercontroller.cpp
    Controllers/compilercontroller.h
    Controllers/projectstatecontroller.cpp
    Controllers/projectstatecontroller.h
    Controllers/modelingcontroller.cpp
    Controllers/modelingcontroller.h
    Controllers/texturingcontroller.cpp
    Controllers/texturingcontroller.h
    Controllers/animationcontroller.cpp
    Controllers/animationcontroller.h
    Controllers/validatorcontroller.cpp
    Controllers/validatorcontroller.h
    Controllers/llamaservercontroller.cpp
    Controllers/llamaservercontroller.h

    # --- Utilities ---
    Utils/promptsanitizer.cpp
    Utils/promptsanitizer.h
    Utils/codescanner.cpp
    Utils/codescanner.h

    # --- Others ---
    styles/darktheme.qss
    db/tasks.sqlite
    memory_service.py
)

target_include_directories(VrindaAI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ✅ Handle translations (optional)
qt_add_translations(
    TARGETS VrindaAI
    TS_FILES VrindaAI_sa_IN.ts
)

# ✅ Link Qt libraries
target_link_libraries(VrindaAI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Sql
        Qt6::PrintSupport
        Qt6::Network
)

# On MinGW builds the mingw32 runtime may be needed to satisfy
# low-level entry-point symbols used by Qt's entrypoint object.
if(MINGW)
    # Try linking a few MinGW support objects that can provide low-level
    # runtime entry symbols used by Qt's entrypoint helper library.
    target_link_libraries(VrindaAI PRIVATE mingw32 mingwex mingwthrd)
endif()

# ✅ Install rules (optional for deployment)
include(GNUInstallDirs)

install(TARGETS VrindaAI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ✅ Deployment script generation
qt_generate_deploy_app_script(
    TARGET VrindaAI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
