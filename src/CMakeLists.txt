#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( pics )
add_subdirectory( servicemenus )

kde_import( libkonq )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES d3lphin.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES d3lphinui.rc DESTINATION ${DATA_INSTALL_DIR}/d3lphin )


##### d3lphin (executable) ######################

kde_add_executable( d3lphin AUTOMOC
  SOURCES
    bookmarkselector.cpp bookmarkssettingspage.cpp
    bookmarkssidebarpage.cpp
    detailsviewsettingspage.cpp dolphin.cpp
    dolphincontextmenu.cpp dolphindetailsview.cpp
    dolphindetailsviewsettings.cpp
    dolphindirlister.cpp dolphiniconsview.cpp
    dolphiniconsviewsettings.cpp dolphinsettings.cpp
    dolphinsettingsbase.cpp dolphinsettingsdialog.cpp
    dolphinstatusbar.cpp dolphinview.cpp
    editbookmarkdialog.cpp filterbar.cpp
    generalsettingspage.cpp iconsviewsettingspage.cpp
    infosidebarpage.cpp itemeffectsmanager.cpp
    main.cpp pixmapviewer.cpp progressindicator.cpp
    renamedialog.cpp settingspagebase.cpp
    sidebarpage.cpp sidebars.cpp sidebarssettings.cpp
    statusbarmessagelabel.cpp statusbarspaceinfo.cpp
    undomanager.cpp urlbutton.cpp urlnavigator.cpp
    urlnavigatorbutton.cpp viewproperties.cpp
    viewpropertiesdialog.cpp viewsettingspage.cpp
  LINK konq-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
