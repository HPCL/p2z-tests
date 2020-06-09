########################
# Set the program name #
########################
#BENCHMARK = propagate

##########################################################
#       Set macros used for the input program            #
##########################################################
# COMPILER options: pgi, gcc, openarc, nvcc, icc, llvm   #
# MODE options: acc, omp, seq, cuda, tbb, eigen, alpaka, #
#               omp4                                     #
##########################################################
COMPILER ?= gcc
MODE ?= omp
TUNEB ?= 0
TUNETRK ?= 0

ifneq ($(TUNEB),0)
TUNE += -Dbsize=$(TUNEB)
endif
ifneq ($(TUNETRK),0)
TUNE += -Dntrks=$(TUNETRK)
endif

################
#  OMP Setting #
################
ifeq ($(MODE),omp)
CSRCS = propagate-toz-test_v4_tune.cpp
ifeq ($(COMPILER),gcc)
CXX=g++
CFLAGS1 += -O3 -I. -fopenmp 
CLIBS1 += -lm -lgomp
endif
ifeq ($(COMPILER),pgi)
CXX=pgc++
CFLAGS1 += -I. -Minfo=mp -fast -mp -Mnouniform -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(COMPILER),icc)
CXX=icc
CFLAGS1 += -Wall -I. -O3 -fopenmp -march=native -xHost -qopt-zmm-usage=high
endif
ifeq ($(COMPILER),llvm)
CXX=clang
CFLAGS1 += -Wall -O3 -I. -fopenmp -fopenmp-targets=x86_64 -lm
#CFLAGS1 = -Wall -O3 -I. -fopenmp -fopenmp-targets=nvptx64 -lm
endif
ifeq ($(COMPILER),ibm)
CXX=xlc
CFLAGS1 += -I. -Wall -v -O3 -qarch=pwr9 -qsmp=noauto:omp -qnooffload #host power9
endif
endif

#################
#  OMP4 Setting #
#################
ifeq ($(MODE),omp4)
CSRCS = propagate-toz-test_OpenMP4.c
ifeq ($(COMPILER),gcc)
CXX=gcc
CFLAGS1 += -O3 -I. -fopenmp -foffload="-lm"
CLIBS1 += -lm -lgomp 
endif
ifeq ($(COMPILER),llvm)
CXX=clang
CFLAGS1 = -Wall -O3 -I. -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -lm
#CFLAGS1 = -Wall -O3 -I. -fopenmp -fopenmp-targets=ppc64le-unknown-linux-gnu -lm
endif
ifeq ($(COMPILER),ibm)
CXX=xlc
CFLAGS1 += -I. -Wall -v -O3 -qsmp=omp -qoffload #device V100
endif
ifeq ($(COMPILER),openarc)
CXX=g++
CSRCS = ./cetus_output/propagate-toz-test_OpenMP4.cpp
# On Linux with CUDA GPU
CFLAGS1 += -O3 -I. -I${openarc}/openarcrt 
CLIBS1 += -L${openarc}/openarcrt -lcuda -lopenaccrt_cuda -lomphelper
# On macOS
#CFLAGS1 = -O3 -I. -I${openarc}/openarcrt -arch x86_64
#CLIBS1 = -L${openarc}/openarcrt -lopenaccrt_opencl -lomphelper -framework OpenCL
endif
endif


################
#  ACC Setting #
################
ifeq ($(MODE),acc)
CSRCS = propagate-toz-test_OpenACC.cpp
ifeq ($(COMPILER),pgi)
CXX=pgc++
CFLAGS1 += -I. -Minfo=acc -fast -Mfprelaxed -acc -ta=tesla -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(COMPILER),openarc)
CXX=g++
CSRCS = ./cetus_output/propagate-toz-test_OpenACC.cpp
# On Linux with CUDA GPU
CFLAGS1 += -O3 -I. -I${openarc}/openarcrt 
CLIBS1 += -L${openarc}/openarcrt -lcuda -lopenaccrt_cuda -lomphelper
# On macOS
#CFLAGS1 = -O3 -I. -I${openarc}/openarcrt -arch x86_64
#CLIBS1 = -L${openarc}/openarcrt -lopenaccrt_opencl -lomphelper -framework OpenCL
endif
endif

################
#  TBB Setting #
################
ifeq ($(MODE),tbb)
CSRCS = propagate-toz-test_tbb.cpp
TBB_PREIX := /opt/intel
CLIBS1+= -I${TBB_PREFIX}/include -L${TBB_PREFIX}/lib -Wl,-rpath,${TBB_PREFIX}/lib -ltbb
ifeq ($(COMPILER),gcc)
CXX=g++
CFLAGS1+=  -fopenmp -O3 -I.
CLIBS1 += -lm -lgomp -L/opt/intel/compilers_and_libraries/linux/tbb/lib/intel64/gcc4.8
endif
ifeq ($(COMPILER),icc)
CXX=icc
CFLAGS1+= -Wall -I. -O3 -fopenmp -march=native -xHost -qopt-zmm-usage=high
endif
endif


