#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_PATH "virtual_disk.img"

void print_menu() {
    printf("\n");
    printf("╔═════════════════════════════════════════════════╗\n");
    printf("║   SISTEMA DE ARQUIVOS - UNIOESTE              ║\n");
    printf("║   Desenvolvido por: Lucas Ivanov & Ryan       ║\n");
    printf("╚═════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Comandos disponíveis:\n");
    printf("  format              - Formata o disco virtual\n");
    printf("  mount               - Monta o sistema de arquivos\n");
    printf("  create <nome> <tipo> - Cria um novo arquivo\n");
    printf("                         Tipos: txt, bin, dir, img, aud, exe\n");
    printf("  write <nome>        - Escreve dados em um arquivo\n");
    printf("  read <nome>         - Lê o conteúdo de um arquivo\n");
    printf("  copy <orig> <dest>  - Copia um arquivo\n");
    printf("  remove <nome>       - Remove um arquivo\n");
    printf("  list                - Lista todos os arquivos\n");
    printf("  info <nome>         - Mostra informações de um arquivo\n");
    printf("  diskinfo            - Mostra informações do disco\n");
    printf("  user <id>           - Altera o usuário (0-7)\n");
    printf("  help                - Mostra este menu\n");
    printf("  exit                - Sai do programa\n");
    printf("\n");
}

FileType parse_file_type(const char *type_str) {
    if (strcmp(type_str, "txt") == 0) return TYPE_TEXTO;
    if (strcmp(type_str, "bin") == 0) return TYPE_BINARIO;
    if (strcmp(type_str, "dir") == 0) return TYPE_DIRETORIO;
    if (strcmp(type_str, "img") == 0) return TYPE_IMAGEM;
    if (strcmp(type_str, "aud") == 0) return TYPE_AUDIO;
    if (strcmp(type_str, "exe") == 0) return TYPE_EXECUTAVEL;
    return TYPE_TEXTO;
}

void cmd_format() {
    printf("\n⚠️  ATENÇÃO: Esta operação irá apagar todos os dados do disco!\n");
    printf("Deseja continuar? (s/n): ");
    
    char confirm;
    scanf(" %c", &confirm);
    getchar(); // Consome o newline
    
    if (confirm == 's' || confirm == 'S') {
        if (fs_format(DISK_PATH) == 0) {
            printf("✓ Disco formatado com sucesso!\n");
        } else {
            printf("✗ Erro ao formatar o disco.\n");
        }
    } else {
        printf("Operação cancelada.\n");
    }
}

FileSystem* cmd_mount() {
    FileSystem *fs = fs_mount(DISK_PATH);
    if (fs) {
        printf("✓ Sistema de arquivos montado!\n");
    } else {
        printf("✗ Erro ao montar. Execute 'format' primeiro.\n");
    }
    return fs;
}

void cmd_create(FileSystem *fs, const char *name, const char *type_str) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    FileType type = parse_file_type(type_str);
    
    if (fs_create(fs, name, type, PERM_ALL) == 0) {
        printf("✓ Arquivo criado com sucesso!\n");
    }
}

void cmd_write(FileSystem *fs, const char *name) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    printf("Digite o conteúdo (finalize com uma linha contendo apenas '###'):\n");
    
    char *buffer = malloc(65536); // 64KB buffer
    char line[256];
    int total_size = 0;
    
    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "###\n") == 0) {
            break;
        }
        int len = strlen(line);
        if (total_size + len < 65536) {
            memcpy(buffer + total_size, line, len);
            total_size += len;
        } else {
            printf("⚠️  Limite de tamanho atingido (64KB).\n");
            break;
        }
    }
    
    if (total_size > 0) {
        if (fs_write(fs, name, buffer, total_size) == 0) {
            printf("✓ Dados escritos com sucesso!\n");
        }
    } else {
        printf("Nenhum dado para escrever.\n");
    }
    
    free(buffer);
}

void cmd_read(FileSystem *fs, const char *name) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    void *buffer = malloc(1024 * 1024); // 1MB buffer
    uint64_t size;
    
    if (fs_read(fs, name, buffer, &size) == 0) {
        printf("\n--- CONTEÚDO DO ARQUIVO '%s' ---\n", name);
        
        // Tenta exibir como texto se for tipo texto
        int is_text = 1;
        for (uint64_t i = 0; i < size && i < 1000; i++) {
            if (((uint8_t*)buffer)[i] < 32 && 
                ((uint8_t*)buffer)[i] != '\n' && 
                ((uint8_t*)buffer)[i] != '\r' && 
                ((uint8_t*)buffer)[i] != '\t') {
                is_text = 0;
                break;
            }
        }
        
        if (is_text) {
            fwrite(buffer, 1, size, stdout);
        } else {
            printf("(Arquivo binário - exibindo hexadecimal dos primeiros 256 bytes)\n");
            for (uint64_t i = 0; i < size && i < 256; i++) {
                printf("%02X ", ((uint8_t*)buffer)[i]);
                if ((i + 1) % 16 == 0) printf("\n");
            }
        }
        
        printf("\n--- FIM DO ARQUIVO ---\n");
    }
    
    free(buffer);
}

