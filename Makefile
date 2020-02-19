<<<<<<< HEAD
#icc propagate-toz-test.C -o propagate-toz-test.exe -fopenmp -O3
CC = icc #gcc throws errors for sinf functions etc 
PGC = pgc++ -acc -L${PGI} -ta=tesla:managed -fPIC -Minfo -Mfprelaxed 
#LDFLAGS += -fopenmp -O3
CXXFLAGS +=  -I/nfs/opt/cuda-10-1/include -fopenmp -O3  #-DUSE_GPU 
NVCC = nvcc
CUDAFLAGS += -arch=sm_70 -O3 -DUSE_GPU#-rdc=true #-L${CUDALIBDIR} -lcudart 
CUDALDFLAGS += -L${CUDALIBDIR} -lcudart

TYPE = icc
#TYPE = cuda
#TYPE = pgi


ifeq ($(TYPE),icc)
COMP = ${CC}
FLAGS = ${CXXFLAGS}
endif
ifeq ($(TYPE),cuda)
COMP = ${NVCC}
FLAGS =  ${CUDAFLAGS} --default-stream per-thread #${CXXFLAGS} -DUSE_GPU 
endif


ifeq ($(TYPE),pgi)
COMP = ${PGC}
FLAGS = -DUSE_ACC
endif


ifeq ($(TYPE),cuda)
propagate : propagate-toz-test-main.o propagate-toz-test.o propagateGPU.o
	$(COMP) $(FLAGS) $(CUDALDFLAGS) -o propagate propagate-toz-test-main.o propagateGPU.o
propagateGPU.o : propagateGPU.cu propagateGPU.h
	$(NVCC) $(FLAGS) -o propagateGPU.o -dc propagateGPU.cu
propagate-toz-test-main.o : propagate-toz-test-main.cc propagateGPU.h
	$(COMP) $(FLAGS) -o propagate-toz-test-main.o -dc propagate-toz-test-main.cc

else
propagate : propagate-toz-test-main.o propagate-toz-test.o
	$(COMP) $(FLAGS) $(CUDALDFLAGS) -o propagate propagate-toz-test.o propagate-toz-test-main.o
propagate-toz-test.o : propagate-toz-test.C propagateGPU.h
#propagate-toz-test.o : propagate-toz-test.C propagate-toz-test.h
	$(COMP) $(FLAGS) -o propagate-toz-test.o -c propagate-toz-test.C
propagate-toz-test-main.o : propagate-toz-test-main.cc propagateGPU.h
#propagate-toz-test-main.o : propagate-toz-test-main.cc propagate-toz-test.h
	$(COMP) $(FLAGS) -o propagate-toz-test-main.o -c propagate-toz-test-main.cc
endif
clean:
	rm -rf propagate *.o 
=======
########################
# Set the program name #
########################
BENCHMARK = propagate

#########################################
# Set macros used for the input program #
#########################################
# COMPILER options: pgi, gcc, openarc   #
# SRCTYPE options: cpp, c               #
# MODE options: acc, omp, seq           #
#########################################
COMPILER ?= pgi
SRCTYPE ?= cpp
MODE ?= acc

######################################
# Set the input source files (CSRCS) #
######################################
ifeq ($(SRCTYPE),cpp)
ifeq ($(MODE),acc)
CSRCS = propagate-toz-test_OpenACC.cpp
else
CSRCS = propagate-toz-test.cpp
endif
else
ifeq ($(MODE),acc)
CSRCS = propagate-toz-test_OpenACC.c
else
CSRCS = propagate-toz-test.c
endif
endif


