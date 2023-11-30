## To Do

- replace the variable for gl_wrappepr.c configuration with a struct
- fonts: add fonts.c and use it in text_renderer
- create emoji_cache.c w/ testing

## Doing

- text_renderer: use UTF-8 characters for line wrapping

## Done

- add fontconfig.c w/ testing
- text_renderer: fix warnings for incorrect last segment wrapping
- text_renderer: impl forceful breaking when ubrk_open() fails