void cmd_copy(FileSystem *fs, const char *src, const char *dest) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    if (fs_copy(fs, src, dest) == 0) {
        printf("✓ Arquivo copiado com sucesso!\n");
    }
}

void cmd_remove(FileSystem *fs, const char *name) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    printf("Deseja realmente remover '%s'? (s/n): ", name);
    char confirm;
    scanf(" %c", &confirm);
    getchar();
    
    if (confirm == 's' || confirm == 'S') {
        if (fs_remove(fs, name) == 0) {
            printf("✓ Arquivo removido com sucesso!\n");
        }
    } else {
        printf("Operação cancelada.\n");
    }
}

void cmd_list(FileSystem *fs) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    fs_list(fs);
}

void cmd_info(FileSystem *fs, const char *name) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    fs_info(fs, name);
}

void cmd_diskinfo(FileSystem *fs) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    fs_disk_info(fs);
}

void cmd_user(FileSystem *fs, int user_id) {
    if (!fs) {
        printf("✗ Sistema não montado. Execute 'mount' primeiro.\n");
        return;
    }
    
    if (user_id < 0 || user_id > 7) {
        printf("✗ ID de usuário inválido (0-7).\n");
        return;
    }
    
    fs_set_user(fs, user_id);
}

int main(int argc, char *argv[]) {
    FileSystem *fs = NULL;
    char command[256];
    char arg1[64], arg2[64], arg3[64];
    
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║                                                       ║\n");
    printf("║   SISTEMA DE ARQUIVOS - TRABALHO FINAL DE SO         ║\n");
    printf("║   Universidade Estadual do Oeste do Paraná           ║\n");
    printf("║                                                       ║\n");
    printf("║   Autores:                                            ║\n");
    printf("║   - Lucas Ivanov Costa                                ║\n");
    printf("║   - Ryan Hideki Inoue Matsunaga Pereira               ║\n");
    printf("║                                                       ║\n");
    printf("║   Professor: Marcio Seiji Oyamada                     ║\n");
    printf("║   Curso: Ciência da Computação                        ║\n");
    printf("║   Ano: 2025                                           ║\n");
    printf("║                                                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    
    print_menu();
    
    while (1) {
        printf("fs> ");
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        // Limpa argumentos
        arg1[0] = arg2[0] = arg3[0] = '\0';
        
        // Parse do comando
        char cmd[64] = {0};
        sscanf(command, "%s %s %s %s", cmd, arg1, arg2, arg3);
        
        if (strlen(cmd) == 0) {
            continue;
        }
        
        // Executa comandos
        if (strcmp(cmd, "format") == 0) {
            cmd_format();
        }
        else if (strcmp(cmd, "mount") == 0) {
            if (fs) {
                printf("⚠️  Sistema já montado. Desmontando...\n");
                fs_unmount(fs);
            }
            fs = cmd_mount();
        }
        else if (strcmp(cmd, "create") == 0) {
            if (strlen(arg1) > 0 && strlen(arg2) > 0) {
                cmd_create(fs, arg1, arg2);
            } else {
                printf("Uso: create <nome> <tipo>\n");
            }
        }
        else if (strcmp(cmd, "write") == 0) {
            if (strlen(arg1) > 0) {
                cmd_write(fs, arg1);
            } else {
                printf("Uso: write <nome>\n");
            }
        }
        else if (strcmp(cmd, "read") == 0) {
            if (strlen(arg1) > 0) {
                cmd_read(fs, arg1);
            } else {
                printf("Uso: read <nome>\n");
            }
        }
        else if (strcmp(cmd, "copy") == 0) {
            if (strlen(arg1) > 0 && strlen(arg2) > 0) {
                cmd_copy(fs, arg1, arg2);
            } else {
                printf("Uso: copy <origem> <destino>\n");
            }
        }
        else if (strcmp(cmd, "remove") == 0 || strcmp(cmd, "rm") == 0) {
            if (strlen(arg1) > 0) {
                cmd_remove(fs, arg1);
            } else {
                printf("Uso: remove <nome>\n");
            }
        }
        else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "ls") == 0) {
            cmd_list(fs);
        }
        else if (strcmp(cmd, "info") == 0) {
            if (strlen(arg1) > 0) {
                cmd_info(fs, arg1);
            } else {
                printf("Uso: info <nome>\n");
            }
        }
        else if (strcmp(cmd, "diskinfo") == 0) {
            cmd_diskinfo(fs);
        }
        else if (strcmp(cmd, "user") == 0) {
            if (strlen(arg1) > 0) {
                cmd_user(fs, atoi(arg1));
            } else {
                printf("Uso: user <id>\n");
            }
        }
        else if (strcmp(cmd, "help") == 0) {
            print_menu();
        }
        else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            printf("Encerrando...\n");
            break;
        }
        else {
            printf("Comando desconhecido: '%s'\n", cmd);
            printf("Digite 'help' para ver os comandos disponíveis.\n");
        }
    }
    
    // Desmonta antes de sair
    if (fs) {
        printf("\nDesmontando sistema de arquivos...\n");
        fs_unmount(fs);
    }
    
    printf("Obrigado por usar nosso sistema de arquivos!\n");
    return 0;
}
