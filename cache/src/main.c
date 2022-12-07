#include <stdio.h>

#pragma GCC diagnostic ignored "-fpermissive"

//extern "C" {
#include <buf0buf.h>
//}

// extern buf_pool_t *buf_pool_init(ulint max_size, ulint curr_size, ulint n_frames);

int main(int argc, const char** argv) {



    ulint max_size = 512;
    ulint curr_size = 10;
    ulint n_frames = 512;

    buf_pool_t *pool = buf_pool_init(max_size, curr_size, n_frames);
    // std::cout << pool->n_frames << std::endl;

    printf("%d\n", pool->n_frames);

    return 0;
}

