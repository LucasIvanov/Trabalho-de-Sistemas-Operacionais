# GUIA DE USO RÁPIDO - SISTEMA DE ARQUIVOS UNIOESTE

## Compilação

```bash
make
```

## Executar o Programa

```bash
./filesystem
```

## Uso Passo a Passo

### 1. Primeira execução - Formatar o disco

```
fs> format
```
Digite 's' para confirmar a formatação.

### 2. Montar o sistema

```
fs> mount
```

### 3. Ver informações do disco

```
fs> diskinfo
```

### 4. Criar arquivos

```
fs> create teste txt
fs> create dados bin
fs> create foto img
```

### 5. Listar arquivos

```
fs> list
```

### 6. Escrever dados em um arquivo

```
fs> write teste
```

Digite o conteúdo linha por linha.
Finalize com uma linha contendo apenas `###`

Exemplo:
```
Olá, mundo!
Este é um teste.
###
```

### 7. Ler um arquivo

```
fs> read teste
```

### 8. Copiar arquivo

```
fs> copy teste copia
```

### 9. Ver informações de um arquivo específico

```
fs> info teste
```

### 10. Remover um arquivo

```
fs> remove copia
```

### 11. Mudar de usuário

```
fs> user 1
```

Para voltar ao root:
```
fs> user 0
```

### 12. Sair do programa

```
fs> exit
```

## Exemplo Completo de Sessão

```
$ ./filesystem

fs> format
s

fs> mount

fs> create teste txt

fs> write teste
Primeira linha
Segunda linha
###

fs> read teste

fs> list

fs> info teste

fs> diskinfo

fs> exit
```

## Tipos de Arquivo Suportados

- `txt` - Arquivos de texto
- `bin` - Arquivos binários
- `dir` - Diretórios
- `img` - Imagens
- `aud` - Áudio
- `exe` - Executáveis

## Permissões

- `0` (user0) - Root, acesso total
- `1-7` - Usuários comuns

## Limites

- Nome do arquivo: máximo 8 caracteres
- Total de arquivos: 2048
- Tamanho do disco: 32 MB
- Tamanho do bloco: 512 bytes

## Troubleshooting

**Erro: "Sistema não montado"**
- Solução: Execute `mount` antes de outras operações

**Erro: "Disco não formatado"**
- Solução: Execute `format` primeiro

**Erro: "Nome muito longo"**
- Solução: Use nomes com até 8 caracteres

**Erro: "Arquivo já existe"**
- Solução: Escolha outro nome ou remova o arquivo existente

**Erro: "Sem permissão"**
- Solução: Mude para o usuário correto ou use user0 (root)
