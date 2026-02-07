# RELATÓRIO FINAL - SISTEMA DE ARQUIVOS

**Universidade Estadual do Oeste do Paraná - UNIOESTE**  
**Disciplina:** Sistemas Operacionais  
**Professor:** Marcio Seiji Oyamada  

**Autores:**
- Lucas Ivanov Costa
- Ryan Hideki Inoue Matsunaga Pereira

**Data:** 2025

---

## RESUMO EXECUTIVO

Este documento apresenta a implementação completa de um sistema de arquivos desenvolvido em linguagem C como trabalho final da disciplina de Sistemas Operacionais. O sistema utiliza **alocação contígua** para armazenamento e **bitmap** para gerenciamento de espaços livres, oferecendo uma implementação didática e funcional dos conceitos estudados.

## ARQUITETURA IMPLEMENTADA

### 1. Estrutura do Disco (32 MB - 65.536 blocos)

```
┌──────────────────────────────────────────┐
│ Bloco 0          │ Superbloco           │  512 bytes
├──────────────────────────────────────────┤
│ Blocos 1-16      │ Bitmap               │  8.192 bytes
├──────────────────────────────────────────┤
│ Blocos 17-144    │ Diretório Raiz       │  65.536 bytes
├──────────────────────────────────────────┤
│ Blocos 145-65535 │ Área de Dados        │  ~33 MB
└──────────────────────────────────────────┘
```

### 2. Metadados (32 bytes por arquivo)

| Campo         | Bytes | Descrição                    |
|---------------|-------|------------------------------|
| Nome          | 8     | Nome do arquivo              |
| Tipo          | 3     | Tipo/extensão                |
| Tamanho       | 6     | Tamanho em blocos            |
| Localização   | 8     | Bloco inicial (alocação contígua) |
| Dono          | 3     | ID do proprietário (0-7)     |
| Permissões    | 3     | Bits de acesso               |
| Modificação   | 1     | Flag de última alteração     |

### 3. Características Principais

✅ **Alocação Contígua**
- Blocos consecutivos no disco
- Excelente desempenho de leitura sequencial
- Facilita o cálculo de endereços

✅ **Gerenciamento por Bitmap**
- 1 bit por bloco (0=livre, 1=ocupado)
- Busca eficiente de espaços livres
- Estrutura compacta (8 KB para 64K blocos)

✅ **Sistema de Permissões**
- 8 usuários (0-7)
- Root (user0) com acesso total
- Controle de leitura, escrita e execução

✅ **Múltiplos Tipos de Arquivo**
- Texto (txt)
- Binário (bin)
- Diretório (dir)
- Imagem (img)
- Áudio (aud)
- Executável (exe)

## FUNCIONALIDADES IMPLEMENTADAS

### Operações Básicas

1. **Formatação (`fs_format`)**
   - Cria estrutura inicial do disco
   - Inicializa superbloco, bitmap e diretório raiz
   - Marca blocos do sistema como ocupados

2. **Montagem/Desmontagem (`fs_mount` / `fs_unmount`)**
   - Carrega estruturas em memória
   - Valida integridade do sistema
   - Sincroniza alterações com o disco

3. **Criação de Arquivos (`fs_create`)**
   - Adiciona entrada no diretório
   - Valida unicidade do nome
   - Define permissões e proprietário

4. **Escrita (`fs_write`)**
   - Busca espaço contíguo no bitmap
   - Aloca blocos necessários
   - Grava dados no disco
   - Atualiza metadados

5. **Leitura (`fs_read`)**
   - Verifica permissões
   - Lê blocos sequencialmente
   - Retorna dados ao usuário

6. **Cópia (`fs_copy`)**
   - Duplica metadados e dados
   - Cria novo arquivo independente

7. **Remoção (`fs_remove`)**
   - Libera blocos no bitmap
   - Remove entrada do diretório
   - Verifica permissões

### Operações Auxiliares

- **Listagem de Arquivos (`fs_list`)**
- **Informações Detalhadas (`fs_info`)**
- **Status do Disco (`fs_disk_info`)**
- **Troca de Usuário (`fs_set_user`)**

## DIFERENÇAS EM RELAÇÃO AO FAT32

| Aspecto              | Nossa Implementação  | FAT32                |
|----------------------|----------------------|----------------------|
| **Alocação**         | Contígua             | Encadeada (lista)    |
| **Gerenciamento**    | Bitmap               | FAT (tabela)         |
| **Fragmentação**     | Externa (menos comum)| Interna (mais comum) |
| **Leitura Seq.**     | Excelente            | Boa                  |
| **Flexibilidade**    | Menor                | Maior                |
| **Simplicidade**     | Alta (didático)      | Média                |

### Vantagens da Nossa Abordagem

✅ Implementação mais simples e didática  
✅ Melhor desempenho em leitura sequencial  
✅ Bitmap compacto e eficiente  
✅ Ideal para aprendizado  

### Desvantagens

