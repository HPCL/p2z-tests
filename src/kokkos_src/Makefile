# KOKKOS_PATH = ${HOME}/kokkos-simd
KOKKOS_PATH = ${HOME}/soft/kokkos-3.3.01
KOKKOS_DEVICES = "OpenMP"
#KOKKOS_DEVICES = "Cuda"
#KOKKOS_DEVICES = "OpenMP,Cuda"

SRC = $(wildcard *.cpp)

default: build
	echo "Start Build"

ifneq (,$(findstring Cuda,$(KOKKOS_DEVICES)))
CXX = ${KOKKOS_PATH}/bin/nvcc_wrapper 
#EXE = ${EXE_NAME}.cuda
# KOKKOS_ARCH = "HSW,Kepler35"
KOKKOS_ARCH = "SKX,Volta70"
KOKKOS_CUDA_OPTIONS = "enable_lambda"
EXE = ../../bin/propagate_nvcc_kokkos.gpu
USE_GPU=1
else
# g++-10- needed on apollo
#CXX = g++-10
CXX = g++
CXX = icpc
#EXE = ${EXE_NAME}.host
EXE = "../../bin/propagate_$(CXX)_kokkos.host"
KOKKOS_ARCH = "SKX"
endif

ifndef NITER
NITER=5
endif
ifndef NLAYER
NLAYER=20
endif

CXXFLAGS = -O3 -g -DNITER=${NITER} -Dnlayers=${NLAYER} 
LINK = ${CXX}
LINKFLAGS =

ifneq (,$(findstring Cuda,$(KOKKOS_DEVICES)))
CXXFLAGS += ipo -mtune=skylake -xCORE-AVX2 -qopt-zmm-usage=high 
endif

ifdef USE_GPU
CXXFLAGS += -DUSE_GPU=1
endif

DEPFLAGS = -M

OBJ = $(SRC:.cpp=.o)
LIB =

include $(KOKKOS_PATH)/Makefile.kokkos

build: $(EXE)

$(EXE): ptoz_data.h $(OBJ) $(KOKKOS_LINK_DEPENDS)
	$(LINK) $(KOKKOS_LDFLAGS) $(LINKFLAGS) $(EXTRA_PATH) $(OBJ) $(KOKKOS_LIBS) $(LIB) -o $(EXE)

clean: kokkos-clean
	rm -f *.o *.cuda *.host

# Compilation rules
%.o:%.cpp $(KOKKOS_CPP_DEPENDS)
	$(CXX) $(KOKKOS_CPPFLAGS) $(KOKKOS_CXXFLAGS) $(CXXFLAGS) $(EXTRA_INC) -c $<

test: $(EXE)
	./$(EXE)
