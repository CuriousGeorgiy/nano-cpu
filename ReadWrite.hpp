#ifndef READWRITE_HPP
#define READWRITE_HPP

union ReadWriteMode {
    struct {
        unsigned constant: 1;
        unsigned reg: 1;
        unsigned ram: 1;
    };
    char mode;
};

typedef double constant_t;

#endif /* READWRITE_HPP */
