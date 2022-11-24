
#include"cfiddle.hpp"
#include"tensor_t.hpp"
#include"util.hpp"
#include<cstdint>
#include<cassert>

extern "C"
void do_convolution_tiled(const tensor_t<uint32_t> & source,
                          const tensor_t<uint32_t> & kernel,
                          tensor_t<uint32_t> & target, int32_t tile_size) {
        disable_prefetcher();
        const int32_t real_tile_size = 8;

	start_measurement();
	for(int32_t i = 0; i < target.size.x; i++) {
		for(int32_t jj = 0; jj < kernel.size.x; jj += real_tile_size) {  // Move the jj chunk loop outside
			if (jj + real_tile_size < kernel.size.x) {
				for(int32_t j = jj; j < jj + real_tile_size; j++) {
					target.get(i,0,0,0) += source.get(i + j,0,0,0) * kernel.get(j,0,0,0);
				} 
			} else if (jj + real_tile_size == kernel.size.x) {
				for(int32_t j = jj; j < kernel.size.x; j++) {
					target.get(i,0,0,0) += source.get(i + j,0,0,0) * kernel.get(j,0,0,0);
                    target.get(i,0,0,0) += source.get(i + j+1,0,0,0) * kernel.get(j+1,0,0,0);
                    j++;
				}
			} else {
				for(int32_t j = jj; j < kernel.size.x; j++) {
					target.get(i,0,0,0) += source.get(i + j,0,0,0) * kernel.get(j,0,0,0);
                    target.get(i,0,0,0) += source.get(i + j+1,0,0,0) * kernel.get(j+1,0,0,0);
                    j++;
				}
			}
		}
	}
	end_measurement();
}

extern "C"
void convolution_tiled(uint64_t source_size,  uint64_t kernel_size, int32_t tile_size) {
        tensor_t<uint32_t> source(source_size,1,1,1);
        tensor_t<uint32_t> kernel(kernel_size,1,1,1);
        uint64_t target_size = source_size - kernel_size;
        tensor_t<uint32_t> target(target_size,1,1,1);

        do_convolution_tiled(source, kernel, target, tile_size);
}

// Cfiddle-signature=e475919535ec99f066cff96db13029cc