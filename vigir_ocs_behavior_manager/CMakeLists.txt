cmake_minimum_required(VERSION 2.8.3)
project(vigir_ocs_behavior_manager)

## Find catkin and any catkin packages on which
## this package depends at build time
find_package(catkin REQUIRED COMPONENTS roscpp rospy roslib std_msgs sensor_msgs geometry_msgs vigir_ocs_msgs flexbe_input message_runtime actionlib actionlib_msgs flexbe_msgs vigir_footstep_planning_msgs)# vigir_perception_msgs)


catkin_python_setup()

## Declare things to be passed to other projects
catkin_package(
    INCLUDE_DIRS src
    LIBRARIES ${PROJECT_NAME}
    CATKIN_DEPENDS roscpp rospy roslib std_msgs sensor_msgs geometry_msgs vigir_ocs_msgs flexbe_input message_runtime actionlib actionlib_msgs flexbe_msgs vigir_footstep_planning_msgs
)



## This plugin includes Qt widgets, so we must include Qt like so:
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

## I prefer the Qt signals and slots to avoid defining "emit", "slots",
## etc because they can conflict with boost signals, so define QT_NO_KEYWORDS here.
add_definitions(-DQT_NO_KEYWORDS)

## Here we specify which header files need to be run through "moc",
## Qt's meta-object compiler.

qt4_wrap_cpp(MOC_FILES
  src/behavior_relay.h
  src/behavior_notification.h
)


## Here we add QtCreator ui files
qt4_wrap_ui(UIC_FILES
  src/behavior_notification.ui
)

## Here we specify the list of source files, including the output of
## the previous command which is stored in ``${MOC_FILES}``.
set(SOURCE_FILES
  ${MOC_FILES}
#  src/behavior_confirmation.cpp
  src/behavior_relay.cpp
  src/behavior_notification.cpp
  src/main.cpp
  src/complex_action_server.h
  src/complex_action_server_imp.h

)
include_directories(include ${catkin_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} src)

#add_executable(vigir_ocs_behavior_manager_exe ${UIC_FILES} ${SOURCE_FILES})
#target_link_libraries(vigir_ocs_behavior_manager_exe ${catkin_LIBRARIES} ${QT_LIBRARIES})
#add_dependencies(vigir_ocs_behavior_manager_exe ${catkin_EXPORTED_TARGETS})

add_library(${PROJECT_NAME} ${UIC_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${QT_LIBRARIES})
add_dependencies(${PROJECT_NAME} ${catkin_EXPORTED_TARGETS})

#add_executable(vigir_ocs_behavior_manager_exe  ${SOURCE_FILES})

install( TARGETS ${PROJECT_NAME} #vigir_ocs_behavior_manager_exe
   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

catkin_install_python(PROGRAMS scripts/behavior_manager
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})



