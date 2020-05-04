OBJETOS = main.o memory.o cliente.o control.o empresa.o file.o intermediario.o prodcons.o scheduler.o time.o
OBJ_DIR = obj

soperpro: $(OBJETOS)
	gcc $(addprefix $(OBJ_DIR)/,$(OBJETOS)) $(OBJ_DIR)/so.o -lrt -lpthread -o bin/soperpro

%.o: src/%.c $($@)
	gcc -I include -o $(OBJ_DIR)/$@ -c $<

clean:
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJETOS))
	rm -f bin/soperpro


cenario1:
	./bin/soperpro tests/in/scenario1 tests/out/scenario1test -l tests/log/cenario1test.log -t 1000

cenario2:
	./bin/soperpro tests/in/scenario2 tests/out/scenario2test -l tests/log/cenario2test.log -t 1000

cenario3:
	./bin/soperpro tests/in/scenario3 tests/out/scenario3test -l tests/log/cenario3test.log -t 1000

cenario4:
	./bin/soperpro tests/in/scenario4 tests/out/scenario4test -l tests/log/cenario4test.log -t 1000