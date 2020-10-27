#ifndef TEXT_HPP
#define TEXT_HPP

class Text {
public:
    struct Line {
        char *str;
        size_t len;
    };

    Line *lines;
    size_t nLines;

    Text(const char *inputFileName);

    Text(const Text &) = delete;
    Text(Text &&) = delete;

    Text &operator=(const Text &) = delete;
    Text &operator=(Text &&) = delete;

    ~Text();
private:
    char *buffer;
    const char *inputFileName;

    void readTextFileToBuffer();
    void countLinesInBuffer();
    void parseBuffer();
};

#endif /* TEXT_HPP */
