# Sistema de Arquivos - UNIOESTE

**Trabalho Final de Sistemas Operacionais**  
**Universidade Estadual do Oeste do Paraná - UNIOESTE**  
**Curso de Bacharelado em Ciência da Computação**

---

## 👥 Autores

- **Lucas Ivanov Costa**
- **Ryan Hideki Inoue Matsunaga Pereira**

**Professor:** Marcio Seiji Oyamada  
**Ano:** 2025

---

## 📋 Descrição do Projeto

Este projeto implementa um sistema de arquivos completo em linguagem C, desenvolvido como trabalho final da disciplina de Sistemas Operacionais. O sistema utiliza **alocação contígua** para armazenamento de dados e **bitmap** para gerenciamento de espaços livres.

### Características Principais

- **Alocação Contígua**: Arquivos são armazenados em blocos sequenciais
- **Bitmap**: Gerenciamento eficiente de blocos livres/ocupados
- **Tamanho do Bloco**: 512 bytes
- **Capacidade Total**: 65.536 blocos (32 MB)
- **Máximo de Arquivos**: 2.048 arquivos simultâneos
- **Sistema de Permissões**: Controle de acesso baseado em usuários (0-7)
- **Múltiplos Tipos de Arquivo**: Texto, Binário, Diretório, Imagem, Áudio, Executável

---

## 🏗️ Arquitetura do Sistema

### Estrutura do Disco

```
┌─────────────────────────────────────────────┐
│  BLOCO 0: SUPERBLOCO                        │  (1 bloco)
├─────────────────────────────────────────────┤
│  BLOCOS 1-16: BITMAP                        │  (16 blocos)
├─────────────────────────────────────────────┤
│  BLOCOS 17-144: DIRETÓRIO RAIZ              │  (128 blocos)
├─────────────────────────────────────────────┤
│  BLOCOS 145-65535: ÁREA DE DADOS            │  (65.391 blocos)
└─────────────────────────────────────────────┘
```

### Metadados do Arquivo (32 bytes)

| Campo            | Tamanho | Descrição                          |
|------------------|---------|------------------------------------|
| Nome             | 8 bytes | Nome do arquivo                    |
| Tipo             | 3 bytes | Tipo/extensão do arquivo           |
| Tamanho          | 6 bytes | Tamanho em blocos                  |
| Localização      | 8 bytes | Bloco inicial                      |
| Dono             | 3 bytes | ID do proprietário (0-7)           |
| Permissões       | 3 bytes | Permissões de acesso               |
| Última alteração | 1 byte  | Status de modificação              |

### Tipos de Arquivo

| Código | Tipo       | Extensão |
|--------|------------|----------|
| 0x1    | Texto      | txt      |
| 0x2    | Binário    | bin      |
| 0x3    | Diretório  | dir      |
| 0x4    | Imagem     | img      |
| 0x5    | Áudio      | aud      |
| 0x6    | Executável | exe      |

### Permissões

| Código | Permissões           | Símbolo |
|--------|----------------------|---------|
| 0x0    | Sem acesso           | ---     |
| 0x1    | Somente leitura      | r--     |
| 0x2    | Escrita              | -w-     |
| 0x3    | Leitura + Escrita    | rw-     |
| 0x4    | Execução             | --x     |
| 0x5    | Leitura + Execução   | r-x     |
| 0x6    | Escrita + Execução   | -wx     |
| 0x7    | Todas                | rwx     |

---

## 🔧 Compilação e Execução

### Pré-requisitos

- GCC (GNU Compiler Collection)
- Make
- Sistema operacional Linux ou Unix-like

### Compilação

```bash
# Compilar o projeto
make

# Compilar e executar
make run

# Ver informações do projeto
make info

# Ver ajuda
make help
```

### Limpeza

```bash
# Remover tudo (executável, objetos e disco virtual)
make clean

# Remover apenas arquivos objeto
make clean-obj

# Remover apenas o disco virtual
make clean-disk
```

---

## 💻 Uso do Sistema

### Comandos Disponíveis

#### Inicialização

