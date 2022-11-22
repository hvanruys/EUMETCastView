LIB_OBJ=  CHOptim.o CHcodec.o CHufftables.o CJBlock.o CJPEGDecoder.o CJPEGLossLessCoder.o CJPEGLossyCoder.o CQuantizationTable.o CompressJPEG.o

CPPFLAGS+= -I../Inc  -I../../Inc -I../../../DISE 

all: libJPEG.a


clean:
	rm -f *.o
	rm -f libJPEG.a

libJPEG.a: $(LIB_OBJ)
	ar r libJPEG.a $(LIB_OBJ)
	ranlib libJPEG.a
	@touch libJPEG.a


.cpp.o:
	g++ $(CPPFLAGS) -c -m$(BITS) $<

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)
