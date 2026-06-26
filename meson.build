project(
  'tagore',
  'c',
  version: '0.1.0',
  license: 'MIT',
  meson_version: '>= 1.3.0',
  default_options: [
    'c_std=c17',
    'warning_level=3',
    'werror=false',
  ],
)

gnome = import('gnome')

tagore_prefix = get_option('prefix')
tagore_datadir = join_paths(tagore_prefix, get_option('datadir'))
tagore_bindir = join_paths(tagore_prefix, get_option('bindir'))
tagore_app_id = 'org.tagore.Tagore'

add_project_arguments(
  [
    '-DGETTEXT_PACKAGE="tagore"',
    '-DTAGORE_APP_ID="@0@"'.format(tagore_app_id),
    '-DTAGORE_LOCALEDIR="@0@"'.format(join_paths(tagore_datadir, 'locale')),
  ],
  language: 'c',
)

tagore_deps = [
  dependency('gtk4', version: '>= 4.14'),
  dependency('libadwaita-1', version: '>= 1.5'),
]

tagore_core_deps = [
  dependency('glib-2.0', version: '>= 2.78'),
  dependency('gio-2.0', version: '>= 2.78'),
  dependency('gobject-2.0', version: '>= 2.78'),
]

tagore_inc = include_directories('include')

tagore_sources = files(
  'src/main.c',
  'src/canvas/tagore-canvas.c',
  'src/core/tagore-application.c',
  'src/file/tagore-recent-files.c',
  'src/image/tagore-image-loader.c',
  'src/processing/tagore-edit-engine.c',
  'src/processing/tagore-edit-state.c',
  'src/ui/tagore-window.c',
  'src/config/tagore-paths.c',
)

executable(
  'tagore',
  tagore_sources,
  dependencies: tagore_deps + tagore_core_deps,
  include_directories: tagore_inc,
  install: true,
  install_dir: tagore_bindir,
)

test_config_paths = executable(
  'test-config-paths',
  files(
    'tests/test-config-paths.c',
    'src/config/tagore-paths.c',
  ),
  dependencies: tagore_core_deps,
  include_directories: tagore_inc,
)

test('config paths', test_config_paths)

test_recent_files = executable(
  'test-recent-files',
  files(
    'tests/test-recent-files.c',
    'src/config/tagore-paths.c',
    'src/file/tagore-recent-files.c',
  ),
  dependencies: tagore_core_deps,
  include_directories: tagore_inc,
)

test('recent files', test_recent_files)

test_edit_state = executable(
  'test-edit-state',
  files(
    'tests/test-edit-state.c',
    'src/processing/tagore-edit-state.c',
  ),
  dependencies: tagore_core_deps,
  include_directories: tagore_inc,
)

test('edit state', test_edit_state)

test_edit_engine = executable(
  'test-edit-engine',
  files(
    'tests/test-edit-engine.c',
    'src/processing/tagore-edit-engine.c',
    'src/processing/tagore-edit-state.c',
  ),
  dependencies: tagore_deps + tagore_core_deps,
  include_directories: tagore_inc,
)

test('edit engine', test_edit_engine)

subdir('po')
