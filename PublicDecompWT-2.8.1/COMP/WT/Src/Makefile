LIB_OBJ= CACCoder.o CACDecoder.o CACModel.o CVLCCoder.o CVLCDecoder.o CWBlock.o CWTCoder.o CWTDecoder.o CompressWT.o

CPPFLAGS+= -I../Inc  -I../../Inc -I../../../DISE

all: libWT.a


clean:
	rm -f *.o
	rm -f libWT.a

libWT.a: $(LIB_OBJ)
	ar r libWT.a $(LIB_OBJ)
	ranlib libWT.a
	@touch libWT.a

.cpp.o:
	g++ $(CPPFLAGS) -c -m$(BITS) $<

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)
