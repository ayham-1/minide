project('minide', ['c', 'cpp'],
  version: '0.0.1')

sources = files([
  'src/app.c',
  'src/input.c',
  'src/texture_lender.c',
  'src/file_manager.c',

  'src/logger.c',
  'src/path.cpp',

  'src/types/hash_table.c',
  'src/types/vector.c',
  'src/types/stack.c',
  'src/types/hocis_vector.c',

  'src/glyph_cache.c',
  'src/fontconfig.c',
  'src/font.c',
  'src/font_manager.c',
  'src/shaper.c',
  'src/rsvg-port.c',

  'src/shaders_util.c',
  'src/text_renderer.c',

  'src/fps_counter.c',

  'src/ui/buffers/meta.c',
  'src/ui/buffers/node.c',

  'src/debug/border.c'
  ])

dep_list = [
  dependency('glew', version: '2.2.0'),
  dependency('freetype2', version: '>=26.1.20'),
  dependency('cglm', version: '>=0.9.1'),
  dependency('glfw3', version: '>=3.3.8'),
  dependency('harfbuzz', version: '>=8.3.0'),
  dependency('fontconfig', version: '>=2.14.2'),
  dependency('cairo', version: '>=1.18.0'),
  dependency('librsvg-2.0', version: '>=2.57.1'),

  dependency('icu-uc', version: '>=73.2'),
  dependency('icu-io', version: '>=73.2'),
  dependency('icu-i18n', version: '>=73.2'),
  ]

incdir = include_directories('include/')

libmin = static_library('min', sources, dependencies: dep_list, include_directories: incdir)

executable('minide', [sources, 'src/main.c'], link_with: [libmin], link_args: '-rdynamic', dependencies: dep_list, include_directories: incdir)

# tests

unit_tests = [
  'path',
  'fontconfig',
  'glyph_cache',

  'hash_table_insert_collision',
  'hash_table_insert_unifont',
  'hash_table_get',

  'text_renderer',
  'text_renderer_emoji',
  'unicode_emoji',

  'buffer_view',

  'input']

foreach unit_test : unit_tests
  e = executable('test_' + unit_test, 'tests/' + unit_test + '.c',
    link_with: libmin, dependencies: dep_list, d_unittest: true,
    override_options: ['b_pie=true'], link_args: '-rdynamic', include_directories: incdir)
  test(unit_test, e)
endforeach
