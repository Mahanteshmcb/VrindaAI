include("C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI/llama.cpp/.qt/QtDeploySupport-MinSizeRel.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/VrindaAI-plugins-MinSizeRel.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE C:/Users/Mahantesh/DevelopmentProjects/VrindaAI/VrindaAI/llama.cpp/MinSizeRel/VrindaAI.exe
    GENERATE_QT_CONF
)
