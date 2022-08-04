#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "./editor.h"

#define LINE_INIT_CAPACITY 1024

static void line_grow(Line *line, size_t n)
{
    size_t new_capacity = line->capacity;

    while (new_capacity - line->size < n) {
        if (new_capacity == 0) {
            new_capacity = LINE_INIT_CAPACITY;
        } else {
            new_capacity *= 2;
        }
    }
    if (new_capacity != line->capacity) {
        line->chars = realloc(line->chars, new_capacity);
    }
}

void line_insert_text_before(Line *line, const char* text, size_t *col)
{

    if (*col > line->size) {
        *col = line->size;
    }

    const size_t text_size = strlen(text);
    line_grow(line, text_size);

    memmove(line->chars + *col + text_size,
            line->chars + *col,
            line->size - *col);
    memcpy(line->chars + *col, text, text_size);
    line->size += text_size;
}

void line_backspace(Line *line, size_t *col)
{
    if (*col > line->size) {
        *col = line->size;
    }

    if (*col > 0 && line->size > 0) {
        memmove(line->chars + *col - 1, line->chars + *col, line->size - *col);
        line->size -= 1;
    }
}

void line_delete(Line *line, size_t *col)
{
    if (*col > line->size) {
        *col = line->size;
    }

    if (*col < line->size && line->size > 0) {
        memmove(line->chars + *col,
                line->chars + *col + 1,
                line->size - *col - 1);
        line->size -= 1;
    }
}

static void editor_push_new_line(editor)
{
    assert(false && "not implemented yet");
}

void editor_insert_text_before_cursor(Editor *editor, const char *text)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size - 1;
        } else {
            editor_push_new_line(editor);
        }
    }

    line_insert_text_before(&editor->lines[editor->cursor_row], text, &editor->cursor_col);
}

void editor_backspace(Editor *editor)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size - 1;
        } else {
            editor_push_new_line(editor);
        }
    }
    line_backspace(&editor->lines[editor->cursor_row], &editor->cursor_col);
}

void editor_delete(Editor *editor)
{
    if (editor->cursor_row >= editor->size) {
        if (editor->size > 0) {
            editor->cursor_row = editor->size - 1;
        } else {
            editor_push_new_line(editor);
        }
    }

    line_delete(&editor->lines[editor->cursor_row], &editor->cursor_col);
}
