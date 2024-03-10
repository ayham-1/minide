## To Do

- add threading to font rendering
- optimize text_renderer for paragraph rendering
- add file_buffer interface
- add path expanding and variable resolving

## Doing


## Done

- add fontconfig.c w/ testing
- text_renderer: fix warnings for incorrect last segment wrapping
- text_renderer: impl forceful breaking when ubrk_open() fails
- text_renderer: use UTF-8 characters for line wrapping
- replace the variable for gl_wrappepr.c configuration with a struct
- create emoji_cache.c w/ testing
- fonts: add fonts.c and use it in text_renderer
- shaper: do emoji & text rendering
- add backtraces to log_error
- reallocs log_info should be log_debug
- fix emoji detection
- add new line wrapping
- add unicode emoji test
- add INPUT_H
- add buffer interface
- add file interface
- add file manager