```bash
format              # Formata o disco virtual (apaga todos os dados)
mount               # Monta o sistema de arquivos
```

#### Operações com Arquivos

```bash
create <nome> <tipo>   # Cria um novo arquivo
                       # Tipos: txt, bin, dir, img, aud, exe

write <nome>           # Escreve dados em um arquivo
                       # Finalize a entrada com uma linha contendo apenas '###'

read <nome>            # Lê e exibe o conteúdo de um arquivo

copy <origem> <dest>   # Copia um arquivo

remove <nome>          # Remove um arquivo
rm <nome>              # Alias para remove
```

#### Consultas

```bash
list                   # Lista todos os arquivos
ls                     # Alias para list

info <nome>            # Mostra informações detalhadas de um arquivo

diskinfo               # Mostra informações do disco
```

#### Gerenciamento de Usuários

```bash
user <id>              # Altera o usuário atual (0-7)
                       # 0 = root (acesso total)
                       # 1-7 = usuários comuns
```

#### Outros

```bash
help                   # Mostra o menu de ajuda
exit                   # Sai do programa
quit                   # Alias para exit
```

### Exemplo de Uso

```bash
# 1. Execute o programa
./filesystem

# 2. Formate o disco (primeira vez)
fs> format

# 3. Monte o sistema
fs> mount

# 4. Crie um arquivo de texto
fs> create teste.txt txt

# 5. Escreva dados no arquivo
fs> write teste.txt
Digite o conteúdo (finalize com uma linha contendo apenas '###'):
Olá, este é um teste do sistema de arquivos!
Este é um trabalho final de Sistemas Operacionais.
###

# 6. Liste os arquivos
fs> list

# 7. Leia o arquivo
fs> read teste.txt

# 8. Copie o arquivo
fs> copy teste.txt copia.txt

# 9. Veja informações do arquivo
fs> info teste.txt

# 10. Veja informações do disco
fs> diskinfo

# 11. Remova um arquivo
fs> remove copia.txt

# 12. Saia do programa
fs> exit
```

---

## 🔒 Sistema de Permissões

### Usuários

- **user0 (root)**: Acesso total a todos os arquivos
- **user1-7**: Usuários comuns com permissões limitadas

### Regras de Acesso

1. O **proprietário** de um arquivo sempre tem acesso total
2. O **root (user0)** sempre tem acesso total
3. Outros usuários dependem das permissões configuradas
4. Apenas o proprietário ou root podem **remover** arquivos

### Exemplo de Controle de Acesso

```bash
# Como root (user0) - criar arquivo
fs> user 0
fs> create privado.txt txt
fs> write privado.txt
Dados privados
###

# Mudar para user1
fs> user 1

# Tentar ler (sucesso se tiver permissão de leitura)
fs> read privado.txt

# Tentar remover (falha - não é o dono)
fs> remove privado.txt
```

---

## 📊 Especificações Técnicas

### Limites do Sistema

| Item                    | Limite          |
|-------------------------|-----------------|
| Tamanho do bloco        | 512 bytes       |
| Total de blocos         | 65.536          |
| Capacidade total        | 32 MB           |
| Máximo de arquivos      | 2.048           |
| Tamanho máximo do nome  | 8 caracteres    |
| Número de usuários      | 8 (0-7)         |
| Tamanho dos metadados   | 32 bytes/arquivo|

### Estruturas de Dados

#### Superbloco (512 bytes)
- Assinatura: "UNIOESTE"
- Informações globais do sistema
- Contadores de blocos e arquivos

#### Bitmap (8.192 bytes = 16 blocos)
- 1 bit por bloco
- 0 = livre, 1 = ocupado
- Total: 65.536 bits

#### Diretório Raiz (65.536 bytes = 128 blocos)
- Tabela de 2.048 entradas
- 32 bytes por entrada
- Metadados de todos os arquivos

---

## 🎯 Diferenças em Relação ao FAT32

Embora baseado em conceitos do FAT32, este sistema possui diferenças importantes:

### Nosso Sistema vs FAT32

