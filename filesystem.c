#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================
   FUNÇÕES AUXILIARES - BITMAP
   ============================================ */

int bitmap_get_bit(uint8_t *bitmap, uint64_t bit_index) {
    uint64_t byte_index = bit_index / 8;
    uint8_t bit_offset = bit_index % 8;
    return (bitmap[byte_index] >> bit_offset) & 1;
}

void bitmap_set_bit(uint8_t *bitmap, uint64_t bit_index) {
    uint64_t byte_index = bit_index / 8;
    uint8_t bit_offset = bit_index % 8;
    bitmap[byte_index] |= (1 << bit_offset);
}

void bitmap_clear_bit(uint8_t *bitmap, uint64_t bit_index) {
    uint64_t byte_index = bit_index / 8;
    uint8_t bit_offset = bit_index % 8;
    bitmap[byte_index] &= ~(1 << bit_offset);
}

int64_t bitmap_find_contiguous(uint8_t *bitmap, uint64_t total_bits, uint64_t num_blocks) {
    uint64_t count = 0;
    uint64_t start = 0;
    
    for (uint64_t i = DATA_START; i < total_bits; i++) {
        if (bitmap_get_bit(bitmap, i) == 0) {
            if (count == 0) {
                start = i;
            }
            count++;
            if (count == num_blocks) {
                return start;
            }
        } else {
            count = 0;
        }
    }
    
    return -1; // Não encontrou espaço contíguo suficiente
}

/* ============================================
   FUNÇÕES AUXILIARES - BLOCOS
   ============================================ */

int block_read(FILE *disk, uint64_t block_num, void *buffer) {
    if (fseek(disk, block_num * BLOCK_SIZE, SEEK_SET) != 0) {
        return -1;
    }
    if (fread(buffer, BLOCK_SIZE, 1, disk) != 1) {
        return -1;
    }
    return 0;
}

int block_write(FILE *disk, uint64_t block_num, const void *buffer) {
    if (fseek(disk, block_num * BLOCK_SIZE, SEEK_SET) != 0) {
        return -1;
    }
    if (fwrite(buffer, BLOCK_SIZE, 1, disk) != 1) {
        return -1;
    }
    fflush(disk);
    return 0;
}

/* ============================================
   FUNÇÕES AUXILIARES - CONVERSÃO
   ============================================ */

uint64_t bytes_to_array(const uint8_t *array, int size) {
    uint64_t value = 0;
    for (int i = 0; i < size; i++) {
        value |= ((uint64_t)array[i]) << (i * 8);
    }
    return value;
}

void array_to_bytes(uint64_t value, uint8_t *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = (value >> (i * 8)) & 0xFF;
    }
}

void metadata_to_entry(const FileMetadata *meta, FileEntry *entry) {
    memcpy(entry->name, meta->name, MAX_FILENAME_LENGTH);
    entry->name[MAX_FILENAME_LENGTH] = '\0';
    
    entry->type = (FileType)bytes_to_array(meta->type, 3);
    entry->size_blocks = bytes_to_array(meta->size, 6);
    entry->size_bytes = entry->size_blocks * BLOCK_SIZE;
    entry->start_block = bytes_to_array(meta->location, 8);
    entry->owner = (uint8_t)bytes_to_array(meta->owner, 3);
    entry->permission = (FilePermission)bytes_to_array(meta->permission, 3);
    entry->last_modified = meta->last_modified;
    
    // Verifica se o arquivo está em uso (nome não vazio)
    entry->is_used = (meta->name[0] != '\0');
}

void entry_to_metadata(const FileEntry *entry, FileMetadata *meta) {
    memset(meta, 0, sizeof(FileMetadata));
    memcpy(meta->name, entry->name, MAX_FILENAME_LENGTH);
    
    array_to_bytes(entry->type, meta->type, 3);
    array_to_bytes(entry->size_blocks, meta->size, 6);
    array_to_bytes(entry->start_block, meta->location, 8);
    array_to_bytes(entry->owner, meta->owner, 3);
    array_to_bytes(entry->permission, meta->permission, 3);
    meta->last_modified = entry->last_modified;
}

const char* filetype_to_string(FileType type) {
    switch(type) {
        case TYPE_TEXTO: return "TXT";
        case TYPE_BINARIO: return "BIN";
        case TYPE_DIRETORIO: return "DIR";
        case TYPE_IMAGEM: return "IMG";
        case TYPE_AUDIO: return "AUD";
        case TYPE_EXECUTAVEL: return "EXE";
        default: return "???";
    }
}

