#ifndef TEXT_H
#define TEXT_H

struct Line {
    char *str;
    size_t len;
};

struct Text {
    char *buffer;
    Line *lines;
    size_t n_lines;
};

int text_ctor(Text *text, const char *input_file_name);
void text_dtor(Text *text);

#endif /* TEXT_H */