ifeq ($(COMPILER),pgi)
###############
# PGI Setting #
###############
ifeq ($(SRCTYPE),cpp)
CXX=pgc++
ifeq ($(MODE),acc)
CFLAGS1 = -I. -Minfo=acc -fast -Mfprelaxed -acc -ta=tesla -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(MODE),omp)
CFLAGS1 = -I. -Minfo=mp -fast -mp -Mnouniform -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(MODE),seq)
CFLAGS1 = -I. -fast 
endif
else
CXX=pgcc
ifeq ($(MODE),acc)
CFLAGS1 = -I. -Minfo=acc -fast -Mfprelaxed -acc -ta=tesla -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(MODE),omp)
CFLAGS1 = -I. -Minfo=mp -fast -mp -Mnouniform -mcmodel=medium -Mlarge_arrays
endif
ifeq ($(MODE),seq)
CFLAGS1 = -I. -fast -c99
endif
endif
endif

ifeq ($(COMPILER),gcc)
###############
# GCC Setting #
###############
ifeq ($(SRCTYPE),cpp)
CXX=g++
ifeq ($(MODE),omp)
CFLAGS1 = -O3 -I. -fopenmp 
CLIBS1 = -lm -lgomp
else
CFLAGS1 = -O3 -I. 
CLIBS1 = -lm
endif
else
CXX=gcc
ifeq ($(MODE),omp)
CFLAGS1 = -O3 -I. -std=c99 -fopenmp
CLIBS1 = -lm -lgomp
else
CFLAGS1 = -O3 -I. -std=c99
CLIBS1 = -lm
endif
endif
endif

ifeq ($(COMPILER),openarc)
###################
# OpenARC Setting #
###################
CXX=g++
CSRCS = ./cetus_output/propagate-toz-test_OpenACC.cpp
# On Linux with CUDA GPU
CFLAGS1 = -O3 -I. -I${openarc}/openarcrt 
CLIBS1 = -L${openarc}/openarcrt -lcuda -lopenaccrt_cuda -lomphelper
# On macOS
#CFLAGS1 = -O3 -I. -I${openarc}/openarcrt -arch x86_64
#CLIBS1 = -L${openarc}/openarcrt -lopenaccrt_opencl -lomphelper -framework OpenCL
endif

ifeq ($(COMPILER),intel)
#################
# Intel Setting #
#################
CXX=icc
CFLAGS1= -Wall -I. -O3 -fopenmp -fopenmp-simd
#CFLAGS1= -Wall -I. -O3 -xMIC-AVX512 -qopenmp -qopenmp-offload=host -fimf-precision=low:sqrt,exp,log,/
endif

ifeq ($(COMPILER),ibm)
###############
# IBM Setting #
###############
CXX=xlc
CFLAGS1= -I. -Wall -v -O3 -qsmp=noauto:omp -qnooffload #host power9
endif

ifeq ($(COMPILER),llvm)
################
# LLVM Setting #
################
CXX=clang
CFLAGS1 = -Wall -O3 -I. -fopenmp -fopenmp-targets=x86_64 -lm
#CFLAGS1 = -Wall -O3 -I. -fopenmp -fopenmp-targets=nvptx64 -lm
endif

################################################
# TARGET is where the output binary is stored. #
################################################
TARGET = ./bin

$(TARGET)/$(BENCHMARK): $(CSRCS)
	if [ ! -d "./bin" ]; then mkdir bin; fi
	$(CXX) $(CFLAGS1) $(CSRCS) $(CLIBS1) -o $(TARGET)/$(BENCHMARK)
	if [ -f "./cetus_output/openarc_kernel.cu" ]; then cp ./cetus_output/openarc_kernel.cu ${TARGET}/; fi
	if [ -f "./cetus_output/openarc_kernel.cl" ]; then cp ./cetus_output/openarc_kernel.cl ${TARGET}/; fi

clean:
	rm -f $(TARGET)/$(BENCHMARK) $(TARGET)/openarc_kernel.* $(TARGET)/*.ptx *.o

purge: clean
	rm -rf bin cetus_output openarcConf.txt 
>>>>>>> 9adff9f1d876a27e1e421319e7e186fc07f49fee
