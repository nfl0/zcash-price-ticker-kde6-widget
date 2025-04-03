# Find necessary Qt modules
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml WebSockets Network)

# Find KDE Frameworks Plasma module
find_package(KF6 REQUIRED COMPONENTS Plasma)

# Define the plugin library target
add_library(com_example_cpppriceticker_plugin MODULE
    priceticker.cpp
    priceticker.h
)

# Tell CMake to run the Meta-Object Compiler (moc) on the header
# ECM=Extra CMake Modules, provided by KDE Frameworks development packages
include(ECMSetupQtPlugin)
ecm_setup_qtplugin_headers(com_example_cpppriceticker_plugin HEADERS priceticker.h)


# Link against the required Qt and KF6 libraries
target_link_libraries(com_example_cpppriceticker_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::WebSockets
    Qt6::Network
    KF6::Plasma
)

# Install the compiled plugin library to the correct Plasma directory
install(TARGETS com_example_cpppriceticker_plugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)