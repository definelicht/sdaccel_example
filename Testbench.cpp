/// \author Johannes de Fine Licht (johannes.definelicht@inf.ethz.ch)
/// \date October 2016
#include "Kernel.h"
#include <iostream>
#include <vector>

int main(const int argc, char const *const *const argv) {

  int nIterations;
  if (argc != 2) {
    std::cerr << "Using default number of operations " << kIterations << ".\n";
    nIterations = kIterations;
  } else {
    nIterations = std::stoi(argv[1]);
  }

  std::vector<Burst> inputBursts(kBursts);
  {
    // Initialize input
    Burst inBurst;
    std::vector<Element_t> inArray(kElementsPerBurst, kFillVal);
    inBurst.Pack(inArray.data());
    for (int i = 0; i < kBursts; ++i) {
      inputBursts[i] = inBurst;
    }
  }

  std::vector<Burst> outputBursts(kBursts);

  // Run kernel
  Kernel(inputBursts.data(), outputBursts.data(), nIterations);

  // Verify results
  std::vector<Element_t> outArray(kElementsPerBurst, 0);
  for (int i = 0; i < kBursts; ++i) {
    outputBursts[i].Unpack(outArray.data());
    for (int j = 0; j < kElementsPerBurst; ++j) {
      if (outArray[j] != kFillVal) {
        std::cerr << "Mismatch at " << i << ": " << outArray[j]
                  << " (should be " << kFillVal << ")\n";
        return 1;
      }
    }
  }

  return 0;
} 