const char* permission_to_string(FilePermission perm) {
    switch(perm) {
        case PERM_NONE: return "---";
        case PERM_READ: return "r--";
        case PERM_WRITE: return "-w-";
        case PERM_READ_WRITE: return "rw-";
        case PERM_EXEC: return "--x";
        case PERM_READ_EXEC: return "r-x";
        case PERM_WRITE_EXEC: return "-wx";
        case PERM_ALL: return "rwx";
        default: return "???";
    }
}

/* ============================================
   FORMATAÇÃO DO SISTEMA DE ARQUIVOS
   ============================================ */

int fs_format(const char *disk_path) {
    FILE *disk = fopen(disk_path, "wb");
    if (!disk) {
        printf("Erro: Não foi possível criar o disco virtual.\n");
        return -1;
    }
    
    // Cria um disco vazio
    uint8_t *zero_block = calloc(1, BLOCK_SIZE);
    for (int i = 0; i < TOTAL_BLOCKS; i++) {
        fwrite(zero_block, BLOCK_SIZE, 1, disk);
    }
    free(zero_block);
    
    // Cria o superbloco
    Superblock sb;
    memset(&sb, 0, sizeof(Superblock));
    strcpy(sb.signature, "UNIOESTE");
    sb.block_size = BLOCK_SIZE;
    sb.total_blocks = TOTAL_BLOCKS;
    sb.free_blocks = TOTAL_BLOCKS - DATA_START;
    sb.bitmap_start = BITMAP_START;
    sb.root_dir_start = ROOT_DIR_START;
    sb.data_start = DATA_START;
    sb.max_files = MAX_FILES;
    sb.current_files = 0;
    
    fseek(disk, 0, SEEK_SET);
    fwrite(&sb, sizeof(Superblock), 1, disk);
    
    // Inicializa o bitmap (marca blocos do sistema como ocupados)
    uint8_t *bitmap = calloc(BITMAP_BLOCKS * BLOCK_SIZE, 1);
    for (int i = 0; i < DATA_START; i++) {
        bitmap_set_bit(bitmap, i);
    }
    
    fseek(disk, BITMAP_START * BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, BITMAP_BLOCKS * BLOCK_SIZE, 1, disk);
    free(bitmap);
    
    // Inicializa o diretório raiz (vazio)
    uint8_t *root_dir = calloc(ROOT_DIR_BLOCKS * BLOCK_SIZE, 1);
    fseek(disk, ROOT_DIR_START * BLOCK_SIZE, SEEK_SET);
    fwrite(root_dir, ROOT_DIR_BLOCKS * BLOCK_SIZE, 1, disk);
    free(root_dir);
    
    fclose(disk);
    printf("Sistema de arquivos formatado com sucesso!\n");
    printf("Tamanho total: %d blocos (%d MB)\n", TOTAL_BLOCKS, 
           (TOTAL_BLOCKS * BLOCK_SIZE) / (1024 * 1024));
    printf("Área de dados: %d blocos\n", TOTAL_BLOCKS - DATA_START);
    return 0;
}

/* ============================================
   MONTAGEM E DESMONTAGEM
   ============================================ */

FileSystem* fs_mount(const char *disk_path) {
    FileSystem *fs = malloc(sizeof(FileSystem));
    if (!fs) {
        printf("Erro: Falha ao alocar memória para o sistema de arquivos.\n");
        return NULL;
    }
    
    // Abre o disco
    fs->disk_file = fopen(disk_path, "r+b");
    if (!fs->disk_file) {
        printf("Erro: Não foi possível abrir o disco virtual.\n");
        free(fs);
        return NULL;
    }
    
    // Lê o superbloco
    fseek(fs->disk_file, 0, SEEK_SET);
    if (fread(&fs->superblock, sizeof(Superblock), 1, fs->disk_file) != 1) {
        printf("Erro: Falha ao ler o superbloco.\n");
        fclose(fs->disk_file);
        free(fs);
        return NULL;
    }
    
    // Verifica a assinatura
    if (strncmp(fs->superblock.signature, "UNIOESTE", 8) != 0) {
        printf("Erro: Assinatura inválida. Disco não formatado?\n");
        fclose(fs->disk_file);
        free(fs);
        return NULL;
    }
    
    // Carrega o bitmap
    fs->bitmap = malloc(BITMAP_BLOCKS * BLOCK_SIZE);
    fseek(fs->disk_file, BITMAP_START * BLOCK_SIZE, SEEK_SET);
    if (fread(fs->bitmap, BITMAP_BLOCKS * BLOCK_SIZE, 1, fs->disk_file) != 1) {
        printf("Erro: Falha ao ler o bitmap.\n");
        fclose(fs->disk_file);
        free(fs);
        return NULL;
    }
    
    // Carrega a tabela de arquivos
    fs->file_table = calloc(MAX_FILES, sizeof(FileEntry));
    uint8_t *root_dir_data = malloc(ROOT_DIR_BLOCKS * BLOCK_SIZE);
    fseek(fs->disk_file, ROOT_DIR_START * BLOCK_SIZE, SEEK_SET);
    fread(root_dir_data, ROOT_DIR_BLOCKS * BLOCK_SIZE, 1, fs->disk_file);
    
    for (int i = 0; i < MAX_FILES; i++) {
        FileMetadata *meta = (FileMetadata *)(root_dir_data + i * METADATA_SIZE);
        metadata_to_entry(meta, &fs->file_table[i]);
    }
    free(root_dir_data);
    
    fs->current_user = 0; // Root por padrão
    
    printf("Sistema de arquivos montado com sucesso!\n");
    printf("Arquivos presentes: %d/%d\n", fs->superblock.current_files, MAX_FILES);
    printf("Blocos livres: %d/%d\n", fs->superblock.free_blocks, 
           TOTAL_BLOCKS - DATA_START);
    
    return fs;
}

