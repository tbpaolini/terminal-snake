#pragma once

#define WINDOWS_ERROR_CHECK(expr) if (!(expr))\
    {\
        windows_error_exit(__FILE__, __LINE__);\
    }

#define LINUX_ERROR_CHECK(expr) if ((expr) == -1)\
    {\
        printf_error_exit(\
            errno, "%d at [%s:%d]: %s.",\
            errno, __FILE__, __LINE__, strerror(errno)\
        );\
    }
