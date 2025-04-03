cmake_minimum_required(VERSION 3.16)

# Project Name and Version
project(CppPriceTicker VERSION 0.1 LANGUAGES CXX)

# Find Extra CMake Modules (essential for KDE/Qt development)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

# Include KDE CMake modules
include(KDEInstallDirs)     # For installation paths (e.g., ${KDE_INSTALL_PLUGINDIR})
include(KDEFrameworksCompilerSettings NO_POLICY_SCOPE) # Sensible compiler settings
include(KDECMakeSettings)   # More KDE specific settings
include(KF6PlasmaMacros)    # For plasma applet installation macro

# Set C++ standard (Qt6 typically requires C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Find Packages ---
# Qt6 is found in src/CMakeLists.txt

# --- Add Subdirectories ---
add_subdirectory(src)

# --- Installation ---
# Install the QML files and metadata.json using the KF6 macro
kf6_plasma_add_applet(com.example.cpppriceticker # Unique name for the applet package
    NAME "ZEC Price Ticker (C++)" # Display name
    PLUGIN_ID "com.example.cpppriceticker" # Matches metadata Id
    METADATA "metadata.json" # The metadata file
    QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/contents/ui/main.qml # List QML files
)

# Install other resources if needed (e.g., icons in contents/icons)
# install(DIRECTORY contents/icons DESTINATION ${KDE_INSTALL_DATADIR}/plasma/applets/com.example.cpppriceticker)