int fs_unmount(FileSystem *fs) {
    if (!fs) return -1;
    
    // Salva o superbloco
    fseek(fs->disk_file, 0, SEEK_SET);
    fwrite(&fs->superblock, sizeof(Superblock), 1, fs->disk_file);
    
    // Salva o bitmap
    fseek(fs->disk_file, BITMAP_START * BLOCK_SIZE, SEEK_SET);
    fwrite(fs->bitmap, BITMAP_BLOCKS * BLOCK_SIZE, 1, fs->disk_file);
    
    // Salva o diretório raiz
    uint8_t *root_dir_data = calloc(ROOT_DIR_BLOCKS * BLOCK_SIZE, 1);
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used) {
            FileMetadata meta;
            entry_to_metadata(&fs->file_table[i], &meta);
            memcpy(root_dir_data + i * METADATA_SIZE, &meta, METADATA_SIZE);
        }
    }
    fseek(fs->disk_file, ROOT_DIR_START * BLOCK_SIZE, SEEK_SET);
    fwrite(root_dir_data, ROOT_DIR_BLOCKS * BLOCK_SIZE, 1, fs->disk_file);
    free(root_dir_data);
    
    // Libera recursos
    fclose(fs->disk_file);
    free(fs->bitmap);
    free(fs->file_table);
    free(fs);
    
    printf("Sistema de arquivos desmontado com sucesso!\n");
    return 0;
}

/* ============================================
   OPERAÇÕES COM ARQUIVOS
   ============================================ */

int fs_create(FileSystem *fs, const char *name, FileType type, FilePermission perm) {
    if (!fs || !name) return -1;
    
    // Verifica tamanho do nome
    if (strlen(name) > MAX_FILENAME_LENGTH) {
        printf("Erro: Nome muito longo (máximo %d caracteres).\n", MAX_FILENAME_LENGTH);
        return -1;
    }
    
    // Verifica se já existe
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            printf("Erro: Arquivo '%s' já existe.\n", name);
            return -1;
        }
    }
    
    // Procura entrada livre
    int free_entry = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!fs->file_table[i].is_used) {
            free_entry = i;
            break;
        }
    }
    
    if (free_entry == -1) {
        printf("Erro: Número máximo de arquivos atingido.\n");
        return -1;
    }
    
    // Cria o arquivo
    FileEntry *entry = &fs->file_table[free_entry];
    strcpy(entry->name, name);
    entry->type = type;
    entry->size_bytes = 0;
    entry->size_blocks = 0;
    entry->start_block = 0;
    entry->owner = fs->current_user;
    entry->permission = perm;
    entry->last_modified = 0;
    entry->is_used = 1;
    
    fs->superblock.current_files++;
    
    printf("Arquivo '%s' criado com sucesso.\n", name);
    return 0;
}

