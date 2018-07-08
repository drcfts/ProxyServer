CC= g++

CFLAGS=	-g -I$(IDIR)

ODIR = obj
IDIR = ../include
LDIR = ../lib

LIBS= -lm

_DEPS =	ProxyParser.h ProxyServer.h

DEPS =	$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ProxyParser.o ProxyServer.o main.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

aracne: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY:
	clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~

executa:
	./aracne -p $(port_number)
