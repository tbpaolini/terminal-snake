#pragma once

#define LINUX_ERROR_CHECK(expr) if ((expr) == -1)\
    {\
        printf_error_exit(\
            errno, "%d at [%s:%d]: %s.",\
            errno, __FILE__, __LINE__, strerror(errno)\
        );\
    }