int fs_write(FileSystem *fs, const char *name, const void *data, uint64_t size) {
    if (!fs || !name || !data || size == 0) return -1;
    
    // Procura o arquivo
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return -1;
    }
    
    FileEntry *entry = &fs->file_table[file_index];
    
    // Verifica permissão
    if (entry->owner != fs->current_user && fs->current_user != 0) {
        if (!(entry->permission & PERM_WRITE)) {
            printf("Erro: Sem permissão de escrita.\n");
            return -1;
        }
    }
    
    // Calcula blocos necessários
    uint64_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Libera blocos antigos se o arquivo já tinha dados
    if (entry->size_blocks > 0) {
        for (uint64_t i = 0; i < entry->size_blocks; i++) {
            bitmap_clear_bit(fs->bitmap, entry->start_block + i);
        }
        fs->superblock.free_blocks += entry->size_blocks;
    }
    
    // Procura espaço contíguo
    int64_t start_block = bitmap_find_contiguous(fs->bitmap, TOTAL_BLOCKS, blocks_needed);
    if (start_block == -1) {
        printf("Erro: Espaço insuficiente no disco.\n");
        return -1;
    }
    
    // Marca blocos como ocupados
    for (uint64_t i = 0; i < blocks_needed; i++) {
        bitmap_set_bit(fs->bitmap, start_block + i);
    }
    
    // Escreve os dados
    uint8_t *buffer = calloc(1, BLOCK_SIZE);
    const uint8_t *data_ptr = (const uint8_t *)data;
    
    for (uint64_t i = 0; i < blocks_needed; i++) {
        uint64_t bytes_to_write = BLOCK_SIZE;
        if (i == blocks_needed - 1) {
            bytes_to_write = size - (i * BLOCK_SIZE);
        }
        memcpy(buffer, data_ptr + (i * BLOCK_SIZE), bytes_to_write);
        block_write(fs->disk_file, start_block + i, buffer);
        memset(buffer, 0, BLOCK_SIZE);
    }
    free(buffer);
    
    // Atualiza metadados
    entry->size_bytes = size;
    entry->size_blocks = blocks_needed;
    entry->start_block = start_block;
    entry->last_modified = 1;
    fs->superblock.free_blocks -= blocks_needed;
    
    printf("Dados escritos no arquivo '%s' (%lu bytes, %lu blocos).\n", 
           name, size, blocks_needed);
    return 0;
}

int fs_read(FileSystem *fs, const char *name, void *buffer, uint64_t *size) {
    if (!fs || !name || !buffer || !size) return -1;
    
    // Procura o arquivo
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return -1;
    }
    
    FileEntry *entry = &fs->file_table[file_index];
    
    // Verifica permissão
    if (entry->owner != fs->current_user && fs->current_user != 0) {
        if (!(entry->permission & PERM_READ)) {
            printf("Erro: Sem permissão de leitura.\n");
            return -1;
        }
    }
    
    if (entry->size_blocks == 0) {
        printf("Arquivo '%s' está vazio.\n", name);
        *size = 0;
        return 0;
    }
    
    // Lê os blocos
    uint8_t *block_buffer = malloc(BLOCK_SIZE);
    uint8_t *data_ptr = (uint8_t *)buffer;
    
    for (uint64_t i = 0; i < entry->size_blocks; i++) {
        block_read(fs->disk_file, entry->start_block + i, block_buffer);
        
        uint64_t bytes_to_copy = BLOCK_SIZE;
        if (i == entry->size_blocks - 1) {
            bytes_to_copy = entry->size_bytes - (i * BLOCK_SIZE);
        }
        memcpy(data_ptr + (i * BLOCK_SIZE), block_buffer, bytes_to_copy);
    }
    free(block_buffer);
    
    *size = entry->size_bytes;
    printf("Arquivo '%s' lido (%lu bytes).\n", name, entry->size_bytes);
    return 0;
}

int fs_copy(FileSystem *fs, const char *src_name, const char *dest_name) {
    if (!fs || !src_name || !dest_name) return -1;
    
    // Procura o arquivo de origem
    int src_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, src_name) == 0) {
            src_index = i;
            break;
        }
    }
    
    if (src_index == -1) {
        printf("Erro: Arquivo origem '%s' não encontrado.\n", src_name);
        return -1;
    }
    
    FileEntry *src = &fs->file_table[src_index];
    
    // Cria o arquivo de destino
    if (fs_create(fs, dest_name, src->type, src->permission) != 0) {
        return -1;
    }
    
    // Copia os dados se houver
    if (src->size_bytes > 0) {
        void *buffer = malloc(src->size_bytes);
        uint64_t size;
        
        if (fs_read(fs, src_name, buffer, &size) == 0) {
            fs_write(fs, dest_name, buffer, size);
        }
        free(buffer);
    }
    
    printf("Arquivo '%s' copiado para '%s'.\n", src_name, dest_name);
    return 0;
}

