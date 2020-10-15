#include "text.h"

#include "error.h"
#include "memory_alloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>


int text_read_text_file_to_buffer(Text *text, const char *input_file_name)
{
    assert(text != NULL);
    assert(input_file_name != NULL);

    HANDLE input_file_handle = CreateFileA(input_file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);

    if (input_file_handle == INVALID_HANDLE_VALUE) {
        ERROR_OCCURRED_CALLING(CreateFileA, "returned INVALID_HANDLE_VALUE");

        return -1;
    }

    DWORD input_file_size = GetFileSize(input_file_handle, NULL);

    if (input_file_size == 0) {
        return 1;
    }
    HANDLE input_file_mapping_handle = CreateFileMappingA(input_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);

    if (input_file_mapping_handle == NULL) {
        ERROR_OCCURRED_CALLING(CreateFileMappingA, "returned NULL");

        if (CloseHandle(input_file_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file handle");
        }

        return -1;
    }

    LPVOID input_file_map_view = MapViewOfFile(input_file_mapping_handle, FILE_MAP_READ, 0, 0, 0);

    if (input_file_map_view == NULL) {
        ERROR_OCCURRED_CALLING(MapViewOfFile, "returned NULL");

        if (CloseHandle(input_file_mapping_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file mapping handle");
        }

        if (CloseHandle(input_file_handle) == 0) {
            ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file handle");
        }

        return -1;
    }

    if ((text->buffer = (char *) calloc(input_file_size + 2, sizeof(*text->buffer))) == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return -1;
    }

    memcpy(text->buffer + 1, input_file_map_view, input_file_size);

    int error_flag1 = 0, error_flag2 = 0, error_flag3 = 0;

    if ((error_flag1 = UnmapViewOfFile(input_file_map_view)) == 0) {
        ERROR_OCCURRED_CALLING(UnmapViewOfFile, "returned zero");
    }

    if ((error_flag2 = CloseHandle(input_file_mapping_handle)) == 0) {
        ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file mapping");
    }

    if ((error_flag3 = CloseHandle(input_file_handle)) == 0) {
        ERROR_OCCURRED_CALLING(CloseHandle, "returned zero on closing input file");
    }

    return !(error_flag1 && error_flag2 && error_flag3);
}

void text_count_lines_in_buffer(Text *text)
{
    assert(text->buffer != NULL);

    int pre_pre_c = EOF,
            pre_c     = EOF;

    text->n_lines = 0;

    const char *reader = text->buffer + 1;

    while (*reader) {
        if ((*reader == '\n') && (pre_c == '\r') && (pre_pre_c != '\n') && (pre_pre_c != EOF)) {
            ++text->n_lines;
        }

        pre_pre_c = pre_c;

        pre_c = (unsigned char) *(reader++);
    }

    if ((pre_c != '\n') && (pre_c != EOF) && (pre_pre_c != '\r')) {
        ++text->n_lines;
    }
}

int text_get_lines_from_buffer(Text *text)
{
    assert(text != NULL);

    text->lines = (Line *) calloc(text->n_lines + 1, sizeof(*text->lines));

    if (text->lines == NULL) {
        ERROR_OCCURRED_CALLING(calloc, "returned NULL");

        return 1;
    }

    if ((text->lines[0].str = strtok(text->buffer + 1, "\r\n")) == NULL) {
        ERROR_OCCURRED_CALLING(strtok, "returned NULL");

        return 1;
    }
    text->lines[0].str[-1] = '\0';
    text->lines[0].len = strlen(text->lines[0].str);

    for (size_t i = 1; i < text->n_lines; ++i) {
        if ((text->lines[i].str = strtok(NULL, "\r\n")) == NULL) {
            ERROR_OCCURRED_CALLING(strtok, "returned NULL");

            return 1;
        }

        text->lines[i].str[-1] = '\0';
        text->lines[i].len = strlen(text->lines[i].str);
    }

    return 0;
}

int text_ctor(Text *text, const char *input_file_name)
{
    assert(text != NULL);

    int error = text_read_text_file_to_buffer(text, input_file_name);

    if (error) {
        return error;
    }

    text_count_lines_in_buffer(text);

    return text_get_lines_from_buffer(text);
}

void text_dtor(Text *text)
{
    assert(text != NULL);

    FREE(text->lines);
    FREE(text->buffer);
}
