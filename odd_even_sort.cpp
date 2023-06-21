#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>

#include "worker.h"

void get_large(float* src1, int len1, float* src2, int len2, float* dst) {
  int pt1 = len1 - 1, pt2 = len2 - 1;
  for (int i = len1 - 1; i >= 0; i--) {
    if (pt2 < 0 || (pt1 >= 0 && src1[pt1] > src2[pt2])) {
      dst[i] = src1[pt1--];
    } else {
      dst[i] = src2[pt2--];
    }
  }
  memcpy(src1, dst, len1 * sizeof(float));
}

void get_small(float* src1, int len1, float* src2, int len2, float* dst) {
  int pt1 = 0, pt2 = 0;
  for (int i = 0; i < len1; i++) {
    if (pt2 >= len2 || (pt1 < len1 && src1[pt1] < src2[pt2])) {
      dst[i] = src1[pt1++];
    } else {
      dst[i] = src2[pt2++];
    }
  }
  memcpy(src1, dst, len1 * sizeof(float));
}

void Worker::sort() {
  /** Your code ... */
  // you can use variables in class Worker: n, nprocs, rank, block_len, data
  if (out_of_range) return;
  
  std::sort(data, data + block_len);
  if (nprocs == 1) return;

  size_t block_size = ceiling(n, nprocs);
  int block_len_left = (int)block_size;
  int block_len_right = (int)std::min(block_size, n - block_size * (rank + 1));

  // new buffer_zone for communication
  float *buffer = new float[std::max(block_len_left, block_len_right)];
  float *tmp = new float[block_len];

  for (int i = 0; i < nprocs; i++) {
    // phase 1: sort (even, odd)
    MPI_Request req[2];

    if (rank % 2) {
      // odd thread
      // send one for compare
      MPI_Irecv(buffer + block_len_left - 1, 1, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &req[0]);
      MPI_Isend(data, 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, &req[1]);
      MPI_Waitall(2, req, nullptr);

      if (data[0] < buffer[block_len_left - 1]) {
        MPI_Irecv(buffer, block_len_left - 1, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(data + 1, block_len - 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, &req[1]);
        MPI_Waitall(2, req, nullptr);
        get_large(data, block_len, buffer, block_len_left, tmp);
      }

    } else if (!last_rank) {
      // even thread
      MPI_Irecv(buffer, 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &req[0]);
      MPI_Isend(data + block_len - 1, 1, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &req[1]);
      MPI_Waitall(2, req, nullptr);

      if (data[block_len - 1] > buffer[0]) {
        MPI_Irecv(buffer + 1, block_len_right - 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(data, block_len - 1, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &req[1]);
        MPI_Waitall(2, req, nullptr);
        get_small(data, block_len, buffer, block_len_right, tmp);
      }

    }

    // phase 2: sort (odd, even)

    if (rank % 2) {
      if (!last_rank) {
        MPI_Irecv(buffer, 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(data + block_len - 1, 1, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &req[1]);
        MPI_Waitall(2, req, nullptr);

        if (data[block_len - 1] > buffer[0]) {
          MPI_Irecv(buffer + 1, block_len_right - 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &req[0]);
          MPI_Isend(data, block_len - 1, MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, &req[1]);
          MPI_Waitall(2, req, nullptr);
          get_small(data, block_len, buffer, block_len_right, tmp);
        }
      }
    } else if (rank > 0) {
      MPI_Irecv(buffer + block_len_left - 1, 1, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &req[0]);
      MPI_Isend(data, 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, &req[1]);
      MPI_Waitall(2, req, nullptr);

      if (data[0] < buffer[block_len_left - 1]) {
        MPI_Irecv(buffer, block_len_left - 1, MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(data + 1, block_len - 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, &req[1]);
        MPI_Waitall(2, req, nullptr);
        get_large(data, block_len, buffer, block_len_left, tmp);
      }
    }

  }

  delete[] buffer;
  delete[] tmp;
}
