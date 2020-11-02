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

    explicit Text(const char *inputFileName);

    Text(const Text &) = delete;
    Text(Text &&) = delete;

    Text &operator=(const Text &) = delete;
    Text &operator=(Text &&) = delete;

    ~Text();
private:
    char *buf;
    const char *inputFileName;

    void readTextFileToBuf();
    void countLinesInBuf();
    void splitBufIntoLines();
};

#endif /* TEXT_HPP */
