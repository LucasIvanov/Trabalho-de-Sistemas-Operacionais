#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* ============================================
   CONFIGURAÇÕES GLOBAIS DO SISTEMA DE ARQUIVOS
   ============================================ */

#define BLOCK_SIZE 512              // Tamanho de cada bloco em bytes
#define TOTAL_BLOCKS 65536          // Total de blocos no disco (32MB)
#define MAX_FILES 2048              // Máximo de arquivos simultaneamente
#define METADATA_SIZE 32            // Tamanho dos metadados de cada arquivo
#define MAX_FILENAME_LENGTH 8       // Tamanho máximo do nome do arquivo
#define SUPERBLOCK_BLOCKS 1         // Blocos para o superbloco
#define BITMAP_BLOCKS 16            // Blocos para o bitmap (65536 bits = 8192 bytes)
#define ROOT_DIR_BLOCKS 128         // Blocos para o diretório raiz (2048 * 32 bytes)

/* Início de cada seção no disco */
#define SUPERBLOCK_START 0
#define BITMAP_START (SUPERBLOCK_START + SUPERBLOCK_BLOCKS)
#define ROOT_DIR_START (BITMAP_START + BITMAP_BLOCKS)
#define DATA_START (ROOT_DIR_START + ROOT_DIR_BLOCKS)

/* ============================================
   TIPOS DE ARQUIVO
   ============================================ */

typedef enum {
    TYPE_TEXTO = 0x1,
    TYPE_BINARIO = 0x2,
    TYPE_DIRETORIO = 0x3,
    TYPE_IMAGEM = 0x4,
    TYPE_AUDIO = 0x5,
    TYPE_EXECUTAVEL = 0x6,
    TYPE_RESERVADO1 = 0x7,
    TYPE_RESERVADO2 = 0x8
} FileType;

/* ============================================
   PERMISSÕES DE ARQUIVO
   ============================================ */

typedef enum {
    PERM_NONE = 0x0,           // Sem acesso
    PERM_READ = 0x1,           // Somente leitura
    PERM_WRITE = 0x2,          // Escrita
    PERM_READ_WRITE = 0x3,     // Leitura + Escrita
    PERM_EXEC = 0x4,           // Execução
    PERM_READ_EXEC = 0x5,      // Leitura + Execução
    PERM_WRITE_EXEC = 0x6,     // Escrita + Execução
    PERM_ALL = 0x7             // Leitura + Escrita + Execução
} FilePermission;

/* ============================================
   ESTRUTURAS DO SISTEMA DE ARQUIVOS
   ============================================ */

/* Superbloco - informações globais do sistema */
typedef struct {
    char signature[8];          // Assinatura do sistema "UNIOESTE"
    uint32_t block_size;        // Tamanho do bloco
    uint32_t total_blocks;      // Total de blocos
    uint32_t free_blocks;       // Blocos livres
    uint32_t bitmap_start;      // Início do bitmap
    uint32_t root_dir_start;    // Início do diretório raiz
    uint32_t data_start;        // Início da área de dados
    uint32_t max_files;         // Máximo de arquivos
    uint32_t current_files;     // Arquivos atuais
    uint8_t reserved[472];      // Reservado para expansão futura
} __attribute__((packed)) Superblock;

/* Metadados do arquivo - 32 bytes */
typedef struct {
    char name[8];               // Nome do arquivo (8 bytes)
    uint8_t type[3];            // Tipo/extensão (3 bytes)
    uint8_t size[6];            // Tamanho em blocos (6 bytes)
    uint8_t location[8];        // Bloco inicial (8 bytes)
    uint8_t owner[3];           // Dono do arquivo (3 bytes)
    uint8_t permission[3];      // Permissões (3 bytes)
    uint8_t last_modified;      // Última modificação (1 byte)
} __attribute__((packed)) FileMetadata;

/* Estrutura de arquivo em uso (memória) */
typedef struct {
    char name[MAX_FILENAME_LENGTH + 1];
    FileType type;
    uint64_t size_bytes;        // Tamanho em bytes
    uint64_t size_blocks;       // Tamanho em blocos
    uint64_t start_block;       // Bloco inicial
    uint8_t owner;              // ID do dono
    FilePermission permission;  // Permissões
    uint8_t last_modified;      // Status de modificação
    int is_used;                // Se está em uso
} FileEntry;

/* Estrutura do sistema de arquivos */
typedef struct {
    FILE *disk_file;            // Arquivo que representa o disco
    Superblock superblock;      // Superbloco
    uint8_t *bitmap;            // Bitmap de blocos livres
    FileEntry *file_table;      // Tabela de arquivos
    uint8_t current_user;       // Usuário atual
} FileSystem;

/* ============================================
   FUNÇÕES PRINCIPAIS DO SISTEMA DE ARQUIVOS
   ============================================ */

/* Inicialização e formatação */
int fs_format(const char *disk_path);
FileSystem* fs_mount(const char *disk_path);
int fs_unmount(FileSystem *fs);

/* Operações com arquivos */
int fs_create(FileSystem *fs, const char *name, FileType type, FilePermission perm);
int fs_write(FileSystem *fs, const char *name, const void *data, uint64_t size);
int fs_read(FileSystem *fs, const char *name, void *buffer, uint64_t *size);
int fs_copy(FileSystem *fs, const char *src_name, const char *dest_name);
int fs_remove(FileSystem *fs, const char *name);

/* Listagem e informações */
int fs_list(FileSystem *fs);
int fs_info(FileSystem *fs, const char *name);
int fs_disk_info(FileSystem *fs);

/* Funções auxiliares */
int fs_set_user(FileSystem *fs, uint8_t user_id);
int fs_check_permission(FileSystem *fs, const char *name, FilePermission required);

/* ============================================
   FUNÇÕES INTERNAS (BITMAP E BLOCOS)
   ============================================ */

/* Gerenciamento de bitmap */
int bitmap_get_bit(uint8_t *bitmap, uint64_t bit_index);
void bitmap_set_bit(uint8_t *bitmap, uint64_t bit_index);
void bitmap_clear_bit(uint8_t *bitmap, uint64_t bit_index);
int64_t bitmap_find_contiguous(uint8_t *bitmap, uint64_t total_bits, uint64_t num_blocks);

/* Operações de bloco */
int block_read(FILE *disk, uint64_t block_num, void *buffer);
int block_write(FILE *disk, uint64_t block_num, const void *buffer);

/* Conversão de metadados */
void metadata_to_entry(const FileMetadata *meta, FileEntry *entry);
void entry_to_metadata(const FileEntry *entry, FileMetadata *meta);

/* Utilitários */
uint64_t bytes_to_array(const uint8_t *array, int size);
void array_to_bytes(uint64_t value, uint8_t *array, int size);
const char* filetype_to_string(FileType type);
const char* permission_to_string(FilePermission perm);

#endif // FILESYSTEM_H
