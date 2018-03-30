#include "Halide.h"
#include "iostream"
#include "papi.h"
#include "time.h"
#include "cstdlib"

inline float gettime(){
	return((float)PAPI_get_virt_usec());
}

int main(int argc, char** argv){
	//PAPI variables
    float t0, t1;
    int events[2] ={PAPI_L1_DCM, PAPI_FP_OPS }, ret;
    long_long values[2];
    if (PAPI_num_counters() < 2) {
        fprintf(stderr, "No hardware counters here, or PAPI not supported.\n");
        exit(1);
    }

	int a_height = 650, a_width = 650;
	int b_height = 650, b_width = 650;	
	
	Halide::Buffer<int32_t> mat_a(a_width, a_height, "Matrix a");	
	Halide::Buffer<int32_t> mat_b(b_width, b_height, "Matrix b");
	Halide::Buffer<int32_t> mat_res(b_width, a_height, "Result matrix");

	Halide::Var x("x"), y("y"), z("z"), x_t("x at transpose"), y_t("y at transpose"), x_r("x at sum"), y_r("y at sum");

	Halide::Func mat_b_t("Matrix b transpose");
	Halide::Func p_prod("partial product");
	Halide::Func prod("Result");
	Halide::RDom r(0, a_width);


	//------------------Initialize-----------------//
	for(int j = 0; j < a_height; j++)
		for(int i = 0; i < a_width; i++)
			mat_a(i, j) = 2;

	for(int j = 0; j < b_height; j++)
		for(int i = 0; i < b_width; i++)
			mat_b(i, j) = 1;

	//------------------Algorithm------------------//
	mat_b_t(x_t, y_t) = mat_b(y_t, x_t);
	p_prod(x, y, z) = mat_a(z, y) * mat_b_t(z, x);	
	prod(x_r, y_r) = Halide::sum(p_prod(x_r, y_r, r.x));

	//------------------Schedule-------------------//
	mat_b_t.compute_inline();
	p_prod.compute_root();

	//---------------Perf measure------------------//
	double best_time = 0.0;
    //float total_sw_flops_at_best;
    long long total_hw_flops_at_best;
    long long total_cache_misses_at_best;
   	int eventSet = PAPI_NULL;
	if ((ret = PAPI_create_eventset(&eventSet)) != PAPI_OK) {
       	fprintf(stderr, "PAPI failed to create eventset : %s\n", PAPI_strerror(ret));
        exit(1);
    }
	if ((ret = PAPI_add_event(eventSet,  PAPI_L1_DCM)) != PAPI_OK) {
        fprintf(stderr, "PAPI failed to add L1_DCM: %s\n", PAPI_strerror(ret));
        exit(1);
    }
	if ((ret = PAPI_add_event(eventSet, PAPI_FP_OPS)) != PAPI_OK) {
    	fprintf(stderr, "PAPI failed to add FP_OPS: %s\n", PAPI_strerror(ret));
        exit(1);
    }
	PAPI_set_debug(PAPI_VERB_ESTOP);
	for(int i = 0; i < 3; i++){
    	t0 = gettime();
    	if ((ret = PAPI_start_counters(events, 2)) != PAPI_OK) {
            fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
            exit(1);
    	}
    	//Realize
    	prod.realize(mat_res);
    	if ((ret = PAPI_read_counters(values, 2)) != PAPI_OK) {
       		fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
        	exit(1);
   		}
    	t1 = gettime();
		double elapsed = (double)(t1-t0);
		if(elapsed < best_time || i == 0){
    		best_time = elapsed;
   			total_hw_flops_at_best = values[1];
    		total_cache_misses_at_best = values[0];
		}
		if ((ret = PAPI_reset(eventSet)) != PAPI_OK) {
            fprintf(stderr, "PAPI failed to reset counters: %s\n", PAPI_strerror(ret));
            exit(1);
    	}
		std::cout << "End of Run " << i + 1 << std::endl;
		//break; 
	}
	std::cout << "Time taken is " << best_time/1000000  << std::endl;
    std::cout << "Hardware Flops - " << total_hw_flops_at_best << std::endl << "Cache misses - " << total_cache_misses_at_best <<std::endl;
	
	return 0;
}
