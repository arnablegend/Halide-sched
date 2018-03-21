#include "iostream"
#include "Halide.h"

int main(){
    //layer parameters
    int ifm_x = 8, ifm_y = 8, num_ifm = 64, num_ofm = 192;
    int kernel_x = 3, kernel_y = 3;
    int pad_x = 1, pad_y = 1, stride = 1, group = 1;

    //halide variables
	Halide::Var x("x"), y("y"), z("z");
	Halide::Func depthwise_conv("Depth wise conv"), preprocess("preprocess");
    Halide::Buffer<int32_t> in(ifm_x, ifm_y, num_ifm, "input");
    Halide::Buffer<int32_t> kernel(kernel_x, kernel_y, num_ifm / group, num_ofm, "kernel");
	Halide::Buffer<int32_t> out(ifm_x, ifm_y, num_ofm, "input");

    //Initializing kernel and IFM
    for(int i = 0; i < num_ifm; i++)
        for(int j = 0; j < ifm_y; j++)
            for(int k = 0; k < ifm_x; k++)
                in(k, j, i) = 1;

    for(int i = 0; i < num_ofm; i++)
        for(int j = 0; j < num_ifm / group; j++)
            for(int k = 0; k < kernel_y; k++)
                for(int l = 0; l < kernel_x; l++)
                    kernel(l, k, j, i) = 2;

    //R Domain for the kernel
    Halide::RDom r(0, kernel_x, 0, kernel_y, 0, num_ifm / group);

    //Algorithm
    //Zero Padding
    preprocess = Halide::BoundaryConditions::constant_exterior(
                    in, 0, 0, ifm_x, 0, ifm_y, 0, num_ifm);
    depthwise_conv(x, y, z)
         = Halide::sum(r, kernel(r.x, r.y, r.z, z)
                        * preprocess(x * stride + r.x - pad_x, y * stride + r.y - pad_y, (z / (num_ofm / group)) * (num_ifm / group) + r.z));

    //Realize and print
    depthwise_conv.realize(out);

    for(int i = 0; i < num_ofm; i++){
        for(int j = 0; j < ifm_y; j++){
            for(int k = 0; k < ifm_x; k++){
                std::cout << out(k, j, i) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

	return 0;
}
