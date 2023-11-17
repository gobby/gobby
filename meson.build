project('gobby', ['cpp', 'c'],
  version : '0.7.0',
  license : 'ISC',
  default_options : ['cpp_std=c++11'])
appname = 'gobby-0.5'

## TODO: WIN32 (+resources), make docs conditional

gnome = import('gnome')
i18n = import('i18n')

gettext_package = 'gobby05'
add_project_arguments('-DGETTEXT_PACKAGE="' + gettext_package + '"', language : 'cpp')
subdir('po')

if target_machine.system() == 'windows'
  subdir('win32')
endif

libxmlpp_dep = dependency('libxml++-2.6')
glibmm_dep = dependency('glibmm-2.4', version : '>= 2.39.93')
giomm_dep = dependency('giomm-2.4', version : '>= 2.39.93')
libgsasl_dep = dependency('libgsasl', version : '>= 0.2.21')
gnutls_dep = dependency('gnutls', version : '>= 3.0.20')
gtkmm_dep = dependency('gtkmm-3.0', version : '>= 3.6.0')
gtksourceview_dep = dependency('gtksourceview-4')

libinfinity_dep = dependency('libinfinity-0.7')
libinftext_dep = dependency('libinftext-0.7')
libinfgtk_dep = dependency('libinfgtk-0.7')
libinftextgtk_dep = dependency('libinftextgtk-0.7')

subdir('code')

conf = configuration_data()
conf.set('GETTEXT_PACKAGE', gettext_package)
configure_file(
  input : 'de.0x539.gobby.gschema.xml.in',
  output : 'de.0x539.gobby.gschema.xml',
  configuration : conf,
  install : true,
  install_dir : get_option('datadir') / 'glib-2.0' / 'schemas')

if target_machine.system() != 'windows'
  desktop_file = i18n.merge_file(
    input : 'gobby-0.5.desktop.in',
    output : 'gobby-0.5.desktop',
    type : 'desktop',
    po_dir : 'po',
    install : true,
    install_dir : get_option('datadir') / 'applications',
  )

  metainfo_file = i18n.merge_file(
    input : 'gobby-0.5.metainfo.xml.in',
    output : 'gobby-0.5.metainfo.xml',
    type : 'xml',
    po_dir : 'po',
    install : true,
    install_dir : get_option('datadir') / 'metainfo',
  )
  appstreamcli = find_program('appstream-util', required: false)
  if appstreamcli.found()
    test(
      'Validate metainfo file',
      appstreamcli,
      args: ['validate-relax', '--nonet', metainfo_file.full_path() ]
    )
  endif
endif

private_icons = install_subdir(
  'icons',
  exclude_directories : [
    'hicolor/48x48/apps',
    'hicolor/scalable/apps',
    'HighContrastLargePrint/48x48/apps',
    'HighContrastLargePrint/scalable/apps',
    'HighContrastLargePrintInverse/48x48/apps',
    'HighContrastLargePrintInverse/scalable/apps'
  ],
  install_dir : get_option('datadir') / appname)
public_icons = install_subdir(
  'icons',
  exclude_directories : [
    'hicolor/48x48/status',
    'hicolor/48x48/actions',
    'hicolor/scalable/status',
    'hicolor/scalable/actions',
    'HighContrastLargePrint/48x48/actions',
    'HighContrastLargePrint/scalable/actions',
    'HighContrastLargePrintInverse/48x48/actions',
    'HighContrastLargePrintInverse/scalable/actions'
  ],
  install_dir : get_option('datadir'))

install_man('gobby-0.5.1')
if get_option('docs')
  subdir('help')
endif

if target_machine.system() != 'windows'
  gnome.post_install(
    glib_compile_schemas : true,
    gtk_update_icon_cache : true,
    update_desktop_database : true)
endif
