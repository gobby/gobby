conf = configuration_data()
conf.set_quoted('GOBBY_LOCALEDIR', get_option('prefix') / get_option('datadir') / 'locale')
conf.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('PRIVATE_ICONS_DIR', get_option('prefix') / get_option('datadir') / 'gobby-0.5' / 'icons')
conf.set_quoted('PUBLIC_ICONS_DIR', get_option('prefix') / get_option('datadir') / 'icons')

configure_file(
  output : 'features.hpp',
  configuration : conf)

ui_files = files([
  'resources/ui/browser-context-menu.ui',
  'resources/ui/connection-dialog.ui',
  'resources/ui/connection-info-dialog.ui',
  'resources/ui/document-location-dialog.ui',
  'resources/ui/entry-dialog.ui',
  'resources/ui/find-dialog.ui',
  'resources/ui/goto-dialog.ui',
  'resources/ui/initial-dialog.ui',
  'resources/ui/menu.ui',
  'resources/ui/open-location-dialog.ui',
  'resources/ui/password-dialog.ui',
  'resources/ui/preferences-dialog.ui',
  'resources/ui/toolbar.ui'
])

gobby_resources_h = custom_target(
  'resource-header',
  input : 'resources/gobby.gresources.xml',
  output : 'gobby-resources.h',
  command : ['glib-compile-resources', '@INPUT@', '--target=@OUTPUT@', '--sourcedir=' + meson.current_source_dir() / 'resources', '--c-name=_gobby', '--generate-header']
)

gobby_resources_c = custom_target(
  'resource-source',
  input : 'resources/gobby.gresources.xml',
  depend_files : ui_files,
  output : 'gobby-resources.c',
  command : ['glib-compile-resources', '@INPUT@', '--target=@OUTPUT@', '--sourcedir=' + meson.current_source_dir() / 'resources', '--c-name=_gobby', '--generate-source']
)

if target_machine.system() == 'windows'
  link_args = [
    '-lws2_32',
    '-Wl,--enable-runtime-pseudo-reloc',
    '-mwindows',
    gobby_res[0].full_path()
    ]
  link_depends = [gobby_res]
else
  link_args = []
  link_depends = []
endif

executable('gobby-0.5',
    sources : [
      gobby_resources_h,
      gobby_resources_c,
      'core/gobject/gobby-undo-manager.c',
      'dialogs/document-location-dialog.cpp',
      'dialogs/connection-info-dialog.cpp',
      'dialogs/connection-dialog.cpp',
      'dialogs/goto-dialog.cpp',
      'dialogs/preferences-dialog.cpp',
      'dialogs/password-dialog.cpp',
      'dialogs/entry-dialog.cpp',
      'dialogs/initial-dialog.cpp',
      'dialogs/find-dialog.cpp',
      'dialogs/open-location-dialog.cpp',
      'core/closableframe.cpp',
      'core/tablabel.cpp',
      'core/textundogrouping.cpp',
      'core/nodewatch.cpp',
      'core/foldermanager.cpp',
      'core/chatsessionview.cpp',
      'core/textsessionuserview.cpp',
      'core/browser.cpp',
      'core/menumanager.cpp',
      'core/toolbar.cpp',
      'core/preferences.cpp',
      'core/userlist.cpp',
      'core/statusbar.cpp',
      'core/folder.cpp',
      'core/selfhoster.cpp',
      'core/titlebar.cpp',
      'core/textsessionview.cpp',
      'core/noteplugin.cpp',
      'core/sessionuserview.cpp',
      'core/windowactions.cpp',
      'core/sessionview.cpp',
      'core/applicationactions.cpp',
      'core/server.cpp',
      'core/credentialsgenerator.cpp',
      'core/chattablabel.cpp',
      'core/userjoin.cpp',
      'core/huebutton.cpp',
      'core/texttablabel.cpp',
      'core/certificatemanager.cpp',
      'core/filechooser.cpp',
      'core/documentinfostorage.cpp',
      'core/knownhoststorage.cpp',
      'core/connectionmanager.cpp',
      'application.cpp',
      'util/closebutton.cpp',
      'util/config.cpp',
      'util/historyentry.cpp',
      'util/file.cpp',
      'util/asyncoperation.cpp',
      'util/uri.cpp',
      'util/serialize.cpp',
      'util/i18n.cpp',
      'window.cpp',
      'commands/file-tasks/task-open-file.cpp',
      'commands/file-tasks/task-save-all.cpp',
      'commands/file-tasks/task-export-html.cpp',
      'commands/file-tasks/task-open.cpp',
      'commands/file-tasks/task-open-multiple.cpp',
      'commands/file-tasks/task-save.cpp',
      'commands/file-tasks/task-new.cpp',
      'commands/file-tasks/task-open-location.cpp',
      'commands/application-commands.cpp',
      'commands/autosave-commands.cpp',
      'commands/edit-commands.cpp',
      'commands/help-commands.cpp',
      'commands/browser-context-commands.cpp',
      'commands/synchronization-commands.cpp',
      'commands/auth-commands.cpp',
      'commands/folder-commands.cpp',
      'commands/user-join-commands.cpp',
      'commands/view-commands.cpp',
      'commands/file-commands.cpp',
      'commands/browser-commands.cpp',
      'commands/subscription-commands.cpp',
      'main.cpp',
      'operations/operation-save.cpp',
      'operations/operation-open-multiple.cpp',
      'operations/operation-subscribe-path.cpp',
      'operations/operations.cpp',
      'operations/operation-delete.cpp',
      'operations/operation-new.cpp',
      'operations/operation-export-html.cpp',
      'operations/operation-open.cpp'
      ],
    dependencies : [
      glibmm_dep,
      giomm_dep,
      gtkmm_dep,
      gtksourceview_dep,
      libxmlpp_dep,
      libinfinity_dep,
      libinftext_dep,
      libinfgtk_dep,
      libinftextgtk_dep,
      sigcpp_dep
      ],
    link_args : link_args,
    link_depends : link_depends,
    install : true)