int fs_remove(FileSystem *fs, const char *name) {
    if (!fs || !name) return -1;
    
    // Procura o arquivo
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            file_index = i;
            break;
        }
    }
    
    if (file_index == -1) {
        printf("Erro: Arquivo '%s' não encontrado.\n", name);
        return -1;
    }
    
    FileEntry *entry = &fs->file_table[file_index];
    
    // Verifica permissão (apenas o dono ou root pode remover)
    if (entry->owner != fs->current_user && fs->current_user != 0) {
        printf("Erro: Apenas o dono pode remover o arquivo.\n");
        return -1;
    }
    
    // Libera os blocos
    if (entry->size_blocks > 0) {
        for (uint64_t i = 0; i < entry->size_blocks; i++) {
            bitmap_clear_bit(fs->bitmap, entry->start_block + i);
        }
        fs->superblock.free_blocks += entry->size_blocks;
    }
    
    // Remove da tabela
    memset(entry, 0, sizeof(FileEntry));
    fs->superblock.current_files--;
    
    printf("Arquivo '%s' removido.\n", name);
    return 0;
}

/* ============================================
   FUNÇÕES DE LISTAGEM E INFORMAÇÕES
   ============================================ */

int fs_list(FileSystem *fs) {
    if (!fs) return -1;
    
    printf("\n========================================\n");
    printf("LISTAGEM DE ARQUIVOS\n");
    printf("========================================\n");
    printf("%-10s %-5s %-8s %-10s %-5s %s\n", 
           "NOME", "TIPO", "TAMANHO", "BLOCOS", "DONO", "PERM");
    printf("----------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used) {
            FileEntry *e = &fs->file_table[i];
            printf("%-10s %-5s %7luB %9lu  %4d  %s\n",
                   e->name,
                   filetype_to_string(e->type),
                   e->size_bytes,
                   e->size_blocks,
                   e->owner,
                   permission_to_string(e->permission));
            count++;
        }
    }
    
    printf("----------------------------------------\n");
    printf("Total: %d arquivo(s)\n", count);
    printf("========================================\n\n");
    
    return 0;
}

int fs_info(FileSystem *fs, const char *name) {
    if (!fs || !name) return -1;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            FileEntry *e = &fs->file_table[i];
            
            printf("\n========================================\n");
            printf("INFORMAÇÕES DO ARQUIVO\n");
            printf("========================================\n");
            printf("Nome:           %s\n", e->name);
            printf("Tipo:           %s\n", filetype_to_string(e->type));
            printf("Tamanho:        %lu bytes\n", e->size_bytes);
            printf("Blocos:         %lu\n", e->size_blocks);
            printf("Bloco inicial:  %lu\n", e->start_block);
            printf("Proprietário:   user%d\n", e->owner);
            printf("Permissões:     %s\n", permission_to_string(e->permission));
            printf("Modificado:     %s\n", e->last_modified ? "Sim" : "Não");
            printf("========================================\n\n");
            
            return 0;
        }
    }
    
    printf("Erro: Arquivo '%s' não encontrado.\n", name);
    return -1;
}

int fs_disk_info(FileSystem *fs) {
    if (!fs) return -1;
    
    printf("\n========================================\n");
    printf("INFORMAÇÕES DO DISCO\n");
    printf("========================================\n");
    printf("Assinatura:     %s\n", fs->superblock.signature);
    printf("Tamanho bloco:  %d bytes\n", fs->superblock.block_size);
    printf("Total blocos:   %d\n", fs->superblock.total_blocks);
    printf("Blocos livres:  %d\n", fs->superblock.free_blocks);
    printf("Blocos usados:  %d\n", 
           fs->superblock.total_blocks - fs->superblock.free_blocks);
    printf("Arquivos:       %d/%d\n", 
           fs->superblock.current_files, fs->superblock.max_files);
    printf("----------------------------------------\n");
    printf("Bitmap início:  bloco %d\n", fs->superblock.bitmap_start);
    printf("Diret. raiz:    bloco %d\n", fs->superblock.root_dir_start);
    printf("Dados início:   bloco %d\n", fs->superblock.data_start);
    printf("========================================\n\n");
    
    return 0;
}

int fs_set_user(FileSystem *fs, uint8_t user_id) {
    if (!fs || user_id > 7) return -1;
    fs->current_user = user_id;
    printf("Usuário alterado para: user%d\n", user_id);
    return 0;
}

int fs_check_permission(FileSystem *fs, const char *name, FilePermission required) {
    if (!fs || !name) return -1;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->file_table[i].is_used && 
            strcmp(fs->file_table[i].name, name) == 0) {
            FileEntry *e = &fs->file_table[i];
            
            if (e->owner == fs->current_user || fs->current_user == 0) {
                return 1; // Dono ou root tem acesso total
            }
            
            return (e->permission & required) == required;
        }
    }
    
    return -1;
}