❌ Fragmentação externa pode ocorrer  
❌ Modificação de arquivos requer realocação  
❌ Menos flexível para crescimento  

## ARQUIVOS DO PROJETO

### Código Fonte

1. **filesystem.h** (Cabeçalho)
   - Definições de constantes
   - Estruturas de dados
   - Protótipos de funções
   - ~200 linhas

2. **filesystem.c** (Implementação)
   - Lógica do sistema de arquivos
   - Operações com arquivos
   - Gerenciamento de bitmap
   - ~700 linhas

3. **main.c** (Interface)
   - Parser de comandos
   - Interface de linha de comando
   - Funções auxiliares de I/O
   - ~300 linhas

4. **test.c** (Testes)
   - 12 suítes de teste
   - Validação de funcionalidades
   - Testes de stress
   - ~470 linhas

### Documentação

5. **README.md** - Documentação completa
6. **QUICKSTART.md** - Guia rápido de uso
7. **Makefile** - Automação da compilação

## COMO USAR

### Compilação

```bash
make
```

### Execução

```bash
./filesystem
```

### Comandos Principais

```
format              # Formatar disco
mount               # Montar sistema
create <nome> <tipo> # Criar arquivo
write <nome>        # Escrever dados
read <nome>         # Ler arquivo
list                # Listar arquivos
info <nome>         # Info detalhada
diskinfo            # Status do disco
exit                # Sair
```

### Exemplo de Uso

```bash
# Terminal
$ ./filesystem

# Dentro do sistema
fs> format          # Formatar (confirmar com 's')
fs> mount           # Montar
fs> create teste txt
fs> write teste
Conteúdo do arquivo
###
fs> read teste
fs> list
fs> exit
```

## VALIDAÇÃO E TESTES

Implementamos 12 suítes de teste automatizado:

1. ✅ Formatação do disco
2. ✅ Montagem do sistema
3. ✅ Criação de arquivos
4. ✅ Escrita e leitura
5. ✅ Arquivos grandes (múltiplos blocos)
6. ✅ Cópia de arquivos
7. ✅ Sistema de permissões
8. ✅ Remoção de arquivos
9. ✅ Alocação contígua
10. ✅ Gerenciamento de bitmap
11. ✅ Persistência de dados
12. ✅ Teste de stress (100 arquivos)

## CONCEITOS APLICADOS

### Sistemas Operacionais

- Gerenciamento de arquivos
- Alocação de espaço em disco
- Estruturas de dados persistentes
- Controle de acesso e permissões

### Programação em C

- Manipulação de bits (bitmap)
- Operações de I/O com arquivos
- Estruturas packed (alinhamento)
- Gerenciamento de memória

### Organização de Dados

- Serialização de estruturas
- Endereçamento de blocos
- Metadados e índices
- Persistência

## LIMITAÇÕES E MELHORIAS FUTURAS

### Limitações Atuais

- Não implementa diretórios hierárquicos
- Alocação contígua pode causar fragmentação
- Sem desfragmentação automática
- Limite de 2048 arquivos

### Possíveis Melhorias

1. **Diretórios Aninhados**
   - Implementar árvore de diretórios
   - Caminhos absolutos e relativos

2. **Desfragmentação**
   - Algoritmo de compactação
   - Reorganização de blocos

3. **Journal/Log**
   - Recuperação de falhas
   - Transações atômicas

4. **Cache**
   - Buffer de blocos em memória
   - Melhor desempenho

5. **Compressão**
   - Economizar espaço
   - Transparente ao usuário

## CONCLUSÃO

Este projeto demonstra a implementação prática de conceitos fundamentais de sistemas operacionais, especialmente no que diz respeito a sistemas de arquivos. A escolha de alocação contígua e bitmap ofereceu um equilíbrio entre simplicidade de implementação e eficiência operacional.

O sistema desenvolvido é totalmente funcional, suportando todas as operações básicas de um sistema de arquivos real: criação, leitura, escrita, cópia e remoção de arquivos, além de controle de acesso baseado em usuários.

A experiência de implementar um sistema de arquivos do zero proporcionou profundo entendimento sobre:
- Organização e persistência de dados
- Gerenciamento de espaço em disco
- Tradução entre abstrações lógicas e armazenamento físico
- Desafios de integridade e consistência de dados

## REFERÊNCIAS

1. **Tanenbaum, A. S.** - Sistemas Operacionais Modernos
2. **Silberschatz, A., Galvin, P. B., Gagne, G.** - Fundamentos de Sistemas Operacionais
3. **Material da Disciplina** - UNIOESTE
4. **Documentação FAT32** - Microsoft Corporation

---

**Agradecimentos:** Agradecemos ao Professor Marcio Seiji Oyamada pela orientação e pelos conhecimentos transmitidos durante a disciplina.

---

**UNIOESTE - Centro de Ciências Exatas e Tecnológicas**  
**Colegiado de Ciência da Computação**  
**Cascavel - PR - 2025**
