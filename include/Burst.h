/// \author Johannes de Fine Licht (johannes.definelicht@inf.ethz.ch)
/// \date April 2016
#pragma once
#include "ap_int.h"

namespace hlsutil {

namespace {

template <unsigned byteWidth>
struct UnsignedIntType {};

template <>
struct UnsignedIntType<sizeof(unsigned char)> {
  typedef unsigned char T;
};

template <>
struct UnsignedIntType<sizeof(unsigned short)> {
  typedef unsigned short T;
};

template <>
struct UnsignedIntType<sizeof(unsigned int)> {
  typedef unsigned int T;
};

template <>
struct UnsignedIntType<sizeof(unsigned long)> {
  typedef unsigned long T;
};

} // End anonymous namespace

template <typename T, unsigned byteWidth>
class Burst {

  typedef typename UnsignedIntType<sizeof(T)>::T Pack_t;
  static const int kElementsPerBurst = byteWidth / sizeof(T);
  static const int kBits = 8 * sizeof(T);

public:

  Burst() {}

  Burst(T const arr[kElementsPerBurst]) { 
    #pragma HLS INLINE
    Pack(arr);
  }

  void Pack(T const arr[kElementsPerBurst]) {
    #pragma HLS INLINE
    #pragma HLS PIPELINE II=1 enable_flush
Burst_Pack:
    for (int i = 0; i < kElementsPerBurst; ++i) {
      #pragma HLS UNROLL
      T element = arr[i];
      Pack_t temp = *reinterpret_cast<Pack_t const *>(&element);
      data_.range((i + 1) * kBits - 1, i * kBits) = temp;
    }
  }

  void Unpack(T arr[kElementsPerBurst]) const {
    #pragma HLS INLINE
    #pragma HLS PIPELINE II=1 enable_flush
Burst_Unpack:
    for (int i = 0; i < kElementsPerBurst; ++i) {
      #pragma HLS UNROLL
      Pack_t temp = data_.range((i + 1) * kBits - 1, i * kBits);
      arr[i] = *reinterpret_cast<T const *>(&temp);
    }
  }

  void operator<<(T const arr[kElementsPerBurst]) {
    #pragma HLS INLINE
    Pack(arr);
  }

  void operator>>(T arr[kElementsPerBurst]) const {
    #pragma HLS INLINE
    Unpack(arr);
  }

private:

  ap_uint<8 * byteWidth> data_;

};

} // End namespace hlsUtil