| Aspecto              | Nosso Sistema          | FAT32                  |
|----------------------|------------------------|------------------------|
| Alocação             | **Contígua**           | Encadeada (lista)      |
| Gerenciamento Livre  | **Bitmap**             | FAT (tabela)           |
| Fragmentação         | Menor inicialmente     | Maior ao longo do tempo|
| Busca de espaço      | Linear no bitmap       | Busca na FAT           |
| Tamanho dos metadados| **32 bytes fixos**     | 32 bytes (similar)     |
| Estrutura            | Simplificada           | Mais complexa          |
| Desempenho (leitura) | Excelente (contíguo)   | Bom                    |
| Desempenho (escrita) | Requer reorganização   | Mais flexível          |

### Vantagens da Nossa Abordagem

✅ **Implementação mais simples e didática**  
✅ **Leitura sequencial muito eficiente**  
✅ **Bitmap compacto e rápido**  
✅ **Menos overhead de metadados**  
✅ **Ideal para fins educacionais**

### Desvantagens

❌ Fragmentação externa pode ser problema  
❌ Modificação de arquivos pode requerer movimentação  
❌ Menos flexível que alocação encadeada  

---

## 📁 Estrutura do Código

```
.
├── filesystem.h       # Definições e estruturas
├── filesystem.c       # Implementação do sistema de arquivos
├── main.c            # Interface de linha de comando
├── Makefile          # Automação da compilação
└── README.md         # Este arquivo
```

### Módulos Principais

#### `filesystem.h`
- Definições de constantes
- Estruturas de dados
- Protótipos de funções

#### `filesystem.c`
- Implementação das operações
- Gerenciamento de bitmap
- Operações de leitura/escrita
- Controle de permissões

#### `main.c`
- Interface com o usuário
- Parser de comandos
- Funções auxiliares de I/O

---

## 🧪 Testes e Validação

### Casos de Teste Recomendados

1. **Formatação e Montagem**
   - Formatar disco
   - Montar sistema
   - Verificar superbloco

2. **Criação de Arquivos**
   - Criar múltiplos arquivos
   - Verificar limite de 2048 arquivos
   - Testar nomes com 8 caracteres

3. **Escrita e Leitura**
   - Escrever dados pequenos
   - Escrever dados grandes (múltiplos blocos)
   - Ler e verificar integridade

4. **Alocação Contígua**
   - Criar arquivo grande
   - Verificar blocos consecutivos
   - Testar limite de espaço

5. **Permissões**
   - Criar arquivo como user0
   - Tentar acessar como user1
   - Verificar controle de acesso

6. **Remoção**
   - Remover arquivo
   - Verificar liberação no bitmap
   - Verificar espaço disponível

---

## 🐛 Tratamento de Erros

O sistema trata os seguintes erros:

- ❌ Disco não formatado
- ❌ Arquivo já existe
- ❌ Arquivo não encontrado
- ❌ Espaço insuficiente no disco
- ❌ Limite de arquivos atingido
- ❌ Nome muito longo
- ❌ Permissão negada
- ❌ Falha de leitura/escrita
- ❌ Usuário inválido

---

## 📚 Referências

1. **Tanenbaum, A. S.** - Sistemas Operacionais Modernos
2. **Silberschatz, A., Galvin, P. B., Gagne, G.** - Fundamentos de Sistemas Operacionais
3. **Material da disciplina** - Sistemas Operacionais, UNIOESTE
4. **Documentação FAT32** - Microsoft Corporation

---

## 📝 Licença

Este projeto foi desenvolvido para fins educacionais como trabalho final da disciplina de Sistemas Operacionais da UNIOESTE.

---

## 📧 Contato

Para dúvidas ou sugestões sobre o projeto, entre em contato com os autores através da UNIOESTE.

---

## ✨ Agradecimentos

Agradecemos ao Professor Marcio Seiji Oyamada pela orientação e conhecimentos transmitidos durante a disciplina de Sistemas Operacionais.

---

**UNIOESTE - Universidade Estadual do Oeste do Paraná**  
**Centro de Ciências Exatas e Tecnológicas**  
**Colegiado de Ciência da Computação**  
**Cascavel - 2025**
