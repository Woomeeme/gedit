shared_module(
  'textsize',
  sources: files(
    'gedit-text-size-app-activatable.c',
    'gedit-text-size-plugin.c',
    'gedit-text-size-view-activatable.c',
    'gedit-text-size-window-activatable.c',
  ),
  include_directories: root_include_dir,
  dependencies: libgedit_dep,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
  name_suffix: module_suffix,
)

custom_target(
  'textsize.plugin',
  input: 'textsize.plugin.desktop.in',
  output: 'textsize.plugin',
  command: msgfmt_plugin_cmd,
  install: true,
  install_dir: get_option('libdir') / 'gedit/plugins',
)
