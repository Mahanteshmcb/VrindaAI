include("C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI/llama.cpp/.qt/QtDeploySupport-Release.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/VrindaAI-plugins-Release.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI/llama.cpp/Release/VrindaAI.exe
    GENERATE_QT_CONF
)
