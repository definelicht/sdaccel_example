/// \author Johannes de Fine Licht (johannes.definelicht@inf.ethz.ch)
/// \date October 2016
#include "Kernel.h"
#include "OpenCL.h"

int main(int argc, char const **argv) {

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
  double elapsed = 0;

  try {
    ocl::Context context(std::string("Xilinx"), std::string(kDeviceString));

    auto inputBuffer = context.MakeBuffer<Burst, ocl::Access::read>(
        inputBursts.cbegin(), inputBursts.cend());
    auto outputBuffer =
        context.MakeBuffer<Burst, ocl::Access::write>(kBursts);

    auto kernel = context.MakeKernelFromBinary(
        "Kernel.xclbin", "Kernel", inputBuffer, outputBuffer,
        nIterations);

    std::cout << "Processing " << kElements << " elements for "
              << nIterations << " iterations, performing " << kAddsPerStage 
              << " additions and " << kMultsPerStage
              << " multiplications at each of " << kDepth
              << " pipeline stages treating " << kElementsPerBurst
              << " elements in patallel.\n" << std::flush;

    elapsed = kernel.ExecuteTask();

    unsigned long nOps = static_cast<unsigned long>(kElements) * nIterations *
                         kDepth * kElementsPerBurst *
                         (kAddsPerStage + kMultsPerStage);

    std::cout << "Finished " << nOps << " operations in " << elapsed
              << " seconds (" << (1e-9 * nOps) / elapsed << " GOps/s).\n";

    outputBuffer.CopyToHost(outputBursts.begin());

  } catch (std::runtime_error err) {
    std::cerr << "Hardware execution failed with error: " << err.what() << "\n";
    return 1;
  }

  // Verification
  for (int i = 0; i < kBursts; ++i) {
    Element_t burst[kElementsPerBurst];
    outputBursts[i] >> burst;
    for (int j = 0; j < kElementsPerBurst; ++j) {
      if (burst[j] != kFillVal) {
        std::cerr << "Mismatch at " << j << ": " << burst[j] << " (should be "
                  << kFillVal << ")\n";
        return 1;
      }
    }
  }

  return 0;
}
