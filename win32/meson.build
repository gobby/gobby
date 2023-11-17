windows = import('windows')

conf = configuration_data()
conf.set('VERSION', meson.project_version())
configure_file(
  input : 'gobby.iss.in',
  output : 'gobby.iss',
  configuration : conf)

gobby_res = windows.compile_resources(
  'gobby.rc',
  depend_files : ['gobby.ico'])