#################
#  CUDA Setting #
#################
ifeq ($(MODE),cuda)
CSRCS = propagate-toz-test_CUDA.cu
#CSRCS = propagate-toz-test_CUDA_v2.cu
ifeq ($(COMPILER),nvcc)
CXX=nvcc
CFLAGS1 += -arch=sm_70 -O3 -DUSE_GPU --default-stream per-thread
CLIBS1 += -L${CUDALIBDIR} -lcudart 
endif
endif

##################
#  EIGEN Setting #
##################
ifeq ($(MODE),eigen)
CSRCS = propagate-toz-test_Eigen.cpp
CLIBS1 += -I/mnt/data1/dsr/mkfit-hackathon/eigen -I/mnt/data1/dsr/cub -L${CUDALIBDIR} -lcudart 
ifeq ($(COMPILER),gcc)
CXX=g++
CFLAGS1 += -fopenmp -O3 -fopenmp-simd -lm -lgomp 
endif
ifeq ($(COMPILER),icc)
CXX=icc
CFLAGS1 += -fopenmp -O3 -fopenmp-simd  -mtune=native -march=native -xHost -qopt-zmm-usage=high
endif
ifeq ($(COMPILER),nvcc)
CXX=nvcc
CSRCS = propagate-toz-test_Eigen.cu
CFLAGS1 += -arch=sm_70 --default-stream per-thread -O3 --expt-relaxed-constexpr -I/mnt/data1/dsr/mkfit-hackathon/eigen -I/mnt/data1/dsr/cub
endif
endif

###################
#  ALPAKA Setting #
###################
ifeq ($(MODE),alpaka)
CSRCS = propagate-toz-test_alpaka.cpp
#CSRCS = alpaka_test.cpp
CLIBS1 = -I/mnt/data1/mgr85/p2z-tests/alpaka_lib/include 
ifeq ($(COMPILER),gcc)
CXX=g++
CFLAGS1+= -DALPAKA_ACC_CPU_BT_OMP4_ENABLED -DALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED -DALPAKA_ACC_CPU_B_SEQ_T_FIBERS_ENABLED -DALPAKA_ACC_CPU_B_OMP2_T_SEQ_ENABLED -DALPAKA_ACC_CPU_B_SEQ_T_OMP2_ENABLED -DALPAKA_ACC_CPU_B_SEQ_T_THREADS_ENABLED
#TBB_PREIX := /opt/intel
#CFLAGS1+= -I${TBB_PREFIX}/include -L${TBB_PREFIX}/lib -Wl,-rpath,${TBB_PREFIX}/lib -ltbb -DALPAKA_ACC_CPU_B_TBB_T_SEQ_ENABLED
CFLAGS1+= -fopenmp -O3 -I.
CLIBS1 += -lm -lgomp
endif
ifeq ($(COMPILER),nvcc)
CXX=nvcc
CFLAGS1+= -arch=sm_70 -O3 -DUSE_GPU --default-stream per-thread -DALPAKA_ACC_GPU_CUDA_ENABLED --expt-relaxed-constexpr --expt-extended-lambda 
CLIBS1 += -L${CUDALIBDIR} -lcudart
endif
endif


################################################
# TARGET is where the output binary is stored. #
################################################
TARGET = ./bin
BENCHMARK = "propagate_$(COMPILER)_$(MODE)"


$(TARGET)/$(BENCHMARK): $(CSRCS)
	if [ ! -d "$(TARGET)" ]; then mkdir bin; fi
	$(CXX) $(CFLAGS1) $(CSRCS) $(CLIBS1) $(TUNE) -o $(TARGET)/$(BENCHMARK)
	if [ -f $(TARGET)/*.ptx ]; then rm $(TARGET)/*.ptx; fi
	if [ -f "./cetus_output/openarc_kernel.cu" ]; then cp ./cetus_output/openarc_kernel.cu ${TARGET}/; fi
	if [ -f "./cetus_output/openarc_kernel.cl" ]; then cp ./cetus_output/openarc_kernel.cl ${TARGET}/; fi

clean:
	rm -f $(TARGET)/$(BENCHMARK) $(TARGET)/openarc_kernel.* $(TARGET)/*.ptx *.o

purge: clean
	rm -rf bin cetus_output openarcConf.txt 
