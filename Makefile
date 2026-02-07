# Makefile para o Sistema de Arquivos - UNIOESTE
# Trabalho Final de Sistemas Operacionais
# Autores: Lucas Ivanov Costa e Ryan Hideki Inoue Matsunaga Pereira

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = filesystem
OBJS = main.o filesystem.o

# Regra padrão
all: $(TARGET)

# Compilação do executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo ""
	@echo "✓ Compilação concluída com sucesso!"
	@echo "Execute './$(TARGET)' para iniciar o sistema de arquivos."
	@echo ""

# Compilação dos objetos
main.o: main.c filesystem.h
	$(CC) $(CFLAGS) -c main.c

filesystem.o: filesystem.c filesystem.h
	$(CC) $(CFLAGS) -c filesystem.c

# Limpeza
clean:
	rm -f $(OBJS) $(TARGET) virtual_disk.img
	@echo "✓ Arquivos de compilação removidos."

# Limpeza apenas dos objetos (mantém o executável)
clean-obj:
	rm -f $(OBJS)
	@echo "✓ Arquivos objeto removidos."

# Remove apenas o disco virtual
clean-disk:
	rm -f virtual_disk.img
	@echo "✓ Disco virtual removido."

# Executa o programa
run: $(TARGET)
	./$(TARGET)

# Testes automatizados
test: $(TARGET)
	@echo "Executando testes automatizados..."
	@echo ""
	@echo "format" | ./$(TARGET)
	@echo ""
	@echo "✓ Testes concluídos!"

# Informações do projeto
info:
	@echo "=========================================="
	@echo "SISTEMA DE ARQUIVOS - UNIOESTE"
	@echo "=========================================="
	@echo "Curso: Ciência da Computação"
	@echo "Disciplina: Sistemas Operacionais"
	@echo "Professor: Marcio Seiji Oyamada"
	@echo ""
	@echo "Autores:"
	@echo "  - Lucas Ivanov Costa"
	@echo "  - Ryan Hideki Inoue Matsunaga Pereira"
	@echo ""
	@echo "Características do Sistema:"
	@echo "  - Alocação contígua de blocos"
	@echo "  - Gerenciamento por bitmap"
	@echo "  - Tamanho do bloco: 512 bytes"
	@echo "  - Total de blocos: 65536 (32MB)"
	@echo "  - Máximo de arquivos: 2048"
	@echo "=========================================="

# Help
help:
	@echo "Comandos disponíveis:"
	@echo "  make           - Compila o projeto"
	@echo "  make run       - Compila e executa"
	@echo "  make clean     - Remove tudo (executável, objetos e disco)"
	@echo "  make clean-obj - Remove apenas os arquivos objeto"
	@echo "  make clean-disk- Remove apenas o disco virtual"
	@echo "  make test      - Executa testes automatizados"
	@echo "  make info      - Mostra informações do projeto"
	@echo "  make help      - Mostra esta ajuda"

.PHONY: all clean clean-obj clean-disk run test info help
