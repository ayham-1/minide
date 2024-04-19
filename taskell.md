## Long Term Todo 

- add threading to font rendering
- optimize text_renderer for paragraph rendering
- add path expanding and variable resolving
- rename text_renderer_t to text_renderer
- design widget renderer system
- add rendering file to the buffer_view
- add chunking to the buffer_view
- add file_buffer interface w/ transaction system
- buffer_view: impl abstract of string text

## Short Term Todo

- buffers: fix reallocs & TODOs
- add line numbers
- add text_renderer_manager
- add multiple text_renderers handling for buffer_viewer

## Doing
- buffer_view: use prev & next to optimize horiz segs
- buffer_view: add arbitrary horizontal segment removal
- buffer_view: add arbitrary horizontal segment addition
- buffer_view: add arbitrary line removal
- buffer_view: add arbitrary appending

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
- design widget renderer system
- add vector type
- add node buffer viewer
- add buffer settings struct
- fix file_manager double free on closing application
- add view struct
- add spacing between lines setting
- text_renderer: localize heights of the line spacing per line for the maximum line height metric
- make coords consistent everywhere (top-left origin)
- buffer_view: render specific section of text based on the buffer_view dimensions
- add box debug rendering
- text_renderer: take into consideration off-screen rendering, somehow find a way to stop the execution of further segments
- text_renderer: allow for 0 wrappable line width
- buffer_view: use prev & next to optimize lines
