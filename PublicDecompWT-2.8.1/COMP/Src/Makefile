LIB_OBJ= CBuffer.o CImage.o CQualityMeasures.o

CPPFLAGS+= -I../Inc  -I../../DISE

all: libCOMP.a

clean:
	rm -f *.o
	rm -f libCOMP.a

libCOMP.a: $(LIB_OBJ)
	ar r libCOMP.a  $(LIB_OBJ)
	ranlib libCOMP.a
	@touch libCOMP.a

.cpp.o:
	g++ $(CPPFLAGS) -c -m$(BITS) $<

depend:
	makedepend -- $(CPPFLAGS) -- $(MAKEDEPFLAGS) -- $(SOURCES)
