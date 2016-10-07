/// \author Johannes de Fine Licht (johannes.definelicht@inf.ethz.ch)
/// \date October 2016
#include "Config.h"

void Compute(Burst const *input, Burst *output);

// Top level function
void Kernel(Burst const input[kBursts], Burst output[kBursts],
            const unsigned iterations) {

  // Interface specifications to conform with SDAccel
  #pragma HLS INTERFACE m_axi port=input offset=slave bundle=gMemIn
  #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gMemOut
  #pragma HLS INTERFACE s_axilite port=input
  #pragma HLS INTERFACE s_axilite port=output
  #pragma HLS INTERFACE s_axilite port=iterations
  #pragma HLS INTERFACE s_axilite port=return

  // Will be stored as BRAM for large arrays
  Burst internalBuffer[kBursts];

  // Read values from off-chip memory into internal buffer
PopulateInternal:
  for (unsigned i = 0; i < kBursts; ++i) {
    #pragma HLS PIPELINE II=1
    internalBuffer[i] = input[i]; 
  }

  // Flatten loop
  const unsigned totalIterations = iterations * kBursts;

MainInternal:
  for (unsigned i = 0; i < totalIterations; ++i) {
    #pragma HLS LOOP_TRIPCOUNT min=kBursts
    #pragma HLS PIPELINE II=1

    Burst inputBurst, outputBurst;

    // Read once every cycle from buffer
    inputBurst = internalBuffer[i % kBursts]; 

    // Do computation
    Compute(&inputBurst, &outputBurst);

    // Write once every cycle to buffer
    internalBuffer[i % kBursts] = outputBurst;
  }

  // Write results back to off-chip memory
ClearInternal:
  for (int i = 0; i < kBursts; ++i) {
    #pragma HLS PIPELINE II=1 enable_flush
    output[i] = internalBuffer[i];
  }

}

void Compute(Burst const *input, Burst *output) {
  #pragma HLS PIPELINE II=1

  // Local registers at each pipeline stage
  static Element_t buffers[kDepth+1][kElementsPerBurst];
  #pragma HLS ARRAY_PARTITION variable=buffers complete dim=1

  // Read in burst to first register 
  input->Unpack(buffers[0]);

  // Unroll number of pipeline stages
Depth:
  for (int d = 0; d < kDepth; ++d) {

    // Unroll width of pipeline
  Width:
    for (int i = 0; i < kElementsPerBurst; ++i) {

      // Read from register of previous stage
      Element_t propagate = buffers[d][i];

      // A value so that the result after all additions will be twice the input
      static const Element_t kAddVal =
          kFillVal * ((1 << kMultsPerStage) - 1) / kAddsPerStage;
      static const Element_t kMultVal = 0.5;

    AddsPerStage:
      for (int j = 0; j < kAddsPerStage; ++j) {
        Element_t eval = propagate + kAddVal;
        propagate = eval;
      }

    MultsPerStage:
      for (int j = 0; j < kMultsPerStage; ++j) {
        Element_t eval = propagate * kMultVal;
        propagate = eval;
      }

      // Write to register of next stage
      buffers[d + 1][i] = propagate;
    }
  }

  // Write back result in last register
  *output = Burst(buffers[kDepth]);
}
