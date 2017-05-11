CXX=g++ -m64
CXXFLAGS=-I../common -Iobjs/ -O3 -Wall -lpthread -w
ISPC=ispc
# note: change target to avx-x2 for AVX capable machines
ISPCFLAGS=-O2 --target=avx-x2 --arch=x86-64

APP_NAME=skiplist
OBJDIR=objs

default: $(APP_NAME)

.PHONY: dirs clean

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *~ $(APP_NAME)

OBJS=$(OBJDIR)/main.o $(OBJDIR)/compare_ispc.o

$(APP_NAME): dirs $(OBJS)
		$(CXX) -std=c++11 $(CXXFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp
		$(CXX) -std=c++11 $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/main.o: $(OBJDIR)/compare_ispc.h

$(OBJDIR)/%_ispc.h $(OBJDIR)//%_ispc.o: %.ispc
		$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h