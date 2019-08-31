/**
 * @file simple_conv.cpp
 * @author Arnab Mitra
 */

#include "Halide.h"
#include "iostream"
#include <chrono>
#include "float.h"

using namespace std;
using namespace Halide;

#define NUM_RUNS (5)

typedef std::chrono::high_resolution_clock _clock;

/**
 * @brief Helper function to initialize Halide Buffers
 * templatized, arguments are
 * @params buf Halide::Buffer which is to be populated
 * @params val Value to populate the buffer with
 * @params dims Dimensions of the buffer. Default is 4
 * @retval None
 */
template <typename T>
inline void init_buffer(Buffer<T>& buf, int val, int dims = 4){
    if(dims == 4){
        for(int l = buf.min(3); l < buf.extent(3); l++)
            for(int k = buf.min(2); k < buf.extent(2); k++)
                for(int j = buf.min(1); j < buf.extent(1); j++)
                    for(int i = buf.min(0); i < buf.extent(0); i++)
                        buf(i, j, k, l) = val;
    }else if(dims == 1){
        for(int i = buf.min(0); i < buf.extent(0); i++)
            buf(i) = val;
    }
}

int main(){
    bool alternate = false, enable_loop_nest = false, enable_tracing = false;
    // Init Layer parameters
    int ifm_x = 299, ifm_y = 299, ifm_c = 3, ifm_n = 1;
    int k_width = 3, k_height = 3, num_k = 32;

    // Declare & Init Buffers
    // Halide Follows the WHCN format of declaring buffers
    Buffer<float> ifm(ifm_x, ifm_y, ifm_c, ifm_n);
    Buffer<float> ofm(ifm_x, ifm_y, num_k, ifm_n);
    // Need to setup the Layer Parameters such that IFM and OFM dimensions remain same
    Buffer<float> kernel(k_width, k_height, ifm_c, num_k);
    Buffer<float> bias(num_k); // One bias value per channel

    init_buffer<float>(ifm, 2);
    init_buffer<float>(kernel, 1);
    init_buffer<float>(bias, 1, 1);

    // Define Halide Funcs
    Func pre("preprocess"), conv("convolution");
    Var x("x"), y("y"), z("z"), n("n");
    RDom r(0, k_width, 0, k_height, 0, ifm_c);

    // Algorithm
    // Apply Zero padding
    pre = BoundaryConditions::constant_exterior(ifm, 0, 0, ifm_x, 0, ifm_y, 0, ifm_c, 0, ifm_n);
    // Two versions of the Convolution algorithm
    // As Kernel size is 3, choosing padding as 1
    if(!alternate){
        conv(x, y, z, n) = Halide::sum(kernel(r.x, r.y, r.z, z) * pre(x + r.x - 1, y + r.x - 1, z + r.z - 1, n))
                         + bias(z);
    }else{
        conv(x, y, z, n) = bias(z);
        conv(x, y, z, n) += (kernel(r.x, r.y, r.z, z) * pre(x + r.x - 1, y + r.x - 1, z + r.z - 1, n));
    }

    // Schedule
    conv.compute_root(); // Simple schedule

    if(enable_loop_nest)
        conv.print_loop_nest();
    if(enable_tracing)
        conv.trace_stores();

    // Execute and Time
    double min_time = FLT_MAX, duration[5];
    _clock::time_point start_time, end_time;
    for(int i = 0; i < NUM_RUNS; i++){
        start_time = _clock::now();
        conv.realize(ofm);
        end_time = _clock::now();
        duration[i] = (double)(chrono::duration_cast<chrono::nanoseconds>(end_time - start_time).count()) / 1.e6;
        if(duration[i] < min_time);
            min_time = duration[i];
    }
    cout << "Minimum time taken from " << NUM_RUNS << " runs : " << min_time << " ms" << endl;
    return 0;
}
