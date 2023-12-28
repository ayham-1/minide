project('minide', 'c',
          version: '0.0.1')

sources = files([
    'src/gl_wrapper.c',
    'src/texture_lender.c',

    'src/logger.c',
    'src/u8string.c',
    'src/path.c',

    'src/types/hash_table.c',

    'src/glyph_cache.c',
    'src/fontconfig.c',
    'src/font.c',
    'src/font_manager.c',
    'src/shaper.c',

    'src/shaders_util.c',
    'src/text_renderer.c',

    'src/fps_counter.c'
  ])

dep_list = [
  dependency('glew', version: '2.2.0'),
  dependency('freetype2', version: '26.1.20'),
  dependency('cglm', version: '0.9.1'),
  dependency('glfw3', version: '>=3.3.8'),
  dependency('harfbuzz', version: '8.3.0'),
  dependency('fontconfig', version: '2.14.2'),

  dependency('icu-uc', version: '>=73.2'),
  dependency('icu-io', version: '>=73.2'),
  dependency('icu-i18n', version: '>=73.2'),
  ]

libmin = static_library('min', sources, dependencies: dep_list)

executable('minide', [sources, 'src/main.c'], link_with: [libmin], dependencies: dep_list)
